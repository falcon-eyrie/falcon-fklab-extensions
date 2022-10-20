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
#include "math_expression_parser/exprtk.hpp"
#include "eventdata/eventdata.hpp"
#include "iprocessor.hpp"

class EventMerger : public IProcessor {
  // CONSTRUCTOR and OVERLOADED METHODS
public:
  EventMerger();
  void CreatePorts() override;
  void CompleteStreamInfo() override;
  void Prepare(GlobalContext &context) override;
  void Process(ProcessingContext &context) override;
  void Postprocess(ProcessingContext &context) override;

  // DATA PORTS
protected:
  PortIn<EventType> *data_in_port_;
  PortOut<EventType> *data_out_port_;

  exprtk::expression<double> equations_;
  std::vector<double>* inputs_;

  // OPTIONS
protected:
  options::String binary_equation_{"equation", options::notempty<std::string>()};

};
