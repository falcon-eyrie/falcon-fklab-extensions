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
  options::Value<unsigned int, false> data_port_{
      3335, options::positive<unsigned int>(true)};
  options::Value<std::uint64_t, false> npackets_{
      0, options::zeroismax<std::uint64_t>()};
  options::Value<unsigned int, false> nchannels_{384, options::positive<unsigned int>(true)};
  options::Value<unsigned int, false> sampling_rate_{30000, options::positive<unsigned int>(true)};
  // PORT
protected:
  PortOut<MultiChannelType<unsigned int>> *data_out_port_;

  // VARIABLES
protected:
  zmq::socket_t data_socket_;
  int last_message_number;
  uint64_t valid_packet_counter_;
  TimePoint first_valid_packet_arrival_time_;
  uint64_t data_corrupted_counter_;
};
