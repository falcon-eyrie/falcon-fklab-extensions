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

EventDelayed::EventDelayed(): delayed_range_(150, 200) {
  add_option("enabled", default_enabled_,
             "Enable the processing of incoming events.");

  // Flexible processing event feature
  add_option(LOCKOUT_PERIOD_S, initial_lockout_period_,
             "Lock out time after the processing of an event.");

  add_option("delayed event", initial_delayed_event_,
             "Enable the delay of the event for a time randomly chosen between the delay range");
  add_option("delay range", initial_delayed_range_,
             "if delayed event is true, the delayed time will be pseudo-randomly "
             "chosen in this range.");

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

  delayed_range_= Range<long int>(initial_delayed_range_());

  if (!initial_delayed_event_()) {
    LOG(INFO) << name() << ". Event are sent on-time.";
  } else {
    LOG(INFO) << name() << ". Events are delayed of a range between"
              << delayed_range_.lower() << " and " << delayed_range_.upper()<< " ms.";
  }


}

void EventDelayed::CreatePorts() {
  data_in_port_ =
      create_input_port<EventType>(EventType::Capabilities(),
                                   PortInPolicy(SlotRange(1), false, 1));

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

void EventDelayed::Preprocess(ProcessingContext &context) {

  ontime_received_event_ = 0;
  delayed_received_event_ = 0;
  event_lockout_ = 0;

  //initialize enough if the past to be sure the first stimulation won't be lockout
  previous_TS_nostim_ =
      Clock::now() -
      std::chrono::milliseconds((long int)lockout_period_->get() + 10);
}

void EventDelayed::Process(ProcessingContext &context) {
  EventType::Data *data_in = nullptr;
  EventType::Data *data_out = nullptr;

  std::string message;
  std::string path = context.resolve_path("run://", "run");
  std::string filepath = path + name();

  std::random_device rd;
  std::mt19937 generator_(rd()); // Standard mersenne_twister_engine (Higher
                                 // complexity / randomness)
  std::uniform_int_distribution<> distrib(delayed_range_.lower(),
                                          delayed_range_.upper());

  while (!context.terminated()) {

    while (!event_queue_.empty() and event_queue_.top().ts < Clock::now()) {
      auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                        Clock ::now() - event_queue_.top().ts)
                        .count();

      LOG(DEBUG) << name() << "Time to sent a delayed event ("
                 << event_queue_.top().data_in->event() << ") with " << millis
                 << "ms late.";

      send_event(event_queue_.top().data_in, data_out, filepath);
      event_queue_.pop();
    }

    if (!data_in_port_->slot(0)->RetrieveData(data_in)) {
      break;
    }

    auto nread = data_in_port_->slot(0)->status_read();

    if (nread == 0) {
      data_in_port_->slot(0)->ReleaseData();
      continue;
    }

    if (enabled_->get()) {
      int wait_time = distrib(generator_);
      if (delayed_event_->get()) {
        ++delayed_received_event_;
        LOG(DEBUG) << name() << "Save an event (" << data_in->event()
                   << ") to send later with " << wait_time << "ms delayed.";
        auto delay =
            data_in->source_timestamp() + std::chrono::milliseconds(wait_time);
        Delayed event(delay, data_in);
        event_queue_.push(event);
      } else {
        ++ontime_received_event_;
        send_event(data_in, data_out, filepath);
      }

      data_in_port_->slot(0)->ReleaseData();
    }
  }
}

void EventDelayed::send_event(EventType::Data *data_in,
                              EventType::Data *data_out, std::string filepath) {
  if (not to_lock_out()) {
    LOG(DEBUG) << name() << "Sent one event: " << data_in->event();
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
    LOG(DEBUG) << name() << data_in->event() << " has been locked-out";
    ++event_lockout_;
  }
}
void EventDelayed::Postprocess(ProcessingContext &context) {

  auto msg = "Successfully executed conversion protocol: " +
             std::to_string(ontime_received_event_) + "ontime and " +
             std::to_string(delayed_received_event_) + " delayed with " +
             std::to_string(event_lockout_) + " events locked out.";

  ontime_received_event_ = 0;
  delayed_received_event_ = 0;
  event_lockout_ = 0;
}

bool EventDelayed::to_lock_out() {
  auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                    Clock ::now() - previous_TS_nostim_)
                    .count();
  if (millis <= lockout_period_->get()) {

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