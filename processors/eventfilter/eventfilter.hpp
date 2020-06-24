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

#ifndef EVENT_FILTER_HPP
#define	EVENT_FILTER_HPP

#include "eventsync/eventsync.hpp"
#include "utilities/time.hpp"
#include "options/options.hpp"

#include <tuple>
#include <chrono>

class DetectionCriterionValue : public options::Value<SlotType,false> {
public:
    using options::Value<SlotType,false>::Value;
    virtual void from_yaml(const YAML::Node & node) override;
};

class EventFilter : public EventSync {

// CONSTRUCTOR and OVERLOADED METHODS
public:
    EventFilter();
    virtual void CreatePorts() override;
    virtual void Prepare( GlobalContext& context ) override;
    virtual void Preprocess( ProcessingContext& context ) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override;   

// methods
protected:
    // return the alive status of the processor, the flag of found target and
    // the timestamp of the target event detected after a read on the input port
    std::tuple<bool, bool, std::size_t> is_there_target(
        PortIn<EventType>* input_port,
        EventCounter& event_counter,
        std::vector<TimePoint>& arrival_times,
        std::vector<uint64_t>& arrival_timestamps  );
    
    // return the time in milliseconds past from two given time points
    inline double time_between( TimePoint t2, TimePoint t1 ) {
        
        duration = t2 - t1;
        return duration.count();
    }

    // return the time in milliseconds past from a given time point
    inline double time_since( TimePoint t ) {
        
        return time_between( Clock::now(), t );
    }

// DATA PORTS
protected:
    PortIn<EventType>* block_in_port_;

// variables
protected:
    //SlotType detections_to_criterion_;
    
    unsigned int n_blocked_events_;
    EventCounter blocking_events_counter_;
    
    TimePoint gate_close_time_;

    std::chrono::duration<double, std::milli> duration;

// constants
protected:
    const uint64_t NULL_TIMESTAMP = std::numeric_limits<uint64_t>::max();
    
// OPTIONS
protected:

    options::Measurement<double,false> blockout_time_{
        10.,
        "ms",
        options::positive<double>()
    };

    options::Measurement<double,false> block_wait_time_{
        1.5,
        "ms",
        options::positive<double>()
    };
    
    options::Measurement<double,false> sync_time_{
        3.5,
        "ms",
        options::positive<double>()
    };

    options::Bool discard_warnings_{false};

    DetectionCriterionValue detections_to_criterion_{1};

};

#endif	// eventfilter.hpp
