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

#ifndef SERIALOUTPUT_HPP
#define SERIALOUTPUT_HPP

#include "iprocessor.hpp"
#include "eventdata/eventdata.hpp"
#include "utilities/time.hpp"
#include "options/options.hpp"
#include "options/units.hpp"


#include <fstream>

class SerialOutput : public IProcessor {

// CONSTRUCTOR and OVERLOADED METHODS
public:
    SerialOutput();
    virtual void CreatePorts() override;
    virtual void Preprocess( ProcessingContext& context ) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override;

// METHODS
protected:
    bool to_lock_out( const uint64_t current_timestamp );

// DATA PORTS AND STATES
protected:
    PortIn<EventType>* data_in_port_;
    StaticState<bool>* enabled_;
    StaticState<double>* lockout_period_;
    StaticState<char>* message_;

// OPTIONS
protected:

    options::Bool default_enabled_{true};
    options::Measurement<double,false>  initial_lockout_period_{
        50,
        "ms",
        options::positive<double>(true)
    };

    options::Value<char, false> default_message_{'1'};
    options::Bool save_stim_events_{false};
    options::String port_address_{"/dev/ttyACM0"};
    options::Int baudrate_{9600};
    options::Value<EventType::Data,false> target_event_{
        DEFAULT_EVENT,
        options::notempty<EventType::Data>()};
    options::Bool print_transmission_updates_{true};

// variables
protected:

    int fd_;
    uint64_t nreceived_events_;
    uint64_t ntarget_events_;
    uint64_t nprotocol_executions_;
    uint64_t n_locked_out_events_;
    
    uint64_t previous_TS_nostim_;
    uint64_t delta_TS_;


// CONSTANT
protected:
    const std::string STIM_EVENT_S = "stim_";
    const std::string ENABLED_S = "enabled";
    const std::string LOCKOUT_PERIOD_S = "lockout period";
    const std::string MESSAGE_S = "message";
};

#endif // serialoutput.hpp
