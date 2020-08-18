// ---------------------------------------------------------------------
// This file is part of falcon-core.
//
// Copyright (C) 2015, 2016, 2017,  Neuro-Electronics Research Flanders
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

#include "burstdetector.hpp"

BurstDetector::BurstDetector() : IProcessor() {
  add_option(THRESHOLD_DEV, initial_threshold_dev_,
             "Multiplier (in number of signal standard deviations) to "
             "compute the initial threshold.");
  add_option(SMOOTH_TIME, initial_smooth_time_,
             "Integration time for estimating signal statistics.");
  add_option(DETECTION_LOCKOUT_TIME, initial_detection_lockout_time_,
             "Lockout time (in seconds) to avoid over-stimulation.");
  add_option(STREAM_EVENTS, default_stream_events_,
             "Enable streaming of burst events.");
  add_option(STREAM_STATISTICS, initial_stats_out_,
             "Enable streaming of statistics.");
  add_option("statistics buffer size", stats_buffer_size_,
             "Size (in seconds) for statistics output buffers.");
}

void BurstDetector::CreatePorts() {
  data_in_port_ = create_input_port<MUAType>("mua", MUAType::Capabilities(),
                                             PortInPolicy(SlotRange(1)));

  data_out_port_ = create_output_port<EventType>(
      EVENTDATA, EventType::Capabilities(), EventType::Parameters("burst"),
      PortOutPolicy(SlotRange(1)));

  stats_out_port_ = create_output_port<MultiChannelType<double>>(
      "statistics",
      MultiChannelType<double>::Capabilities(ChannelRange(N_STATS_OUT)),
      MultiChannelType<double>::Parameters(), PortOutPolicy(SlotRange(1)));

  threshold_ = create_broadcaster_state("threshold", 0.0, Permission::READ);

  signal_mean_ = create_broadcaster_state("mean", 0.0, Permission::READ);

  signal_dev_ = create_broadcaster_state("deviation", 0.0, Permission::READ);

  threshold_dev_ = create_static_state(THRESHOLD_DEV, initial_threshold_dev_(),
                                       true, Permission::WRITE);

  detection_lockout_time_ = create_static_state(
      DETECTION_LOCKOUT_TIME, initial_detection_lockout_time_(), true,
      Permission::WRITE);

  stream_events_ = create_static_state(STREAM_EVENTS, default_stream_events_(),
                                       true, Permission::WRITE);

  smooth_time_ = create_static_state(SMOOTH_TIME, initial_smooth_time_(), true,
                                     Permission::WRITE);

  stats_out_ = create_static_state(STREAM_STATISTICS, initial_stats_out_(),
                                   true, Permission::WRITE);

  burst_ = create_broadcaster_state("burst", false, Permission::READ);

  bin_size_mua_ = create_follower_state("bin size", 1.0, Permission::READ);
}

void BurstDetector::CompleteStreamInfo() {
  stats_nsamples_ = stats_buffer_size_() * 1e3 /
                    data_in_port_->streaminfo(0).parameters().bin_size;
  if (stats_nsamples_ == 0) {
    throw ProcessingCreatePortsError(
        "Stats buffersize is smaller than MUA bin size.", name());
  }

  stats_out_port_->streaminfo(0).set_stream_rate(
      data_in_port_->streaminfo(0).stream_rate());
  stats_out_port_->streaminfo(0).set_parameters(
      MultiChannelType<double>::Parameters(
          N_STATS_OUT, stats_nsamples_,
          1. / data_in_port_->streaminfo(0).parameters().bin_size * 1e3));

  data_out_port_->streaminfo(0).set_stream_rate(IRREGULARSTREAM);
}

void BurstDetector::Preprocess(ProcessingContext &context) {
  signal_mean_->set(0);
  signal_dev_->set(0);
  threshold_->set(0);
  block_ = 0;

  sample_rate_ =
      1. / data_in_port_->slot(0)->streaminfo().parameters().bin_size * 1e3;

  LOG(UPDATE) << name() << ". Incoming Sample rate: " << sample_rate_;

  burn_in_ = initial_smooth_time_() * sample_rate_;

  if (burn_in_ == 0) {
    burn_in_ = 1;
    LOG(UPDATE) << name() << ". " << SMOOTH_TIME
                << " too low. Burn-in set to 1 sample.";
  }

  double alpha = 1.0 / burn_in_;

  running_statistics_.reset(
      new dsp::algorithms::RunningMeanMAD(alpha, burn_in_, false));
  threshold_detector_.reset(new dsp::algorithms::ThresholdCrosser(0));
}

void BurstDetector::Process(ProcessingContext &context) {
  typename MUAType::Data *data_in = nullptr;
  typename EventType::Data *event_out = nullptr;
  typename MultiChannelType<double>::Data *stats_out = nullptr;

  double value, test_value;
  auto stats_nsamples_counter = stats_nsamples_;
  auto burnin_update_sent = false;

  // burn-in period
  while (running_statistics_->is_burning_in() && !context.terminated()) {
    if (!data_in_port_->slot(0)->RetrieveData(data_in)) {
      break;
    }

    if (!burnin_update_sent) {
      LOG(UPDATE) << name() << ": burn-in period starting ("
                  << initial_smooth_time_() << " seconds)";
      burnin_update_sent = true;
    }

    running_statistics_->add_sample(data_in->mua());
    data_in_port_->slot(0)->ReleaseData();
  }

  if (!running_statistics_->is_burning_in()) {
    LOG(UPDATE) << name() << ": end of burn-in period";
    LOG(UPDATE) << name()
                << ": statistics: center = " << running_statistics_->center()
                << ", dispersion = " << running_statistics_->dispersion();

    LOG(UPDATE) << name()
                << ": burst detection starts now with initial threshold of "
                << (threshold_dev_->get() * running_statistics_->dispersion());
  }

  while (!context.terminated()) {
    // retrieve new data
    if (!data_in_port_->slot(0)->RetrieveData(data_in)) {
      break;
    }

    // update threshold and alpha only once for an incoming data bucket
    threshold_->set(threshold_dev_->get() * running_statistics_->dispersion());
    threshold_detector_->set_threshold(threshold_->get());
    running_statistics_->set_alpha(1.0 / (smooth_time_->get() * sample_rate_));
    value = data_in->mua();
    test_value = std::abs(value - running_statistics_->center());

    if (stats_out_->get()) {
      if (stats_nsamples_counter == stats_nsamples_) {
        stats_out_port_->slot(0)->PublishData();
        stats_out = stats_out_port_->slot(0)->ClaimData(false);
        stats_out->set_source_timestamp();
        stats_out->set_hardware_timestamp(data_in->hardware_timestamp());
        stats_nsamples_counter = 0;
      }

      stats_out->set_data_sample(stats_nsamples_counter, 0, test_value);
      stats_out->set_data_sample(stats_nsamples_counter, 1,
                                 threshold_detector_->threshold());
      stats_out->set_sample_timestamp(stats_nsamples_counter,
                                      data_in->hardware_timestamp());

      ++stats_nsamples_counter;
    }

    if (block_ > 0) {
      --block_;
      continue;
    } else if (burst_->get()) {
      burst_->set(false);
    }

    if (threshold_detector_->has_crossed_up(test_value)) {
      block_ = static_cast<decltype(block_)>(detection_lockout_time_->get() *
                                             sample_rate_ / 1e3);
      if (stream_events_->get()) {
        event_out = data_out_port_->slot(0)->ClaimData(false);
        event_out->set_source_timestamp(data_in->source_timestamp());
        event_out->set_hardware_timestamp(data_in->hardware_timestamp());
        data_out_port_->slot(0)->PublishData();
      }
    }

    running_statistics_->add_sample(value);
    data_in_port_->slot(0)->ReleaseData();
    signal_mean_->set(running_statistics_->center());
    signal_dev_->set(running_statistics_->dispersion());
  }
}

void BurstDetector::Postprocess(ProcessingContext &context) {
  LOG(INFO) << name() << ". Streamed "
            << data_out_port_->slot(0)->nitems_produced() << " burst events.";
}

REGISTERPROCESSOR(BurstDetector)
