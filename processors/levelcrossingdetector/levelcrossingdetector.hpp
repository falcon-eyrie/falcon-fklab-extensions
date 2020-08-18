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
#include <vector>

#include "eventdata/eventdata.hpp"
#include "iprocessor.hpp"
#include "multichanneldata/multichanneldata.hpp"

#include "options/units.hpp"

class LevelCrossingDetector : public IProcessor {
  // CONSTRUCTOR and OVERLOADED METHODS
 public:
  LevelCrossingDetector();
  void CreatePorts() override;
  void Preprocess(ProcessingContext &context) override;
  void Process(ProcessingContext &context) override;
  void Postprocess(ProcessingContext &context) override;

  // DATA PORTS
 protected:
  PortIn<MultiChannelType<double>> *data_in_port_;
  PortOut<EventType> *data_out_port_;

  // STATES
 protected:
  StaticState<double> *threshold_;
  StaticState<bool> *upslope_;
  StaticState<unsigned int> *post_detect_block_;

  // VARIABLES
 protected:
  std::vector<double> previous_sample_;
  uint64_t n_detections_;
  MultiChannelType<double>::Data *data_in_;
  EventType::Data *data_out_;

  // METHODS
 protected:
  /* When the post detection block state is updated, it checks if it is
   * upper a certain level, log the info, and transform it in microsecs
   *
   * @input post_detection_block new value from the post detection block state
   */
  void post_detection_block_update(unsigned int post_detection_block);

  // CONSTANTS
 public:
  const std::string DEFAULT_EVENT = "threshold_crossing";
  const unsigned int LOW_POST_DETECTION_BLOCK_US = 30;
  const std::string THRESHOLD = "threshold";
  const std::string UPSLOPE = "upslope";
  const std::string POST_DETECT_BLOCK = "post detect block";

  // OPTIONS
 protected:
  options::Double initial_threshold_{0.0};
  options::Bool initial_upslope_{true};

  options::Measurement<unsigned int, false> initial_post_detect_block_{
      2, "sample"};

  options::Value<EventType::Data, false> event_prototype_{
      DEFAULT_EVENT, options::notempty<EventType::Data>()};
};
