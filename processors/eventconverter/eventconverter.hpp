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

/*
 * EventConverter: convert incoming EventData into another event
 * 
 * input ports:
 * events <EventData> (1 slot1)
 * 
 * output ports:
 * events <IData> (1 slot)
 *
 * 
 * options:
 * event_name <string> - name for generating the delayed event
 * replace <bool> - if True output events will be replaced by "event_name" events,
 * if False output events will have "event_name" appended to their original name
 */

#ifndef EVENT_CONVERTER_HPP
#define	EVENT_CONVERTER_HPP

#include "eventdata/eventdata.hpp"
#include "iprocessor.hpp"

class EventConverter : public IProcessor {

// CONSTRUCTOR and OVERLOADED METHODS

public:
    EventConverter();
    virtual void CreatePorts() override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override;

// DATA PORTS
protected:
    PortIn<EventType>* data_in_port_;
    PortOut<EventType>* data_out_port_;


// OPTIONS
protected:
    options::String event_name_{
        "stimulation",
        options::notempty<std::string>()};

    options::Bool replace_{true};

};

#endif	// eventconverter.hpp

