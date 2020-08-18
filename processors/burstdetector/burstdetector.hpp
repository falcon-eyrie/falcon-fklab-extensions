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
#include "muadata/muadata.hpp"
#include "multichanneldata/multichanneldata.hpp"

class BurstDetector : public IProcessor {
  // CONSTRUCTOR and OVERLOADED METHODS
 public:
  BurstDetector();
  void CreatePorts() override;
  void CompleteStreamInfo() override;
  void Preprocess(ProcessingContext &context) override;
  void Process(ProcessingContext &context) override;
  void Postprocess(ProcessingContext &context) override;

  // PORTS
 protected:
  PortIn<MUAType> *data_in_port_;
  PortOut<EventType> *data_out_port_;
  PortOut<MultiChannelType<double>> *stats_out_port_;

  // STATES
 protected:
  BroadcasterState<double> *threshold_;
  BroadcasterState<double> *signal_mean_;
  BroadcasterState<double> *signal_dev_;
  BroadcasterState<bool> *burst_;
  FollowerState<double> *bin_size_mua_;

  StaticState<double> *threshold_dev_;
  StaticState<double> *smooth_time_;
  StaticState<double> *detection_lockout_time_;
  StaticState<bool> *stream_events_;
  StaticState<bool> *stats_out_;

  bool stats_out_enabled_;
  std::uint64_t stats_nsamples_;

  std::uint64_t block_;
  std::uint64_t burn_in_;
  double sample_rate_;

  double acc_;

  std::unique_ptr<dsp::algorithms::RunningMeanMAD> running_statistics_;
  std::unique_ptr<dsp::algorithms::ThresholdCrosser> threshold_detector_;

 public:
  // configure options names ( keep common between options and states )
  const std::string THRESHOLD_DEV = "threshold dev";
  const std::string SMOOTH_TIME = "smooth time";
  const std::string DETECTION_LOCKOUT_TIME = "detection lockout time";
  const std::string STREAM_EVENTS = "stream events";
  const std::string STREAM_STATISTICS = "stream statistics";

 protected:
  const unsigned int N_STATS_OUT = 2;

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
};
