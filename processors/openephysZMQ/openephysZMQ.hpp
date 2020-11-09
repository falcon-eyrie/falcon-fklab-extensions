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

#include "iprocessor.hpp"
#include "multichanneldata/multichanneldata.hpp"
#include "utilities/zmqutil.hpp"
#include <nlohmann/json.hpp>
// for convenience
using json = nlohmann::json;



typedef std::map<std::string, std::vector<unsigned int>> ChannelMap;
const int OPEN_EPHYS_SIGNAL_SAMPLING_FREQUENCY = 200;

class OpenEphysZMQ : public IProcessor {
  // CONSTRUCTOR and OVERLOADED METHODS
public:
  OpenEphysZMQ();
  void Preprocess(ProcessingContext &context) override;
  void CreatePorts() override;
  void CompleteStreamInfo() override;
  void Process(ProcessingContext &context) override;
  void Postprocess(ProcessingContext &context) override;

protected:
  options::String address_{"127.0.0.1", options::notempty<std::string>()};
  options::Value<unsigned int, false> data_port_{5556,
                                            options::positive<unsigned int>(true)};
  options::Value<unsigned int, false> heartbeat_port_{5557,
                                                 options::positive<unsigned int>(true)};

  options::Value<ChannelMap, false> channelmap_;
  options::Value<std::uint64_t, false> npackets_{
      0, options::zeroismax<std::uint64_t>()};
  options::Value<unsigned int, false> batch_size_{5};

// PORT
protected:
  std::map<std::string, PortOut<MultiChannelType<float>> *> data_ports_;
  std::map<int, std::vector<float> *> samples_;
  unsigned int sample_counter_;
  uint64_t valid_packet_counter_;
  TimePoint first_valid_packet_arrival_time_;
  std::vector<uint64_t>* timestamps;


  // VARIABLES
protected:
  zmq::socket_t data_socket_;
  uint64_t  data_counter_;
  int  last_message_number = -1;
  uint64_t  data_corrupted_counter_;
};