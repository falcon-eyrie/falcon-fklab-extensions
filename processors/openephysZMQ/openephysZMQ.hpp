// ---------------------------------------------------------------------
// This file is part of falcon-core.
//
// Copyright (C) 2021 - present Neuro-Electronics Research Flanders
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
#include "channel_generated.h"

#include "multichanneldata/multichanneldata.hpp"
#include "utilities/zmqutil.hpp"
#include "flatbuffers/flatbuffers.h"


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
  // OPTIONS
  options::String address_{"127.0.0.1", options::notempty<std::string>()};
  options::Value<unsigned int, false> port_{5556, options::positive<unsigned int>(true)};
  options::Value<std::uint64_t, false> npackets_{0, options::zeroismax<std::uint64_t>()};
  options::Value<unsigned int, false> batch_size_{1};
  options::Value<unsigned int, false> nchannels_{384, options::positive<unsigned int>(true)};

  // PORT
protected:
  PortOut<MultiChannelType<double>>* data_port_;

  // VARIABLES
protected:
  zmq::socket_t socket_;

  uint64_t last_message_number_;
  uint64_t missing_packets_counter_;
  uint64_t valid_packets_counter_;
  uint64_t invalid_packets_counter_;
  TimePoint first_valid_packet_arrival_time_;

  flatbuffers::FlatBufferBuilder flatbuilder_;
  openephysflatbuffer::ContinuousDataBuilder builder_;

};
