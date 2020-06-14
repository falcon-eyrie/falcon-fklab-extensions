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

/* EventLogger: takes an EventData stream and logs the arrival of a target event
 * 
 * input ports:
 * events <EventType> (1 slot)
 *
 * output ports:
 * none
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

#ifndef EVENTLOGGER_HPP
#define EVENTLOGGER_HPP

#include "iprocessor.hpp"
#include "eventdata/eventdata.hpp"
#include "utilities/general.hpp"
#include "options/options.hpp"
#include "options/units.hpp"

class EventLogger : public IProcessor
{
// CONSTRUCTOR and OVERLOADED METHODS
public:
    EventLogger();
    virtual void CreatePorts() override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override; 

// DATA PORTS
protected:
    PortIn<EventType>* event_port_;
    EventCounter event_counter_;

// OPTIONS
protected:
    options::Value<EventType::Data,false> target_event_{
        DEFAULT_EVENT, 
        options::notempty<EventType::Data>()};
};

#endif //eventlogger.hpp
