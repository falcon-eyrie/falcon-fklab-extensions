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

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>

#include <cstdint>
#include <limits>
#include <map>
#include <string>
#include <vector>

#include "iprocessor.hpp"
#include "multichanneldata/multichanneldata.hpp"
#include "neuralynx/nlx.hpp"
#include "options/options.hpp"
#include "utilities/time.hpp"

typedef std::map<std::string, std::vector<unsigned int>> ChannelMap;

class NlxReader : public IProcessor {
  // CONSTRUCTOR and OVERLOADED METHODS
 public:
  NlxReader();
  void Configure(const GlobalContext &context) override;
  void CreatePorts() override;
  void CompleteStreamInfo() override;
  void Prepare(GlobalContext &context) override;
  void Preprocess(ProcessingContext &context) override;
  void Process(ProcessingContext &context) override;
  void Postprocess(ProcessingContext &context) override;

  // METHODS
 protected:
  /* log statistics (packet invalid, duplicated, out of order,
   * missed, gaps, filled + synchronous lag in ms) if update
   * @input condition log only when true
   */
  void print_stats(bool condition = true);

  // PORT
 protected:
  std::map<std::string, PortOut<MultiChannelType<double>> *> data_ports_;

  // CONSTANTS
 public:
  struct timeval timeout_;
  const decltype(timeout_.tv_sec) TIMEOUT_SEC = 3;
  static constexpr uint16_t MAX_NCHANNELS = 128;
  static constexpr decltype(MAX_NCHANNELS) UDP_BUFFER_SIZE =
      nlx::NLX_PACKETBYTESIZE(MAX_NCHANNELS);

  // VARIABLES
 protected:
  fd_set file_descriptor_set_;
  int udp_socket_;
  struct sockaddr_in server_addr_;

  unsigned int sample_counter_;
  uint64_t valid_packet_counter_;

  TimePoint first_valid_packet_arrival_time_;
  uint64_t timestamp_;
  uint64_t last_timestamp_;

  // UDP_BUFFER_SIZE is in bytes, so divide by size of int32_t
  char buffer_[UDP_BUFFER_SIZE];

  nlx::NlxSignalRecord nlxrecord_;
  nlx::NlxStatistics stats_;

  // OPTIONS
 protected:
  options::Value<ChannelMap, false> channelmap_;
  options::String address_{"127.0.0.1"};
  options::Value<unsigned int, false> port_{5000};
  options::Value<std::uint64_t, false> npackets_{
      0, options::zeroismax<std::uint64_t>()};
  options::Value<unsigned int, false> batch_size_{1};
  options::Value<unsigned int, false> nchannels_{nlx::NLX_DEFAULT_NCHANNELS};
  options::Measurement<std::uint64_t, false> update_interval_{
      20, "second",
      options::multiplied<std::uint64_t>(nlx::NLX_SIGNAL_SAMPLING_FREQUENCY) +
          options::zeroismax<std::uint64_t>()};
  options::Bool triggered_{false};
  options::Value<uint32_t, false> hardware_trigger_channel_{0};
};
