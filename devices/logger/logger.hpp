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

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "device.hpp"

#include "yaml-cpp/yaml.h"
#include "g3log/src/g2log.hpp"

#include <string>

#include "interfaces/message.hpp"
#include "interfaces/dio.hpp"
#include "adapters/dio.hpp"

class Logger : public device::Device {
public:
    Logger();
    
    void Configure(device::DeviceProperties & props, const YAML::Node & options) override ;
    
    void log_message(const std::string & message);

protected:
    std::string preamble_;
    std::string level_;
};

class LoggerMessageOut : public device::DeviceInterface<Logger,MessageOut> {
    bool message_out_send(const std::string & message);
};

class LoggerDigitalIOAdapter : public DigitalIOAdapter<Logger> {
public:
    using DigitalIOAdapter<Logger>::DigitalIOAdapter;
    
    DigitalState write_state(const DigitalState & state) override;
    DigitalState toggle_state(const DigitalState & mask) override;
};

class LoggerDigitalOutInterface : public device::DeviceInterface<LoggerDigitalIOAdapter,DigitalOut> {
public:
    virtual size_t digital_out_num_channels() const;
    
    virtual DigitalState digital_out_state() const;
    
    virtual bool digital_out_write(const DigitalState & state);
    virtual bool digital_out_write_channel(size_t channel, bool state);
    
    virtual bool digital_out_toggle(const DigitalState & mask);
    virtual bool digital_out_toggle_channel(size_t channel);
};

#endif  // LOGGER_HPP
