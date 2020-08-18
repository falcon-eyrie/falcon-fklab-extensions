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

#include "distributor.hpp"
#include "utilities/general.hpp"

Distributor::Distributor() : IProcessor(PRIORITY_MEDIUM) {
  add_option("channelmap", channelmap_,
             "Mapping of channels to processor output ports.", true);
}

void Distributor::CreatePorts() {
  input_port_ = create_input_port<MultiChannelType<double>>(
      MultiChannelType<double>::Capabilities(ChannelRange(1, MAX_N_CHANNELS)),
      PortInPolicy(SlotRange(1)));

  for (auto &it : channelmap_()) {
    data_ports_[it.first] = create_output_port<MultiChannelType<double>>(
        it.first,
        MultiChannelType<double>::Capabilities(ChannelRange(it.second.size())),
        MultiChannelType<double>::Parameters(),
        PortOutPolicy(SlotRange(1), BUFFER_SIZE, WAIT_STRATEGY));
  }
}

void Distributor::CompleteStreamInfo() {
  incoming_batch_size_ = input_port_->streaminfo(0).parameters().nsamples;
  max_n_channels_ = input_port_->streaminfo(0).parameters().nchannels;

  LOG(INFO) << name() << ". Incoming batch size: " << incoming_batch_size_
            << ".";

  for (auto &it : data_ports_) {
    it.second->streaminfo(0).set_parameters(
        MultiChannelType<double>::Parameters(
            channelmap_().at(it.first).size(), incoming_batch_size_,
            input_port_->streaminfo(0)
                .parameters()
                .sample_rate));

    it.second->streaminfo(0).set_stream_rate(
        input_port_->streaminfo(0).stream_rate());
  }
}

void Distributor::Prepare(GlobalContext &context) {
  // check channel map
  for (auto const &it : channelmap_()) {
    if (it.second.size() == 0) {
      throw ProcessingPrepareError(
          "Channel map entry " + it.first + " has zero channels.", name());
    }

    for (auto const &ch : it.second) {
      if (ch >= max_n_channels_) {
        throw ProcessingPrepareError(
            "Channel " + std::to_string(static_cast<int>(ch)) + " is invalid",
            name());
      }
    }
  }
}

void Distributor::Process(ProcessingContext &context) {
  MultiChannelType<double>::Data *data_in = nullptr;
  int port_index;
  unsigned int ch, s;
  std::vector<MultiChannelType<double>::Data *> data_out_vector(
      data_ports_.size());

  while (!context.terminated()) {
    // retrieve new data packet
    if (!input_port_->slot(0)->RetrieveData(data_in)) {
      break;
    }

    // claim output data buckets
    // and copy timestamps from upstream
    port_index = 0;
    for (auto const &it : data_ports_) {
      data_out_vector[port_index] = it.second->slot(0)->ClaimData(false);
      data_out_vector[port_index]->set_hardware_timestamp(
          data_in->hardware_timestamp());
      data_out_vector[port_index]->set_source_timestamp(
          data_in->source_timestamp());
      port_index++;
    }

    // for each entry in the channel map
    port_index = 0;
    for (auto const &it_chmap : channelmap_()) {
      data_out_vector[port_index]->set_sample_timestamps(
          data_in->sample_timestamps());

      for (ch = 0; ch < it_chmap.second.size(); ch++) {
        for (s = 0; s < incoming_batch_size_; s++) {
          data_out_vector[port_index]->set_data_sample(
              s, ch, data_in->data_sample(s, it_chmap.second[ch]));
        }
      }
      port_index++;
    }

    // publish data buckets
    for (auto &it : data_ports_) {
      it.second->slot(0)->PublishData();
    }
    // release input data bucket
    input_port_->slot(0)->ReleaseData();
  }
}

void Distributor::Postprocess(ProcessingContext &context) {
  SlotType s;
  for (auto &it : data_ports_) {
    for (s = 0; s < it.second->number_of_slots(); ++s) {
      LOG(INFO) << name() << ". Port " << it.first << ". Slot " << s
                << ". Streamed " << it.second->slot(s)->nitems_produced()
                << " data packets. ";
    }
  }
}

REGISTERPROCESSOR(Distributor)
