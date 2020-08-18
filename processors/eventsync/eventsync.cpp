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

#include "eventsync.hpp"

EventSync::EventSync() : IProcessor() {
  add_option("target event", target_event_, "The event to synchronize on.");
}

void EventSync::CreatePorts() {
  data_in_port_ = create_input_port<EventType>(
      EVENTDATA, EventType::Capabilities(), PortInPolicy(SlotRange(1, 256)));

  data_out_port_ = create_output_port<EventType>(
      EVENTDATA, EventType::Capabilities(),
      EventType::Parameters(target_event_().event()),
      PortOutPolicy(SlotRange(1)));
}

void EventSync::Process(ProcessingContext &context) {
  EventType::Data *data_in = nullptr;
  EventType::Data *data_out = nullptr;
  uint64_t target_events_counter = 0;
  timestamps_.reset();

  while (!context.terminated()) {
    for (SlotType s = 0; s < data_in_port_->number_of_slots(); ++s) {
      if (!data_in_port_->slot(s)->RetrieveData(data_in)) {
        break;
      }
      ++event_counter_.all_received;

      if (*data_in == target_event_()) {
        ++target_events_counter;
        ++event_counter_.target;
        update_latest_ts(data_in);
      } else {
        ++event_counter_.non_target;
      }
      data_in_port_->slot(s)->ReleaseData();
    }

    if (target_events_counter == data_in_port_->number_of_slots()) {
      data_out = data_out_port_->slot(0)->ClaimData(false);
      data_out->set_source_timestamp();
      data_out->set_hardware_timestamp(timestamps_.hw);
      data_out->set_event(target_event_());
      data_out_port_->slot(0)->PublishData();
      target_events_counter = 0;
      ++n_events_synced_;
      timestamps_.reset();
    }
  }
}

void EventSync::Postprocess(ProcessingContext &context) {
  log_and_reset_counters(data_in_port_->name(), event_counter_);

  LOG(INFO) << name() << ". " << n_events_synced_ << " events synced.";
  n_events_synced_ = 0;
}

void EventSync::update_latest_ts(EventType::Data *data_in) {
  if (data_in->source_timestamp() > timestamps_.source) {
    timestamps_.source = data_in->source_timestamp();
  }
  if (data_in->hardware_timestamp() > timestamps_.hw) {
    timestamps_.hw = data_in->hardware_timestamp();
  }
}

void EventSync::log_and_reset_counters(std::string port_name,
                                       EventCounter &counter) {
  auto msg = ". '" + port_name + "' counters.\n\t\t\t\t" +
             std::to_string(counter.all_received) +
             " events received.\n\t\t\t\t" + std::to_string(counter.target) +
             " target events received.\n\t\t\t\t" +
             std::to_string(counter.non_target) + " non-target events received";
  if (counter.consistent_counters()) {
    LOG(INFO) << name() << msg << ". Counters are consistent.";
  } else {
    LOG(WARNING) << name() << ". Counters are inconsistent.";
  }

  counter.reset();
}

REGISTERPROCESSOR(EventSync)
