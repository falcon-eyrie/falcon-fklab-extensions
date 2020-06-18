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

/* EventSync: synchronizes on the occurrence of a target event on all
 * its input slots, before emitting the same target event
 * 
 * input ports:
 * events <EventType> (1-256 slots)
 *
 * output ports:
 * events <EventType> (1 slot)
 *
 * exposed states:
 * none
 *
 * exposed methods:
 * none
 *
 * options:
 * target_event <string> - target event
 * 
 */

#ifndef EVENTSYNC_HPP
#define EVENTSYNC_HPP

#include "iprocessor.hpp"
#include "eventdata/eventdata.hpp"
#include "utilities/general.hpp"
#include "utilities/time.hpp"

class EventSync : public IProcessor {

// CONSTRUCTOR and OVERLOADED METHODS
public:
    EventSync();
    virtual void CreatePorts() override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override;

// methods
protected:
    void reset_timestamps(TimestampRegister timestamp_reg);
    void update_latest_ts(EventType::Data* data_in);
    void log_and_reset_counters( PortIn<EventType>* in_port, EventCounter& counter );

// DATA PORTS
protected:
    PortIn<EventType>* data_in_port_;
    PortOut<EventType>* data_out_port_;

// variables
protected:
    EventCounter event_counter_;
    uint64_t n_events_synced_;
    
    TimestampRegister timestamps_;

// OPTIONS
protected:
    options::Value<EventType::Data,false> target_event_{
        DEFAULT_EVENT, 
        options::notempty<EventType::Data>()};
};

#endif // eventsync.hpp
