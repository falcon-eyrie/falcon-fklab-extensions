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
  void Configure(const YAML::Node &node, const GlobalContext &context) override;
  void Preprocess(ProcessingContext &context) override;
  void Process(ProcessingContext &context) override;
  void Postprocess(ProcessingContext &context) override;

  /* check if enough time has passed since the last triggered event. if still
   * in the look out period, the event is ignored.
   *
   * @return true if still in the look out period
   */
  bool to_lock_out();

  /* Wrote in a file (in append mode) the data
   *
   * @input file_path path where to write the file (in the context of this
   * processor - run context path)
   * @input filename  (in the context of this processor - prefix option + event
   * name)
   * @input data data to write (in the context of this processor - the
   * timestamp)
   *
   */
  void write_data_logfile(std::string file_path, std::string filename,
                          uint64_t data);
  // DATA PORTS
protected:
  PortIn<EventType> *data_in_port_;
  PortOut<EventType> *data_out_port_;

  // STATES
protected:
  FollowerState<bool> *disabled_;
  FollowerState<bool> *delayed_event_;
  StaticState<double> *lockout_period_;

  // OPTIONS
protected:
  options::Bool default_disabled_{false};
  options::Measurement<double, false> initial_lockout_period_{
      50, "ms", options::positive<double>(true)};
  options::Bool initial_delayed_event_{false};
  options::Bool save_events_{true};
  options::String prefix_{"stim_"};


  options::Value<std::vector<long int>, true> initial_delayed_range_{{150, 200}};


private:
  void send_event(EventType::Data *data_in, EventType::Data *data_out,std::string type,
                  std::string filepath);
  // variables
protected:
  uint64_t ontime_received_event_;
  uint64_t delayed_received_event_;
  uint64_t event_lockout_;

  Range<long int> delayed_range_;
  TimePoint previous_TS_nostim_;

  std::priority_queue<Delayed, std::vector<Delayed>, std::greater<Delayed>>
      event_queue_;

  // CONSTANT
protected:
  const std::string ENABLED_S = "disabled";
  const std::string LOCKOUT_PERIOD_S = "lockout period";
};