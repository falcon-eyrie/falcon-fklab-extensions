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

#include "logger.hpp"

Logger::Logger() : device::Device(), preamble_(""), level_("debug") {}

void Logger::Configure(device::DeviceProperties & props,
                       const YAML::Node & options) {
    if (options["preamble"]) {
        preamble_ = options["preamble"].as<std::string>();
    }
    
    if (options["level"]) {
        level_ = options["level"].as<std::string>();
    }
}

void Logger::log_message(const std::string & message) {
    if (level_=="debug") { LOG(DEBUG) << preamble_ << message; }
    else if (level_=="info") { LOG(INFO) << preamble_ << message; }
}

bool LoggerMessageOut::message_out_send(const std::string & message) {
    device()->log_message(message);
    return true;
}



DigitalState LoggerDigitalIOAdapter::write_state(const DigitalState & state) {
    auto written = DigitalIOAdapter<Logger>::write_state(state);
    adaptee().log_message("write " + state.to_string() + " ->  " + written.to_string());
    return written;
}

DigitalState LoggerDigitalIOAdapter::toggle_state(const DigitalState & mask) {
    auto written = DigitalIOAdapter<Logger>::toggle_state(mask);
    adaptee().log_message("toggle " + mask.to_string() + " -> " + written.to_string());
    return written;
}

size_t LoggerDigitalOutInterface::digital_out_num_channels() const {
    return device()->num_channels();
}

DigitalState LoggerDigitalOutInterface::digital_out_state() const {
    return device()->read_state();
}

bool LoggerDigitalOutInterface::digital_out_write(const DigitalState & state) {
    device()->write_state(state);
    return true;
}

bool LoggerDigitalOutInterface::digital_out_write_channel(size_t channel, bool state) {
    DigitalState d(device()->num_channels());
    d.set_exclusive_mask(channel);
    d.set();
    LOG(INFO) << d.to_string();
    device()->write_state(d);
    return true;
}

bool LoggerDigitalOutInterface::digital_out_toggle(const DigitalState & mask) {
    device()->toggle_state(mask);
    return true;
}

bool LoggerDigitalOutInterface::digital_out_toggle_channel(size_t channel) {
    DigitalState d(device()->num_channels());
    d.set_exclusive_mask(channel);
    d.set();
    device()->toggle_state(d);
    return true;
}



REGISTERDEVICE(Logger)
REGISTER_DEVICE_INTERFACE(Logger,MessageOut,LoggerMessageOut)
REGISTER_DEVICE_ADAPTER(Logger,LoggerDigitalIOAdapter)
REGISTER_DEVICE_INTERFACE(LoggerDigitalIOAdapter,DigitalOut,LoggerDigitalOutInterface)
