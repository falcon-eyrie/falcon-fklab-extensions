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

#include <map>
#include <string>
#include <vector>

#include "iprocessor.hpp"
#include "multichanneldata/multichanneldata.hpp"
#include "options/options.hpp"

typedef std::map<std::string, std::vector<unsigned int>> ChannelMap;

class Distributor : public IProcessor {
  // CONSTRUCTOR and OVERLOADED METHODS
 public:
  Distributor();
  void CreatePorts() override;
  void CompleteStreamInfo() override;
  void Prepare(GlobalContext &context) override;
  void Process(ProcessingContext &context) override;
  void Postprocess(ProcessingContext &context) override;

  // PORTS
 protected:
  PortIn<MultiChannelType<double>> *input_port_;
  std::map<std::string, PortOut<MultiChannelType<double>> *> data_ports_;

  // variables
 protected:
  unsigned int incoming_batch_size_;
  unsigned int max_n_channels_;

  // constants
 protected:
  const unsigned int MAX_N_CHANNELS = 4096;
  // maximum number of channels that the distributor can handle
  const int BUFFER_SIZE = 2000;  // ring buffer size on the output ports
  const WaitStrategy WAIT_STRATEGY = WaitStrategy::kBlockingStrategy;

  // OPTIONS
 protected:
  options::Value<ChannelMap, false> channelmap_;
};
