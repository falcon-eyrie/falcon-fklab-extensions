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

/* EventSource: generates an EventData stream by randomly emitting
 * events from a list of candidates at a predefined rate
 * 
 * input ports:
 * none
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
 * events <list of string> - list of events to emit
 * rate <double> - (approximate) event rate
 * 
 */
 
#ifndef EVENTSOURCE_HPP
#define EVENTSOURCE_HPP

#include "iprocessor.hpp"
#include "eventdata/eventdata.hpp"
#include "options/options.hpp"

class EventSource : public IProcessor {
    
public:

    EventSource () : IProcessor() {
        add_option("events", event_list_);
        add_option("rate", event_rate_);
    }

    virtual void Configure( const YAML::Node& node, const GlobalContext& context) override;
    virtual void CreatePorts() override;
    virtual void Process( ProcessingContext& context ) override;

public:
    const double DEFAULT_EVENT_RATE = 1.0;
    const std::string DEFAULT_EVENT = "default_eventsource_event";

protected:
    PortOut<EventType>* event_port_;

    options::Vector<std::string> event_list_{
        {DEFAULT_EVENT},
        options::notempty<std::vector<std::string>>() +
        options::each<std::string>(options::notempty<std::string>())};
    
    options::Double event_rate_{DEFAULT_EVENT_RATE, options::positive<double>()};
    
};

#endif // eventsource.hpp
