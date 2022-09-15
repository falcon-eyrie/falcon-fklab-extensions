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

EventDelayed::EventDelayed() : delayed_range_(150, 200) {
    add_option(DISABLED_S, default_disabled_,
               "Enable the processing of incoming events.");

    add_option(DELAYED_S, initial_delayed_event_,
               "Enable the delay of the event for a time randomly chosen between the delay range");
    add_option("delayed range", initial_delayed_range_,
               "if delayed event is true, the delayed time will be pseudo-randomly "
               "chosen in this range.");
    // message feature
    add_option("message/detection", msg_detection_,
               "Message to send on the detection channel");
    add_option("message/delayed", msg_delayed_,
               "Message to send on the stimulation channel");
    add_option("message/ontime", msg_ontime_,
               "Message to send on ontime mode.");

    // Lock-out time 

    add_option(STOP_ANALYSIS_TIME_S+"/starting time", when_stop_analysis_period_,
               "when to start stopping detection after a stimulation.");

    add_option(STOP_ANALYSIS_TIME_S+"/period", initial_stop_analysis_period_,
               "Lock out time for detecting pattern after a stimulation");

    add_option(STOP_DETECTION_TIME_S+"/period", initial_stop_detection_period_,
               "Lock out time for sending new detection/stimulation after a stimulation.");

    add_option(STOP_DETECTION_TIME_S+"/detection", start_after_detection_,
               "Start stopping for detecting pattern after a detection");

    add_option(STOP_DETECTION_TIME_S+"/stimulation", start_after_stimulation_,
               "Start stopping for detecting pattern after a stimulation");

    // saving feature
    add_option("enable saving", save_events_,
               "Enable saving of target events to disk.");

    add_option("filename prefix", prefix_,
            "if enable saving is true, the saving file is name 'prefix + event'");
}

void EventDelayed::Configure(const GlobalContext &context) {

    if (initial_stop_detection_period_() == 0) {
        LOG(INFO) << name() << ". No lockout period set.";
    } else {
        LOG(INFO) << name() << ". Max output frequency set to "
                  << 1e3 / static_cast<double>(initial_stop_detection_period_()) << " Hz.";
    }

    if (initial_stop_analysis_period_() == 0) {
        LOG(INFO) << name() << ". No analysis time off set after stimulation.";
    } else {
        LOG(INFO) << name() << ". Analysis time off after stimulation set to "
                  << static_cast<double>(initial_stop_analysis_period_()) << " s.";
    }

    delayed_range_ = Range<long int>(initial_delayed_range_());

    if (!initial_delayed_event_()) {
        LOG(INFO) << name() << ". Event are sent on-time.";
    } else {
        LOG(INFO) << name() << ". Events are delayed of a range between"
                  << delayed_range_.lower() << " and " << delayed_range_.upper() << " ms.";
    }

    if (delayed_range_.upper() - delayed_range_.lower() < initial_stop_detection_period_()) {

        LOG(INFO) << name()
                  << ". When the difference between maximal and minimal possible delay is below the lockout period,"
                  << "some weird behavior of detection locked out or not locked out when it should, could occur.";
    }

    LOG_IF(WARNING, start_after_detection_() != start_after_stimulation_()) << name()
                                                                         << "Event trigger lockout time:\n Be careful if you switch on ontime mode "
                                                                         << "where stimulation and detection are in the same time, "
                                                                         << "your options set will be interpreted as do a stimulation lock-out time. \n"
                                                                         << "Turn off both detection and stimulation if you want to deactivate. ";



}

void EventDelayed::CreatePorts() {
    data_in_port_ =
            create_input_port<EventType>(EventType::Capabilities(),
                                         PortInPolicy(SlotRange(1), false, 1));

    output_port_ = create_output_port<EventType>(EventType::Capabilities(),
                                                        EventType::Parameters(DEFAULT_EVENT),
                                                        PortOutPolicy(SlotRange(1)));
    // -----  Mode state --- //
    disabled_ = create_follower_state(DISABLED_S, default_disabled_(),
                                      Permission::WRITE);

    delayed_event_ = create_follower_state(
            DELAYED_S, initial_delayed_event_(), Permission::WRITE);

    // ----- Lock-out state --- //
    stop_detection_period_ = create_static_state(
            STOP_DETECTION_TIME_S, initial_stop_detection_period_(), true, Permission::WRITE);

    stop_analysis_period_ = create_static_state(
            STOP_ANALYSIS_TIME_S, initial_stop_analysis_period_(), true, Permission::WRITE);

    analysis_unlocked_ = create_broadcaster_state(
            "analysis enabled", true, Permission::READ);      // connected to the ripple detection processor to disable/enabled ripple processing
}

void EventDelayed::Preprocess(ProcessingContext &context) {

    ontime_received_event_ = 0;
    delayed_received_event_ = 0;
    event_lockout_ = 0;

    //initialize enough if the past to be sure the first stimulation won't be lockout
    previous_TS_nostim_ =
            Clock::now() -
            std::chrono::milliseconds((long int) stop_detection_period_->get() + 10);

    std::string path = context.resolve_path("run://", "run");
    std::string filepath = path + name();

    create_file(filepath, prefix_() + msg_delayed_());
    create_file(filepath, prefix_() + msg_detection_());
    create_file(filepath, prefix_() + msg_ontime_());
}

void EventDelayed::Process(ProcessingContext &context) {
    EventType::Data *data_in = nullptr;
    std::random_device rd;
    std::mt19937 generator_(rd()); // Standard mersenne_twister_engine (Higher
    // complexity / randomness)
    std::uniform_int_distribution<> distrib(delayed_range_.lower(),
                                            delayed_range_.upper());

    while (!context.terminated()) {


        // This part is about sending out delayed events
        while (!delayed_event_queue_.empty() and delayed_event_queue_.top().ts < Clock::now()) {
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                    Clock::now() - delayed_event_queue_.top().ts)
                    .count();

            LOG(DEBUG) << name() << ". Time to sent a delayed event ("
                       << delayed_event_queue_.top().data_in->event() << ") with " << millis
                       << "ms late.";

            send_event(delayed_event_queue_.top().data_in,  msg_delayed_());
            delayed_event_queue_.pop();
        }

        // This part is about triggering delayed lock-out
        while(!lockout_queue_.empty() and lockout_queue_.top().ts < Clock::now()){


            LOG(DEBUG) << name() << ". Start a lockout after stimulation for " + std::to_string((long int)stop_analysis_period_->get() ) + " ms.";
            // stop detecting in the ripple detector
            analysis_unlocked_->set(false);

            // buzy sleep has no detections should be received and not stimulation should be sent during this time.
            std::this_thread::sleep_for(std::chrono::milliseconds((long int)stop_analysis_period_->get() ));
            analysis_unlocked_->set(true);
            lockout_queue_.pop();

            while (!delayed_event_queue_.empty() and delayed_event_queue_.top().ts < Clock::now()) { //Remove any stimulations which would have happened during the detection/stimulation lockout
                LOG(DEBUG) << name() << "The stimulation of this " << delayed_event_queue_.top().data_in->event() << " has been locked-out due to the detection lockout after stimulation.";
                delayed_event_queue_.pop();
            }
        }

        if (!data_in_port_->slot(0)->RetrieveData(data_in)) {
            break;
        }

        auto nread = data_in_port_->slot(0)->status_read();

        if (nread == 0) {
            data_in_port_->slot(0)->ReleaseData();
            continue;
        }
        // If not stimulation disabled
        if (!disabled_->get()) {
            int wait_time = distrib(generator_);
            // If stimulation is delayed
            if (delayed_event_->get()) {
                ++delayed_received_event_;
                LOG(INFO) << name() << ". Save an event (" << data_in->event() << ") to send later with " << wait_time
                          << "ms delayed.";
                auto delay =
                        data_in->source_timestamp() + std::chrono::milliseconds(wait_time);

                if ((not start_after_stimulation_() or not to_lock_out_in_future(delay)) and ( not start_after_detection_() or not to_lock_out())) {
                    Delayed event(delay, data_in);
                    delayed_event_queue_.push(event);
                    send_event(data_in, msg_detection_());
                    for(auto time_to_start:  when_stop_analysis_period_()){
                        Delayed event_lockout(delay+ std::chrono::milliseconds(time_to_start), data_in);
                        lockout_queue_.push(event_lockout);
                    }

                } else {
                    LOG(DEBUG) << name() << data_in->event() << " has been locked-out in delayed mode";
                    ++event_lockout_;
                }
            // If stimulation is sent at the same time as detection = ontime mode
            } else {
                ++ontime_received_event_;
                if (not(start_after_detection_() or start_after_stimulation_()) or not to_lock_out()) {
                    send_event(data_in, msg_ontime_());
                    for(auto time_to_start:  when_stop_analysis_period_()){
                        Delayed event_lockout(data_in->source_timestamp() + std::chrono::milliseconds(time_to_start), data_in);
                        lockout_queue_.push(event_lockout);
                    }
                } else {
                    LOG(DEBUG) << name() << data_in->event() << " has been locked-out in ontime mode";
                    ++event_lockout_;
                }
            }
        // Stimulation event is disable - detection is still sent
        } else {
            ++ontime_received_event_;
            if (not start_after_detection_() or not to_lock_out()) {
                send_event(data_in, msg_detection_());
            } else {
                LOG(DEBUG) << name() << data_in->event() << " has been locked-out in disable mode";
                ++event_lockout_;
            }
        }
        data_in_port_->slot(0)->ReleaseData();

    }
}

void EventDelayed::send_event(EventType::Data *data_in, std::string type) {

    LOG(INFO) << name() << ". Sent one event: " << type;
    EventType::Data *data_out = output_port_->slot(0)->ClaimData(true);
    data_out->set_hardware_timestamp(data_in->hardware_timestamp());

    data_out->set_event(type);
    data_out->set_source_timestamp();
    output_port_->slot(0)->PublishData();

    if (save_events_()) { // save stim events to disk
        uint64_t serial_number = data_in->serial_number();
        LOG(DEBUG) << prefix_()<< type;
        streams_[prefix_() + type]->write(reinterpret_cast<const char *>(&serial_number),
                                    sizeof(decltype(serial_number)));
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

bool EventDelayed::to_lock_out_in_future(TimePoint start_event) {
    TimePoint last_future_event;
    if (delayed_event_queue_.empty())
        last_future_event = previous_TS_nostim_;
    else
        last_future_event = delayed_event_queue_.top().ts;

    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(start_event - last_future_event).count();

    if (millis <= stop_detection_period_->get()){
        LOG(DEBUG) << name() << ". Start a stimulation lockout after stimulation for " + std::to_string(stop_detection_period_->get()) + " secs.";
        return true;
    }

    previous_TS_nostim_ = last_future_event;
    return false;
}


bool EventDelayed::to_lock_out() {

    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - previous_TS_nostim_).count();

    if (millis <= stop_detection_period_->get()){
        LOG(DEBUG) << name() << ". Start a stimulation lockout after detection for " + std::to_string(stop_detection_period_->get()) + " secs.";
        return true;
    }

    previous_TS_nostim_ = Clock::now();
    return false;
}


REGISTERPROCESSOR(EventDelayed)
