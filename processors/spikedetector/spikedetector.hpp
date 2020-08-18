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

#include <memory>
#include <string>

#include "dsp/algorithms.hpp"
#include "eventdata/eventdata.hpp"
#include "iprocessor.hpp"
#include "multichanneldata/multichanneldata.hpp"
#include "options/options.hpp"
#include "spikedata/spikedata.hpp"

class SpikeDetector : public IProcessor {
  // CONSTRUCTOR and OVERLOADED METHODS
 public:
  SpikeDetector();
  void CreatePorts() override;
  void CompleteStreamInfo() override;
  void Prepare(GlobalContext &context) override;
  void Process(ProcessingContext &context) override;
  void Postprocess(ProcessingContext &context) override;

  // PORTS
 protected:
  PortIn<MultiChannelType<double>> *data_in_port_;
  PortOut<SpikeType> *data_out_port_spikes_;
  PortOut<EventType> *data_out_port_events_;

  // STATES
 protected:
  StaticState<double> *threshold_;
  StaticState<unsigned int> *peak_lifetime_;

  // VARIABLES
 protected:
  unsigned int n_channels_;
  size_t n_incoming_;
  size_t incoming_buffer_size_samples_;
  uint64_t n_streamed_events_;

  std::unique_ptr<dsp::algorithms::SpikeDetector> spike_detector_;
  std::unique_ptr<MultiChannelType<double>::Data> inverted_signals_;

  // CONSTANTS
 public:
  unsigned int MAX_N_CHANNELS = 8;
  const std::string PEAK_LIFETIME = "peak lifetime";
  const std::string THRESHOLD = "threshold";
  const int RINGBUFFER_SIZE = 1e5;

  // OPTIONS
 protected:
  options::Double initial_threshold_{60.};
  options::Bool invert_signal_{true};
  options::Measurement<double, false> buffer_size_{
      0.5, "ms", options::positive<double>(true)};
  options::Bool strict_time_bin_check_{true};
  options::Measurement<unsigned int, false> initial_peak_lifetime_{8, "sample"};
};
