// ---------------------------------------------------------------------
// This file is part of falcon-core.
//
// Copyright (C) 2015, 2016, 2017 Neuro-Electronics Research Flanders
//
// Falcon-server is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Falcon-server is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with falcon-core. If not, see <http://www.gnu.org/licenses/>.
// ---------------------------------------------------------------------

#include "rebuffer.hpp"

#include <algorithm>

#include "utilities/time.hpp"

Rebuffer::Rebuffer() : IProcessor() {
  add_option("downsample factor", downsample_factor_,
             "The factor for downsampling the signal.");
  add_option("buffer size", buffer_size_,
             "Output buffer size in samples or seconds.");
}

void Rebuffer::CreatePorts() {
  data_in_port_ = create_input_port<MultiChannelType<double>>(
      "data", MultiChannelType<double>::Capabilities(ChannelRange(1, 256)),
      PortInPolicy(SlotRange(0, 256)));

  data_out_port_ = create_output_port<MultiChannelType<double>>(
      "data", MultiChannelType<double>::Capabilities(ChannelRange(1, 256)),
      MultiChannelType<double>::Parameters(), PortOutPolicy(SlotRange(0, 256)));
}

void Rebuffer::Configure(const GlobalContext &context) {
  LOG(INFO) << name() << ". Downsample factor set to " << downsample_factor_()
            << ".";
  LOG(INFO) << name() << ". Buffer size set to " << buffer_size_.to_string()
            << ".";
}

void Rebuffer::CompleteStreamInfo() {
  // check if we have the same number of input and output slots
  if (data_in_port_->number_of_slots() != data_out_port_->number_of_slots()) {
    throw ProcessingStreamInfoError(
        "Number of outputs does not match the number of inputs.", name());
  }

  // compute number of output samples for each input stream
  if (buffer_size_.unit() == units::precise::sample_units &&
      buffer_size_() > 0.) {
    sample_buffer_.assign(data_in_port_->number_of_slots(), buffer_size_());
  } else if (buffer_size_.unit() == units::precise::second &&
             buffer_size_() > 0.) {
    sample_buffer_.assign(data_in_port_->number_of_slots(), 0);

    for (int k = 0; k < data_in_port_->number_of_slots(); ++k) {
      sample_buffer_[k] = time2samples<unsigned int>(
          buffer_size_(),
          data_in_port_->streaminfo(k).parameters().sample_rate /
              downsample_factor_());
      if (sample_buffer_[k] == 0) {
        throw ProcessingStreamInfoError("Buffer duration is zero.", name());
      }
    }
  } else {
    for (int k = 0; k < data_in_port_->number_of_slots(); ++k) {
      sample_buffer_[k] =
          std::max(1u, static_cast<unsigned int>(std::floor(
                           data_in_port_->streaminfo(k).parameters().nsamples /
                           downsample_factor_())));
    }
  }

  // finalize
  for (int k = 0; k < data_in_port_->number_of_slots(); ++k) {
    data_out_port_->streaminfo(k).set_parameters(
        MultiChannelType<double>::Parameters(
            data_in_port_->streaminfo(k).parameters().nchannels,
            sample_buffer_[k],
            data_in_port_->streaminfo(k).parameters().sample_rate /
                downsample_factor_()));
    data_out_port_->streaminfo(k).set_stream_rate(
        data_in_port_->streaminfo(k).stream_rate() *
        data_in_port_->streaminfo(k).parameters().nsamples / sample_buffer_[k]);
  }
}

void Rebuffer::Process(ProcessingContext &context) {
  auto nslots = data_in_port_->number_of_slots();

  MultiChannelType<double>::Data *data_in = nullptr;
  std::vector<MultiChannelType<double>::Data *> data_out;

  data_out.assign(nslots, nullptr);
  decltype(sample_buffer_) sample_out_counter = sample_buffer_;
  decltype(sample_buffer_) offset;
  offset.assign(nslots, 0);

  unsigned int s = 0;

  while (!context.terminated()) {
    // go through all slots
    for (int k = 0; k < nslots; ++k) {
      // retrieve new data
      if (!data_in_port_->slot(k)->RetrieveData(data_in)) {
        break;
      }

      s = 0;

      while (s < data_in->nsamples()) {
        for (s = offset[k]; s < data_in->nsamples() &&
                            sample_out_counter[k] < sample_buffer_[k];
             s += downsample_factor_()) {
          for (unsigned int c = 0; c < data_in->nchannels(); ++c) {
            data_out[k]->set_data_sample(sample_out_counter[k], c,
                                         data_in->data_sample(s, c));
          }
          data_out[k]->set_sample_timestamp(sample_out_counter[k],
                                            data_in->sample_timestamp(s));
          sample_out_counter[k]++;
        }

        if (sample_out_counter[k] == sample_buffer_[k]) {
          data_out_port_->slot(k)->PublishData();
          data_out[k] = data_out_port_->slot(k)->ClaimData(false);
          data_out[k]->CloneTimestamps(*data_in);
          sample_out_counter[k] = 0;
        }

        if (s >= data_in->nsamples()) {
          offset[k] = s - data_in->nsamples();
        } else {
          offset[k] = s;
        }
      }

      data_in_port_->slot(k)->ReleaseData();
    }
  }
}

REGISTERPROCESSOR(Rebuffer)
