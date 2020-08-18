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

#include <chrono>
#include <limits>
#include <tuple>
#include <vector>

#include "eventsync/eventsync.hpp"
#include "options/options.hpp"
#include "utilities/time.hpp"

class DetectionCriterionValue : public options::Value<SlotType, false> {
 public:
  using options::Value<SlotType, false>::Value;
  void from_yaml(const YAML::Node &node) override;
};

class EventFilter : public EventSync {
  // CONSTRUCTOR and OVERLOADED METHODS
 public:
  EventFilter();
  void CreatePorts() override;
  void Prepare(GlobalContext &context) override;
  void Preprocess(ProcessingContext &context) override;
  void Process(ProcessingContext &context) override;
  void Postprocess(ProcessingContext &context) override;

  // METHODS
 protected:
  /* Check for every slot after a read on the input port if a target is found
   * @input input_port input port to read
   * @input event_counter structure with three counters (all received, target,
   * non target) updated when meeting events
   * @input arrival_times keep track of every event arrival in the processor
   * @input arrival_timestamps keeps track of every event timestamp hardware
   *
   * @return the alive status of the processor, the flag of found target and
   * the timestamp of the target event detected
   */
  std::tuple<bool, bool, std::size_t>
  is_there_target(PortIn<EventType> *input_port, EventCounter &event_counter,
                  std::vector<TimePoint> &arrival_times,
                  std::vector<uint64_t> &arrival_timestamps);

  // return time in milliseconds past from two given time points t1 and t2
  inline double time_between(TimePoint t2, TimePoint t1) {
    duration = t2 - t1;
    return duration.count();
  }

  // return the time in milliseconds past from a given time point t
  inline double time_since(TimePoint t) {
    return time_between(Clock::now(), t);
  }

  // DATA PORTS
 protected:
  PortIn<EventType> *block_in_port_;

  // VARIABLES
 protected:
  unsigned int n_blocked_events_;
  EventCounter blocking_events_counter_;
  TimePoint gate_close_time_;
  std::chrono::duration<double, std::milli> duration;

  // CONSTANTS
 protected:
  const uint64_t NULL_TIMESTAMP = std::numeric_limits<uint64_t>::max();

  // OPTIONS
 protected:
  options::Measurement<double, false> blockout_time_{
      10., "ms", options::positive<double>()};

  options::Measurement<double, false> block_wait_time_{
      1.5, "ms", options::positive<double>()};

  options::Measurement<double, false> sync_time_{3.5, "ms",
                                                 options::positive<double>()};

  options::Bool discard_warnings_{false};
  DetectionCriterionValue detections_to_criterion_{1};
};
