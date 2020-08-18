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

#include "levelcrossingdetector.hpp"

#include <limits>

LevelCrossingDetector::LevelCrossingDetector() : IProcessor() {
  add_option(THRESHOLD, initial_threshold_,
             "Threshold (in data units) that needs to be crossed.");
  add_option(
      UPSLOPE, initial_upslope_,
      "Either detect upward (true) or downward (false) threshold crossings.");
  add_option(POST_DETECT_BLOCK, initial_post_detect_block_,
             "Refractory period after threshold",
             "crossing detection (in number of samples).");
  add_option("event", event_prototype_,
             "The event to emit when the input signal crosses the threshold.");
}

void LevelCrossingDetector::CreatePorts() {
  data_in_port_ = create_input_port<MultiChannelType<double>>(
      "data", MultiChannelType<double>::Capabilities(ChannelRange(1, 256)),
      PortInPolicy(SlotRange(1)));

  data_out_port_ = create_output_port<EventType>(
      EVENTDATA, EventType::Capabilities(), EventType::Parameters(),
      PortOutPolicy(SlotRange(1)));

  threshold_ = create_static_state(THRESHOLD, initial_threshold_(), true,
                                   Permission::WRITE);

  upslope_ =
      create_static_state(UPSLOPE, initial_upslope_(), true, Permission::WRITE);

  post_detect_block_ = create_static_state(
      POST_DETECT_BLOCK, initial_post_detect_block_(), true, Permission::WRITE);
}

void LevelCrossingDetector::Preprocess(ProcessingContext &context) {
  double init_value;
  post_detection_block_update(initial_post_detect_block_());

  if (upslope_->get()) {
    init_value = std::numeric_limits<int>::max();
  } else {
    init_value = std::numeric_limits<int>::min();
  }

  previous_sample_.assign(data_in_port_->streaminfo(0).parameters().nchannels,
                          init_value);
}

void LevelCrossingDetector::Process(ProcessingContext &context) {
  double threshold = 0;
  bool upslope = false;
  unsigned int post_detect_block = initial_post_detect_block_();
  unsigned int post_detect_block_old = initial_post_detect_block_();
  bool crossing_detected = false;
  unsigned int nblock = 0;

  while (!context.terminated()) {
    if (!data_in_port_->slot(0)->RetrieveData(data_in_)) {
      break;
    }

    threshold = threshold_->get();
    upslope = upslope_->get();
    post_detect_block_old = post_detect_block;
    post_detect_block = post_detect_block_->get();

    if (post_detect_block_old != post_detect_block) {
      post_detection_block_update(post_detect_block);
    }

    // if blocking and post_detect_block value changed to a lower value, make
    // sure to update the current block value
    if (nblock > post_detect_block) {
      nblock = post_detect_block;
    }

    // loop through each sample
    for (unsigned int s = 0; s < data_in_->nsamples(); ++s) {
      if (nblock > 0) {
        --nblock;

        if (nblock == 0) {
          for (unsigned int c = 0; c < data_in_->nchannels(); ++c) {
            previous_sample_[c] = data_in_->data_sample(s, c);
          }
        }
        continue;
      }

      // loop through each channel
      for (unsigned int c = 0; c < data_in_->nchannels(); ++c) {
        // for up slope:
        if ((upslope && (previous_sample_[c] <= threshold) &&
             (data_in_->data_sample(s, c) > threshold)) ||
            (!upslope && (previous_sample_[c] >= threshold) &&
             (data_in_->data_sample(s, c) < threshold))) {
          crossing_detected = true;
          break;
        }
      }

      if (crossing_detected) {
        data_out_ = data_out_port_->slot(0)->ClaimData(false);
        data_out_->set_source_timestamp(data_in_->source_timestamp());
        data_out_->set_hardware_timestamp(data_in_->sample_timestamp(s));
        data_out_->set_serial_number(data_in_->serial_number());
        data_out_->set_event(event_prototype_());
        data_out_port_->slot(0)->PublishData();

        crossing_detected = false;
        ++n_detections_;

        nblock = post_detect_block;

        if ((n_detections_ % 50) == 0) {
          LOG(DEBUG) << name() << ". " << n_detections_
                     << " detections of event " << event_prototype_().event()
                     << " occurred.";
        }
      }

      for (unsigned int c = 0; c < data_in_->nchannels(); ++c) {
        previous_sample_[c] = data_in_->data_sample(s, c);
      }
    }

    data_in_port_->slot(0)->ReleaseData();
  }
}

void LevelCrossingDetector::Postprocess(ProcessingContext &context) {
  LOG(INFO) << name() << ". " << n_detections_ << " detections of event "
            << event_prototype_().event() << " occurred.";
  n_detections_ = 0;
}

void LevelCrossingDetector::post_detection_block_update(
    unsigned int post_detection_block) {
  double post_detection_block_us =
      post_detection_block /
      data_in_port_->streaminfo(0).parameters().sample_rate * 1e6;

  LOG(INFO) << name() << ". Post-detection block is set to "
            << post_detection_block_us << " microseconds.";

  if (post_detection_block_us < LOW_POST_DETECTION_BLOCK_US) {
    LOG(WARNING) << name() << ". Post-detection block might be too low!";
  }
}

REGISTERPROCESSOR(LevelCrossingDetector)
