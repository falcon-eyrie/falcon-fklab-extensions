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

#include "eventsource.hpp"

#include <chrono>
#include <random>
#include <thread>

EventSource::EventSource() : IProcessor() {
  add_option("events", event_list_, "List of events to generate.");
  add_option("rate", event_rate_,
             "Rate (in Hz) at which events are generated.");
}

void EventSource::Configure(const GlobalContext &context) {
  for (auto &el : event_list_()) {
    LOG(INFO) << name() << ". Event " << el << " configured for streaming.";
  }

  LOG(INFO) << name() << ". Event rate set to " << event_rate_.to_string();
}

void EventSource::CreatePorts() {
  event_port_ = create_output_port<EventType>(
      EVENTDATA, EventType::Capabilities(),
      EventType::Parameters(DEFAULT_EVENT), PortOutPolicy(SlotRange(1)));
}

void EventSource::Process(ProcessingContext &context) {
  EventType::Data *data = nullptr;

  if (event_list_().size() == 0) {
    return;
  }

  std::default_random_engine generator;
  std::uniform_int_distribution<unsigned int> distribution(
      0, event_list_().size() - 1);

  auto delay = std::chrono::milliseconds(
      static_cast<unsigned int>(1000.0 / event_rate_()));

  while (!context.terminated()) {
    std::this_thread::sleep_for(delay);
    data = event_port_->slot(0)->ClaimData(false);
    data->set_source_timestamp();
    data->set_hardware_timestamp(static_cast<uint64_t>(
        data->time_since(context.run().start_time()).count()));

    data->set_event(event_list_()[distribution(generator)]);
    event_port_->slot(0)->PublishData();
  }
}

REGISTERPROCESSOR(EventSource)
