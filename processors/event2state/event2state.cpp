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

#include "event2state.hpp"

Event2State::Event2State() : IProcessor() {
  add_option("target event", target_event_, "The target event that sets the state to true");
}

void Event2State::CreatePorts() {
  data_in_port_ = create_input_port<EventType>(
      EVENTDATA, EventType::Capabilities(), PortInPolicy(SlotRange(1, 256)));

  data_out_port_ = create_output_port<EventType>(
      EVENTDATA, EventType::Capabilities(),
      EventType::Parameters(target_event_().event()),
      PortOutPolicy(SlotRange(1)));

  enabled_ = create_broadcaster_state(
      "enabled", false, Permission::READ);
}

void Event2State::Process(ProcessingContext &context) {
  EventType::Data *data_in = nullptr;
  EventType::Data *data_out;

  while (!context.terminated()) {
    if (!data_in_port_->slot(0)->RetrieveData(data_in)) {
      break;
    }
    ++event_counter_.all_received;
    if (*data_in == target_event_()){
      ++event_counter_.target;
      LOG_IF(INFO, enabled_->exchange(true)) << name() << ". Mode True.";
      enabled_->set(true);

    } else {
      ++event_counter_.non_target;
      LOG_IF(DEBUG, enabled_->exchange(true)) << name() << ". Mode False.";
      enabled_->set(false);
    }
    

    data_out = data_out_port_->slot(0)->ClaimData(false);
    data_out->set_source_timestamp();
    data_out->set_hardware_timestamp(data_in->hardware_timestamp());
    data_out->set_event(target_event_());
    data_out_port_->slot(0)->PublishData();

    data_in_port_->slot(0)->ReleaseData();
  }
}

void Event2State::Postprocess(ProcessingContext &context) {
  log_and_reset_counters(data_in_port_->name(), event_counter_);
}

void Event2State::log_and_reset_counters(std::string port_name,
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

REGISTERPROCESSOR(Event2State)
