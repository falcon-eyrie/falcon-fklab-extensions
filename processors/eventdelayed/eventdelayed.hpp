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

#include "eventconverter/eventconverter.hpp"
#include "eventdata/eventdata.hpp"
#include "iprocessor.hpp"
#include "utilities/general.hpp"
#include "utilities/math_numeric.hpp"
#include "utilities/time.hpp"
#include <queue>
#include <string>

struct Delayed {
  TimePoint ts;
  EventType::Data *data_in;

  Delayed(TimePoint ts, EventType::Data *data_in) : ts(ts), data_in(data_in) {}
  bool operator>(const Delayed &test) const { return (ts > test.ts); }
};

class EventDelayed : public IProcessor {
  // CONSTRUCTOR and OVERLOADED METHODS
public:
  EventDelayed();
  void CreatePorts() override;
  void Configure(const GlobalContext &context) override;
  void Preprocess(ProcessingContext &context) override;
  void Process(ProcessingContext &context) override;
  void Postprocess(ProcessingContext &context) override;

  /* Use with delayed event check if enough time will have passed between this stimulation and the last occuring stimulation.
   * if still in the look out period, the event is ignored.
   * @input TimePoint when the new stimulation should occur in the future. 
   * @return true if still in the look out period
   */
  bool to_lock_out_in_future(TimePoint start_event);
  /* check if enough time has passed since the last triggered event. if still
   * in the look out period, the event is ignored.
   *
   * @return true if still in the look out period
   */
  bool to_lock_out();

  // DATA PORTS
protected:
  PortIn<EventType> *data_in_port_;
  PortOut<EventType> *output_port_;

  // STATES
protected:
  FollowerState<bool> *disabled_;
  FollowerState<bool> *delayed_event_;
  StaticState<double> *stop_detection_period_;
  StaticState<double>  *stop_analysis_period_;
  BroadcasterState<bool> *analysis_unlocked_;

  // OPTIONS
protected:
  options::Bool default_disabled_{false};
  options::Measurement<double, false> initial_stop_detection_period_{
      50, "ms", options::positive<double>(false)};
  options::Value<std::vector<int>, true> when_stop_analysis_period_{{0, 0}};
  options::Measurement<double, false> initial_stop_analysis_period_{
      50., "ms", options::positive<double>(false)};
  options::Bool start_after_detection_{false};
  options::Bool start_after_stimulation_{true};
  options::Bool initial_delayed_event_{false};
  options::Bool save_events_{true};
  options::String prefix_{"stim_"};
  options::String msg_delayed_{"d"};
  options::String msg_detection_{"r"};
  options::String msg_ontime_{"o"};
  options::Value<std::vector<long int>, true> initial_delayed_range_{{150, 200}};

private:
  void send_event(EventType::Data *data_in, std::string type);
  // variables
protected:
  uint64_t ontime_received_event_;
  uint64_t delayed_received_event_;
  uint64_t event_lockout_;

  Range<long int> delayed_range_;
  TimePoint previous_TS_nostim_;

  std::priority_queue<Delayed, std::vector<Delayed>, std::greater<Delayed>>
      delayed_event_queue_;

  std::priority_queue<Delayed, std::vector<Delayed>, std::greater<Delayed>>
      lockout_queue_;


  // CONSTANT
protected:
  const std::string DISABLED_S = "detection only mode";
  const std::string DELAYED_S = "delayed mode";
  const std::string STOP_DETECTION_TIME_S = "event trigger lockout time";
  const std::string STOP_ANALYSIS_TIME_S = "analysis lockout time" ;
};
