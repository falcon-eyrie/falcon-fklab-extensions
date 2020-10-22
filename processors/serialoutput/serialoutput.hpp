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
#include "eventdata/eventdata.hpp"
#include "iprocessor.hpp"
#include "options/options.hpp"
#include "options/units.hpp"
#include "serialib/lib/serialib.h"

#include <fstream>

class SerialOutput : public IProcessor {

  // CONSTRUCTOR and OVERLOADED METHODS
public:
  SerialOutput();
  void CreatePorts() override;
  void Preprocess(ProcessingContext &context) override;
  void Process(ProcessingContext &context) override;
  void Postprocess(ProcessingContext &context) override;

  // DATA PORTS AND STATES
protected:
  PortIn<EventType> *data_in_port_;
  PortOut<EventType> *data_out_port_;

  // OPTIONS
protected:
  options::String port_address_{"/dev/ttyACM0"};
  options::Int baudrate_{9600};
  options::Bool event_log_{true};

  // variables
protected:
  serialib fd_;
};

