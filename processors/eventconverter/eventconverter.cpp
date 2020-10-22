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

#include "eventconverter.hpp"

EventConverter::EventConverter() {
  add_option("event name", event_name_, "New event name after conversion");
  add_option("replace", replace_,
             "Either replace incoming events with new event name, or generate "
             "output events by appending incoming event and new event name.");
}

void EventConverter::CreatePorts() {
  data_in_port_ = create_input_port<EventType>(EventType::Capabilities(),
                                               PortInPolicy(SlotRange(1)));

  data_out_port_ = create_output_port<EventType>(
      EventType::Capabilities(), EventType::Parameters(DEFAULT_EVENT),
      PortOutPolicy(SlotRange(1)));
}

void EventConverter::Process(ProcessingContext &context) {
  EventType::Data *data_in = nullptr;
  EventType::Data *data_out = nullptr;

  while (!context.terminated()) {
    if (!data_in_port_->slot(0)->RetrieveData(data_in)) {
      break;
    }

    data_out = data_out_port_->slot(0)->ClaimData(true);
    data_out->set_hardware_timestamp(data_in->hardware_timestamp());

    if (replace_()) {
      data_out->set_event(event_name_());
    } else {
      data_out->set_event(data_in->event() + event_name_());
    }
    data_in_port_->slot(0)->ReleaseData();
    data_out->set_source_timestamp();
    data_out_port_->slot(0)->PublishData();
  }
}

void EventConverter::Postprocess(ProcessingContext &context) {
  LOG(INFO) << name() << ". Streamed " << data_in_port_->slot(0)->status_read()
            << " events";
}

REGISTERPROCESSOR(EventConverter)