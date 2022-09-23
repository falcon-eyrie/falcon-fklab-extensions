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
#include <vector>
#include <zmq.hpp>

#include "iprocessor.hpp"
#include "options/options.hpp"
#include "serializer.hpp"
#include "yaml-cpp/yaml.h"
#include "eventdata/eventdata.hpp"

class EventSink : public IProcessor {
  // CONSTRUCTOR and OVERLOADED METHODS
 public:
  EventSink();
  void CreatePorts() override;
  void Preprocess(ProcessingContext &context) override;
  void Process(ProcessingContext &context) override;

  // DATA PORTS
 protected:
  PortIn<EventType> *data_port_;

  // OPTIONS
 protected:
  options::Value<unsigned int, false> port_{5555};
  options::Value<unsigned int, false> ttl_{0};
  options::Value<unsigned int, false> eventid_{0};
  options::String address_{"*"};
  options::String system_{"oe"};

  // VARIABLES
 protected:
  std::unique_ptr<zmq::socket_t> socket_;
};
