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

#include <string>
#include <vector>

#include "eventdata/eventdata.hpp"
#include "iprocessor.hpp"
#include "options/options.hpp"

class EventSource : public IProcessor {
  // CONSTRUCTOR and OVERLOADED METHODS
 public:
  EventSource();
  void Configure(const GlobalContext &context) override;
  void CreatePorts() override;
  void Process(ProcessingContext &context) override;

  // CONSTANTS
 public:
  const double DEFAULT_EVENT_RATE = 1.0;
  const std::string DEFAULT_EVENT = "default_eventsource_event";

  // VARIABLES
 protected:
  PortOut<EventType> *event_port_;

  // OPTIONS
 protected:
  options::Vector<std::string> event_list_{
      {DEFAULT_EVENT},
      options::notempty<std::vector<std::string>>() +
          options::each<std::string>(options::notempty<std::string>())};

  options::Measurement<double> event_rate_{DEFAULT_EVENT_RATE, "Hz",
                                           options::positive<double>()};
};
