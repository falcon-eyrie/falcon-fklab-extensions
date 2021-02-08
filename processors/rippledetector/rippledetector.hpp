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

#pragma once

#include <memory>
#include <string>

#include "dsp/algorithms.hpp"
#include "eventdata/eventdata.hpp"
#include "iprocessor.hpp"
#include "multichanneldata/multichanneldata.hpp"
#include "options/options.hpp"
#include "options/units.hpp"

class RippleDetector : public IProcessor {
  // CONSTRUCTOR and OVERLOADED METHODS
 public:
  RippleDetector();
  void CreatePorts() override;
  void CompleteStreamInfo() override;
  void Preprocess(ProcessingContext &context) override;
  void Process(ProcessingContext &context) override;
  void Postprocess(ProcessingContext &context) override;

  // METHODS
 protected:
  double compute_value(MultiChannelType<double>::Data *data_in,
                       unsigned int sample);

  // DATA PORTS
 protected:
  PortIn<MultiChannelType<double>> *data_in_port_;
  PortOut<EventType> *event_out_port_;
  PortOut<MultiChannelType<double>> *stats_out_port_;

  // STATES
 protected:
  ProducerState<double> *threshold_;
  ProducerState<double> *signal_mean_;
  ProducerState<double> *signal_dev_;
  BroadcasterState<bool> *ripple_;
  StaticState<double> *threshold_dev_;
  StaticState<double> *detection_lockout_time_;
  FollowerState<bool> * detection_enabled_;
  StaticState<bool> *stream_events_;
  StaticState<double> *smooth_time_;
  StaticState<bool> *stats_out_;

  // VARIABLES
 protected:
  bool stats_out_enabled_;
  std::uint64_t stats_nsamples_;
  std::uint64_t block_;
  std::uint64_t burn_in_;
  double sample_rate_;
  double acc_;
  std::unique_ptr<dsp::algorithms::RunningMeanMAD> running_statistics_;
  std::unique_ptr<dsp::algorithms::ThresholdCrosser> threshold_detector_;

  // CONSTANTS
 public:
  const unsigned int N_STATS_OUT = 2;
  const std::string THRESHOLD_DEV = "threshold dev";
  const std::string SMOOTH_TIME = "smooth time";
  const std::string DETECTION_LOCKOUT_TIME = "analysis lockout time";
  const std::string STREAM_EVENTS = "stream events";
  const std::string STREAM_STATISTICS = "stream statistics";

  // OPTIONS
 protected:
  options::Double initial_threshold_dev_{6.};
  options::Measurement<double, false> initial_smooth_time_{
      10., "second", options::positive<double>(true)};
  options::Measurement<double, false> initial_detection_lockout_time_{
      30., "ms", options::positive<double>(true)};

  options::Bool default_stream_events_{true};
  options::Bool initial_stats_out_{true};
  options::Measurement<double, false> stats_buffer_size_{
      0.5, "second", options::positive<double>(true)};
  options::Value<unsigned int, false> stats_downsample_factor_{
      1, options::positive<unsigned int>(true)};
  options::Bool use_power_{true};
};
