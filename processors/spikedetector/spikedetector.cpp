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

#include "spikedetector.hpp"

SpikeDetector::SpikeDetector() : IProcessor() {
  add_option(THRESHOLD, initial_threshold_,
             "Spike detection threshold in data units.");
  add_option("invert signal", invert_signal_,
             "Invert signal before spike detection.");
  add_option("buffer size", buffer_size_,
             "Size (in seconds) of data buffer used for spike detection.");
  add_option("strict time bin check", strict_time_bin_check_,
             "Strict check of compatibility of spike detection buffer size "
             "with the upstream processor");
  add_option(PEAK_LIFETIME, initial_peak_lifetime_,
             "Peak life time in samples");
}

void SpikeDetector::CreatePorts() {
  data_in_port_ = create_input_port<MultiChannelType<double>>(
      "data",
      MultiChannelType<double>::Capabilities(ChannelRange(1, MAX_N_CHANNELS)),
      PortInPolicy(SlotRange(1)));

  data_out_port_spikes_ = create_output_port<SpikeType>(
      SPIKEDATA, SpikeType::Capabilities(ChannelRange(1, MAX_N_CHANNELS)),
      SpikeType::Parameters(buffer_size_()),
      PortOutPolicy(SlotRange(1), RINGBUFFER_SIZE));

  data_out_port_events_ = create_output_port<EventType>(
      EVENTDATA, EventType::Capabilities(), EventType::Parameters(),
      PortOutPolicy(SlotRange(1)));

  threshold_ = create_static_state(THRESHOLD, initial_threshold_(), true,
                                   Permission::WRITE);

  peak_lifetime_ = create_static_state(PEAK_LIFETIME, initial_peak_lifetime_(),
                                       true, Permission::WRITE);
}

void SpikeDetector::CompleteStreamInfo() {
  double incoming_stream_rate = data_in_port_->streaminfo(0).stream_rate();
  incoming_buffer_size_samples_ =
      data_in_port_->slot(0)->streaminfo().parameters().nsamples;
  double incoming_buffer_size_ms =
      incoming_buffer_size_samples_ /
      data_in_port_->slot(0)->streaminfo().parameters().sample_rate * 1000;

  try {
    double tmp = buffer_size_();
    check_buffer_sizes_and_log(incoming_buffer_size_ms, tmp,
                               strict_time_bin_check_(), n_incoming_, name());
    buffer_size_ = tmp;
  } catch (std::runtime_error &error) {
    throw ProcessingStreamInfoError(error.what(), name());
  }

  n_channels_ = data_in_port_->slot(0)->streaminfo().parameters().nchannels;
  auto parms = data_out_port_spikes_->streaminfo(0).parameters();
  parms.nchannels = n_channels_;
  parms.sample_rate = incoming_stream_rate;
  data_out_port_spikes_->streaminfo(0).set_parameters(parms);
  data_out_port_spikes_->streaminfo(0).set_stream_rate(
      incoming_stream_rate / (incoming_buffer_size_samples_ * n_incoming_));
  data_out_port_events_->streaminfo(0).set_stream_rate(IRREGULARSTREAM);
}

void SpikeDetector::Prepare(GlobalContext &context) {
  spike_detector_.reset(new dsp::algorithms::SpikeDetector(
      n_channels_, initial_threshold_(), initial_peak_lifetime_()));

  if (invert_signal_()) {
    inverted_signals_.reset(new MultiChannelType<double>::Data());
    inverted_signals_->Initialize(
        n_channels_, incoming_buffer_size_samples_,
        data_in_port_->slot(0)->streaminfo().parameters().sample_rate);
  }
}

void SpikeDetector::Process(ProcessingContext &context) {
  MultiChannelType<double>::Data *data_in_;
  MultiChannelType<double>::Data *signals = nullptr;

  size_t sample_buffer_counter = 0;
  decltype(data_in_->hardware_timestamp()) hw_timestamp = 0;

  std::unique_ptr<EventType::Data> single_spike_event(
      new EventType::Data("spike"));
  std::unique_ptr<EventType::Data> multiple_spikes_event(
      new EventType::Data("spikes"));
  SpikeType::Data *spike_data_out_;
  EventType::Data *event_data_out_;

  while (!context.terminated()) {
    // update state variables
    spike_detector_->set_threshold(threshold_->get());
    spike_detector_->set_peak_life_time(peak_lifetime_->get());

    // claim one data bucket and look for spikes
    spike_data_out_ = data_out_port_spikes_->slot(0)->ClaimData(true);

    // look for spikes
    while (sample_buffer_counter < n_incoming_) {
      if (!data_in_port_->slot(0)->RetrieveData(data_in_)) {
        break;
      }

      // SpikeData will be marked with the the first timestamp of the
      // buffer of samples used for detection
      if (sample_buffer_counter == 0) {
        hw_timestamp = data_in_->hardware_timestamp();
      }

      // if spike detection has to be performed on the inverted signal,
      // make a local copy of the inverted signal and use it for spike detection
      if (invert_signal_()) {
        for (size_t sample = 0; sample < incoming_buffer_size_samples_;
             ++sample) {
          for (unsigned int channel = 0; channel < n_channels_; ++channel) {
            inverted_signals_->set_data_sample(
                sample, channel, -data_in_->data_sample(sample, channel));
          }
        }
        signals = inverted_signals_.get();
      } else {
        signals = data_in_;
      }

      // detect spikes sample by sample and collect each detected spike
      for (size_t sample = 0; sample < incoming_buffer_size_samples_;
           ++sample) {
        if (spike_detector_->is_spike<double *>(
                data_in_->sample_timestamp(sample),
                signals->begin_sample(sample))) {
          spike_data_out_->add_spike(
              spike_detector_->amplitudes_detected_spike(),
              spike_detector_->timestamp_detected_spike());
        }
      }
      // update counters and timestamp data
      ++sample_buffer_counter;
      spike_data_out_->set_hardware_timestamp(hw_timestamp);
      spike_data_out_->set_source_timestamp();
      data_in_port_->slot(0)->ReleaseData();
    }

    // publish results on the two ports
    data_out_port_spikes_->slot(0)->PublishData();
    sample_buffer_counter = 0;
    if (spike_data_out_->n_detected_spikes() > 0) {
      event_data_out_ = data_out_port_events_->slot(0)->ClaimData(false);
      if (spike_data_out_->n_detected_spikes() > 1) {
        event_data_out_->set_event(*multiple_spikes_event);
      } else {
        event_data_out_->set_event(*single_spike_event);
      }
      event_data_out_->set_hardware_timestamp(hw_timestamp);
      data_out_port_events_->slot(0)->PublishData();
    }
  }
}

void SpikeDetector::Postprocess(ProcessingContext &context) {
  LOG(INFO) << name() << ". # spikes detected = " << spike_detector_->nspikes();
  spike_detector_->reset();
}

REGISTERPROCESSOR(SpikeDetector)
