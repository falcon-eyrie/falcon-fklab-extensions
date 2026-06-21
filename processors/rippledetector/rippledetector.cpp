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

#include "rippledetector.hpp"

#include <algorithm>

RippleDetector::RippleDetector() : IProcessor() {
    add_option(THRESHOLD_DEV, initial_threshold_dev_,
               "Multiplier (in number of signal standard deviations) to "
               "compute the initial threshold.");
    add_option(SMOOTH_TIME, initial_smooth_time_,
               "Integration time for estimating signal statistics.");
    add_option(DETECTION_LOCKOUT_TIME, initial_detection_lockout_time_,
               "Lockout time (in ms) to avoid over-stimulation.");
    add_option(STREAM_EVENTS, default_stream_events_, "Enable streaming of ripple events.");
    add_option(STREAM_STATISTICS, initial_stats_out_, "Enable streaming of statistics.");
    add_option("statistics_buffer_size", stats_buffer_size_,
               "Size (in seconds) for statistics output buffers.");
    add_option("statistics_downsample_factor", stats_downsample_factor_,
               "Downsample factor of streamed statistics signal");
    add_option("use_power", use_power_, "Use power of signal for detection.");
}

void RippleDetector::CreatePorts() {
    data_in_port_ = create_input_port<TimeSeriesType<double>>(
        "in_signal", TimeSeriesType<double>::Capabilities(ChannelRange(1, 256)),
        PortInPolicy(SlotRange(1)));

    event_out_port_ = create_output_port<EventType>(EVENTDATA, EventType::Parameters("ripple"),
                                                    PortOutPolicy(SlotRange(1)));

    stats_out_port_ = create_output_port<TimeSeriesType<double>>(
        "statistics", TimeSeriesType<double>::Parameters(), PortOutPolicy(SlotRange(1)));

    threshold_ = create_producer_state("threshold", 0.0, false, Permission::READ);

    signal_mean_ = create_producer_state("mean", 0.0, false, Permission::READ);

    signal_dev_ = create_producer_state("deviation", 0.0, false, Permission::READ);

    threshold_dev_ =
        create_static_state(THRESHOLD_DEV, initial_threshold_dev_(), true, Permission::WRITE);

    detection_lockout_time_ = create_static_state(
        DETECTION_LOCKOUT_TIME, initial_detection_lockout_time_(), true, Permission::WRITE);

    detection_enabled_ = create_follower_state("detection enabled", true, Permission::NONE);

    stream_events_ =
        create_static_state(STREAM_EVENTS, default_stream_events_(), true, Permission::WRITE);

    smooth_time_ =
        create_static_state(SMOOTH_TIME, initial_smooth_time_(), true, Permission::WRITE);

    stats_out_ =
        create_static_state(STREAM_STATISTICS, initial_stats_out_(), true, Permission::WRITE);

    ripple_ = create_broadcaster_state("ripple", false, Permission::READ);
}

void RippleDetector::CompleteStreamInfo() {
    stats_nsamples_ = stats_buffer_size_() * data_in_port_->prototype(0).sample_rate() /
                      stats_downsample_factor_();
    stats_nsamples_ = std::max(stats_nsamples_, 1UL);

    stats_out_port_->streaminfo(0).set_parameters(TimeSeriesType<double>::Parameters(
        STATS_LABEL, stats_nsamples_,
        data_in_port_->prototype(0).sample_rate() / stats_downsample_factor_()));
    stats_out_port_->streaminfo(0).set_stream_parameters(data_in_port_->streaminfo(0));
}

void RippleDetector::Preprocess(ProcessingContext& _) {
    signal_mean_->set(0);
    signal_dev_->set(0);
    threshold_->set(0);
    block_ = 0;
    sample_rate_ = data_in_port_->prototype(0).sample_rate();
    burn_in_ = initial_smooth_time_() * sample_rate_;
    double alpha = 1.0 / burn_in_;

    running_statistics_.reset(new dsp::algorithms::RunningMeanMAD(alpha, burn_in_, false));
    threshold_detector_.reset(new dsp::algorithms::ThresholdCrosser(0));
}

void RippleDetector::Process(ProcessingContext& context) {
    TimeSeriesType<double>::Data* data_in = nullptr;
    EventType::Data* event_out = nullptr;
    TimeSeriesType<double>::Data* stats_out = nullptr;
    double value, test_value;
    auto stats_nsamples_counter = stats_nsamples_;
    unsigned int stats_skip_counter = 0;
    auto burnin_update_sent = false;

    // burn-in period
    while (running_statistics_->is_burning_in() && !context.terminated()) {
        if (!data_in_port_->slot(0)->RetrieveData(data_in)) {
            break;
        }

        if (!burnin_update_sent) {
            LOG(UPDATE) << name() << ": burn-in period starting (" << initial_smooth_time_()
                        << " seconds)";
            burnin_update_sent = true;
        }
        for (unsigned int sample = 0; sample < data_in->nsamples(); ++sample) {
            running_statistics_->add_sample(compute_value(data_in, sample));
        }

        data_in_port_->slot(0)->ReleaseData();
    }

    if (!running_statistics_->is_burning_in()) {
        LOG(UPDATE) << name() << ": end of burn-in period";
        LOG(UPDATE) << name() << ": statistics: center = " << running_statistics_->center()
                    << ", dispersion = " << running_statistics_->dispersion();

        LOG(UPDATE) << name() << ": ripple detection starts now with initial threshold of "
                    << (threshold_dev_->get() * running_statistics_->dispersion());
    }

    // ripple detection
    while (!context.terminated()) {
        // retrieve new data
        if (!data_in_port_->slot(0)->RetrieveData(data_in)) {
            break;
        }

        // update threshold and alpha only once for an incoming data bucket
        threshold_->set(threshold_dev_->get() * running_statistics_->dispersion());
        threshold_detector_->set_threshold(threshold_->get());
        running_statistics_->set_alpha(1.0 / (smooth_time_->get() * sample_rate_));

        // loop through each sample
        for (unsigned int sample_index = 0; sample_index < data_in->nsamples(); sample_index++) {
            value = compute_value(data_in, sample_index);
            test_value = std::abs(value - running_statistics_->center());

            if (stats_out_->get()) {
                if (stats_nsamples_counter == stats_nsamples_) {
                    stats_out_port_->slot(0)->PublishData();
                    stats_out = stats_out_port_->slot(0)->ClaimData(false);
                    stats_out->set_source_timestamp(data_in->source_timestamp());
                    stats_out->set_hardware_timestamp(data_in->sample_timestamp(sample_index));
                    stats_nsamples_counter = 0;
                }

                if (stats_skip_counter == 0) {
                    stats_out->set_data_sample(stats_nsamples_counter, "statistics", test_value);
                    stats_out->set_data_sample(stats_nsamples_counter, "threshold",
                                               threshold_detector_->threshold());
                    stats_out->set_data_sample(stats_nsamples_counter, "deviation",
                                               running_statistics_->dispersion());
                    stats_out->set_sample_timestamp(stats_nsamples_counter,
                                                    data_in->sample_timestamp(sample_index));
                    stats_skip_counter = stats_downsample_factor_();
                    ++stats_nsamples_counter;
                }
                --stats_skip_counter;
            }

            if (block_ > 0) {  // post-detection lock-out time
                --block_;
                continue;
            } else if (!detection_enabled_->get()) {
                continue;
            } else if (ripple_->get()) {
                ripple_->set(false);
            }

            if (threshold_detector_->has_crossed_up(test_value)) {
                block_ = static_cast<decltype(block_)>(detection_lockout_time_->get() *
                                                       sample_rate_ / 1e3);
                if (stream_events_->get()) {
                    event_out = event_out_port_->slot(0)->ClaimData(false);
                    event_out->set_source_timestamp(data_in->source_timestamp());
                    event_out->set_hardware_timestamp(data_in->sample_timestamp(sample_index));
                    event_out->forward_ingestion_ns(*data_in);
                    event_out_port_->slot(0)->PublishData();
                }
            }

            running_statistics_->add_sample(value);
        }
        data_in_port_->slot(0)->ReleaseData();
        signal_mean_->set(running_statistics_->center());
        signal_dev_->set(running_statistics_->dispersion());
    }
}

void RippleDetector::Postprocess(ProcessingContext& _) {
    LOG(INFO) << name() << ". Streamed " << event_out_port_->slot(0)->nitems_produced()
              << " ripple events.";
}

inline double RippleDetector::compute_value(TimeSeriesType<double>::Data* data_in,
                                            unsigned int sample) {
    if (use_power_()) {
        // Grab first column pointer for this row slice
        auto c = data_in->begin_sample(sample);
        const auto end = data_in->end_sample(sample);

        // Square the first channel directly
        acc_ = (*c) * (*c);

        // Loop horizontally across remaining channels
        for (++c; c != end; ++c) {
            acc_ += (*c) * (*c);  // Accumulate squared voltage
        }

        // Flatten to spatial average power
        return acc_ / data_in->ncolumns();
    }

    // Fallback: simple spatial mean across channels
    return data_in->mean_sample(sample);
}

REGISTERPROCESSOR(RippleDetector)
