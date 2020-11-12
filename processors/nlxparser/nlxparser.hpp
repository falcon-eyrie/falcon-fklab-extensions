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
#include <vector>
#include <string>
#include <limits>

#include "iprocessor.hpp"
#include "multichanneldata/multichanneldata.hpp"
#include "neuralynx/nlx.hpp"
#include "options/options.hpp"
#include "utilities/time.hpp"
#include "vectordata/vectordata.hpp"

enum class GapFill { NONE = 0, ASAP, DISTRIBUTED };

std::string gapfill_to_string(GapFill x);
GapFill string_to_gapfill(std::string s);

namespace YAML {
template <> struct convert<GapFill> {
  static Node encode(const GapFill &rhs) {
    Node node;
    node = gapfill_to_string(rhs);
    return node;
  }

  static bool decode(const Node &node, GapFill &rhs) {
    rhs = string_to_gapfill(node.as<std::string>());
    return true;
  }
};
}  // namespace YAML

class NlxParser : public IProcessor {
 public:
  NlxParser();
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

  // VARIABLES
 protected:
  unsigned int nchannels_;
  unsigned int sample_counter_;
  uint64_t valid_packet_counter_;
  TimePoint first_valid_packet_arrival_time_;
  uint64_t timestamp_;
  uint64_t last_timestamp_;
  nlx::NlxSignalRecord nlxrecord_;
  nlx::NlxStatistics stats_;
  std::vector<unsigned int> channel_list_;
  int64_t n_filling_packets_;

  // PORTS
 protected:
  PortOut<MultiChannelType<double>> *output_port_signal_;
  PortOut<MultiChannelType<uint32_t>> *output_port_ttl_;
  PortIn<VectorType<uint32_t>> *data_in_port_;

  // STATES
 protected:
  BroadcasterState<uint64_t> *n_invalid_;

  // OPTIONS
 protected:
  options::Value<unsigned int, false> batch_size_{2};
  options::Measurement<std::uint64_t, false> update_interval_{
      20, "second",
      options::multiplied<std::uint64_t>(nlx::NLX_SIGNAL_SAMPLING_FREQUENCY) +
          options::zeroismax<std::uint64_t>()};
  options::Bool triggered_{false};
  options::Value<uint32_t, false> hardware_trigger_channel_{0};
  options::Value<GapFill, false> gap_fill_{GapFill::ASAP};
};
