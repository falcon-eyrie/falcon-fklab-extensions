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
// Falcon-sever is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with falcon-core. If not, see <http://www.gnu.org/licenses/>.
// ---------------------------------------------------------------------

#pragma once
#include <string>

#include "iprocessor.hpp"
#include "muadata/muadata.hpp"
#include "options/units.hpp"
#include "spikedata/spikedata.hpp"

class MUAEstimator : public IProcessor {
  // CONSTRUCTOR and OVERLOADED METHODS
 public:
  MUAEstimator();
  void CreatePorts() override;
  void CompleteStreamInfo() override;
  void Prepare(GlobalContext &context) override;
  void Process(ProcessingContext &context) override;

  // DATA PORTS
 protected:
  PortIn<SpikeType> *data_in_port_;
  PortOut<MUAType> *data_out_port_;

  // STATES
 protected:
  StaticState<double> *bin_size_;
  BroadcasterState<double> *mua_;

  // VARIABLES
 protected:
  double current_bin_size_;
  double previous_bin_size_;
  double spike_buffer_size_;
  std::size_t n_spike_buffers_;

  // CONSTANTS
 public:
  const std::string BIN_SIZE = "bin size";

  // OPTIONS
 protected:
  options::Measurement<double, false> initial_bin_size_{
      10., "ms", options::positive<double>(true)};
};
