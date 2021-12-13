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
#include "timeseriesdata/timeseriesdata.hpp"
#include "recording_utilities/channellist.hpp"
#include "options/options.hpp"

typedef std::map<std::string, ChannelList<unsigned int>> ChannelMap;

class ColumnSplitter : public IProcessor {
  // CONSTRUCTOR and OVERLOADED METHODS
 public:
  ColumnSplitter();
  void CreatePorts() override;
  void CompleteStreamInfo() override;
  void Process(ProcessingContext &context) override;
  void Postprocess(ProcessingContext &context) override;

  // PORTS
 protected:
  PortIn<TimeSeriesType<double>> *input_port_;
  PortOut<TimeSeriesType<double>> *output_port_;

  // variables
 protected:
  unsigned int incoming_batch_size_;
  unsigned int max_n_channels_;
  unsigned int n_channel_by_group_;

  // constants
 protected:
  const unsigned int MAX_N_CHANNELS = 4096;

  // OPTIONS
 protected:
  options::Double ngroups_;
};
