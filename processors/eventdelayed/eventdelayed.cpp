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

#include "eventdelayed.hpp"


EventDelayed::EventDelayed() {

  // Flexible processing event feature
  add_option(LOCKOUT_PERIOD_S, initial_lockout_period_,
             "Lock out time after the processing of an event.");

  add_option("delayed event", initial_delayed_event_,
             "Enable the delay of the event for 1 lockout time period");
  add_option("target event", target_event_, "Event to be processed");

  // saving feature
  add_option("enable saving", save_events_,
             "Enable saving of target events to disk.");
  add_option(
      "filename prefix", prefix_,
      "if enable saving is true, the saving file is name 'prefix + event'");
}

void EventDelayed::Configure(const YAML::Node &node,
                               const GlobalContext &context) {

  if (initial_lockout_period_() <= 0) {
    LOG(INFO) << name() << ". No lockout period set.";
  } else {
    LOG(INFO) << name() << ". Max output frequency set to "
              << 1e3 / static_cast<double>(initial_lockout_period_()) << " Hz.";
  }

  if ((initial_delayed_event_() and initial_lockout_period_() <= 0) or !initial_delayed_event_()){
    LOG(INFO) << name() << ". Event are sent on-time.";
  } else {
    LOG(INFO) << name() << ". Events are delayed of "
              << static_cast<double>(initial_lockout_period_()) << " ms.";
  }
}

void EventDelayed::CreatePorts() {
  data_in_port_ = create_input_port<EventType>(EventType::Capabilities(),
                                               PortInPolicy(SlotRange(1)));

  data_out_port_ = create_output_port<EventType>(
      EventType::Capabilities(), EventType::Parameters(DEFAULT_EVENT),
      PortOutPolicy(SlotRange(1)));

  enabled_ = create_static_state(ENABLED_S, default_enabled_(), true,
                                 Permission::WRITE);

  lockout_period_ = create_static_state(
      LOCKOUT_PERIOD_S, initial_lockout_period_(), true, Permission::WRITE);

  delayed_event_ = create_static_state(
      "delayed event", initial_delayed_event_(), true, Permission::WRITE);
}

// TODO: delay uniform between a min/max bound -> option boundaries

void EventDelayed::Preprocess(ProcessingContext &context) {

  // reset counters and logs
  event_counter_.reset();
  execution_ = 0;
  delayed_execution_ = 0;
  previous_TS_nostim_ = Clock::now();
}

void EventDelayed::Process(ProcessingContext &context) {
  EventType::Data *data_in = nullptr;
  EventType::Data *data_out = nullptr;

  std::string message;
  std::string path = context.resolve_path("run://", "run");
  std::string filepath = path + name();

  while (!context.terminated()) {

    if (!data_in_port_->slot(0)->RetrieveData(data_in)) {
      break;
    }
    ++event_counter_.all_received;

    if (enabled_->get() && target_event_() == *data_in) {
      ++event_counter_.target;

      int wait_time = 0;
      if (delayed_event_->get()) {
        ++delayed_execution_;
        event_queue_.push(data_in->source_timestamp() +
                          std::chrono::milliseconds(wait_time));
      } else {
        send_event(data_in, data_out, filepath);
      }
    } else {
      ++event_counter_.non_target;
    }

    while (event_queue_.top() > Clock::now()) {
      ++delayed_execution_;
      send_event(data_in, data_out, filepath);
      event_queue_.pop();
    }

    data_in_port_->slot(0)->ReleaseData();
  }
}

void EventDelayed::send_event(EventType::Data *data_in,EventType::Data *data_out,
                              std::string filepath){
  if (not to_lock_out()) {
    ++execution_;
    data_out = data_out_port_->slot(0)->ClaimData(true);
    data_out->set_hardware_timestamp(data_in->hardware_timestamp());

    data_out->set_event(data_in->event());
    data_out->set_source_timestamp();
    data_out_port_->slot(0)->PublishData();
    if (save_events_()) { // save stim events to disk
      write_data_logfile(filepath, prefix_() + data_in->event(),
                         data_in->serial_number());
    }
  } else {
    ++event_counter_.lockout_target;
  }
}
void EventDelayed::Postprocess(ProcessingContext &context) {

  auto msg =  ". Received " + std::to_string(event_counter_.all_received)
            + " events, of which " + std::to_string(event_counter_.target)
            + " were targets. Successfully executed conversion protocol (ontime) "
            + std::to_string(execution_ - delayed_execution_)  + "- (delayed)"
            + std::to_string(delayed_execution_) + " times out of "
            + std::to_string(event_counter_.target)
            + ". " + std::to_string(event_counter_.lockout_target) + " events were locked out.";

  if (event_counter_.consistent_counters()
      and event_counter_.target == execution_ ) {
    LOG(INFO) << name() << msg << " Counters are consistent.";
  } else {
    LOG(WARNING) << name() << " Counters are inconsistent.";
  }

  event_counter_.reset();
  execution_ = 0;
  delayed_execution_ = 0;
}

bool EventDelayed::to_lock_out() {
  if (previous_TS_nostim_.time_since_epoch() <= std::chrono::milliseconds(lockout_period_->get())) {
    return true;
  }

  previous_TS_nostim_ = Clock::now();
  return false;
}

void EventDelayed::write_data_logfile(std::string file_path,
                                        std::string filename, uint64_t data) {
  if (save_events_()) { // save stim events to disk
    // filename will also be the key to the container of files
    // check if this type of event has been saved before
    if (streams_.count(filename) == 0) {
      create_file(file_path, filename);
    }

    streams_[filename]->write(reinterpret_cast<const char *>(&data),
                              sizeof(decltype(data)));
  }
}

REGISTERPROCESSOR(EventDelayed)