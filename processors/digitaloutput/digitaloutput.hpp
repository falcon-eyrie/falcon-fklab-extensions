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


#ifndef DIGITALOUTPUT_HPP
#define DIGITALOUTPUT_HPP

#include "iprocessor.hpp"
#include "eventdata/eventdata.hpp"
#include "dio/dio.hpp"
#include "utilities/time.hpp"

typedef std::map<std::string,std::map<std::string,std::vector<uint32_t>>> ProtocolYAMLMap;
typedef std::map<std::string,std::unique_ptr<DigitalOutputProtocol>> ProtocolMap;

class DigitalOutput : public IProcessor {
    
public:
    virtual void Configure(const YAML::Node& node, const GlobalContext& context) override;
    virtual void CreatePorts() override;
    virtual void Preprocess( ProcessingContext& context ) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override;
    

protected:
    bool to_lock_out( const uint64_t current_timestamp );
    
protected:
    PortIn<EventType>* data_in_port_;
    
    bool default_enabled_;
    StaticState<decltype(default_enabled_)>* enabled_state_;
    
    int default_lockout_period_;
    StaticState<decltype(default_lockout_period_)>* lockout_period_;
    
    bool save_stim_events_;
    std::wstring device_name_;
    
    std::unique_ptr<DigitalDevice> device_;
    ProtocolMap protocols_;
    
    bool print_protocol_execution_updates_;
    
    std::uint64_t nreceived_events_;
    decltype(nreceived_events_) ntarget_events_;
    decltype(nreceived_events_) nprotocol_executions_;
    decltype(nreceived_events_) n_locked_out_events_;
    
    uint64_t previous_TS_nostim_;
    decltype(default_lockout_period_) delta_TS_;
    
public:
    const decltype(default_enabled_) DEFAULT_ENABLED = true;
    const decltype(default_lockout_period_) DEFAULT_LOCKOUT_PERIOD = 300;
    const decltype(save_stim_events_) DEFAULT_SAVE_STIM_EVENTS = true;
    const unsigned int DEFAULT_PULSE_WIDTH = 400;
    const unsigned int DEFAULT_DUMMY_NCHANNELS = 16;

    const std::string ENABLED = "enabled";
    const std::string LOCKOUT_PERIOD = "lockout period";

protected:
    const std::string STIM_EVENT = "stim_";
};

#endif // digitaloutput.hpp
