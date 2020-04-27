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


#ifndef BLACKHOLE_DEVICE_H
#define BLACKHOLE_DEVICE_H

#include "device.hpp"
#include "dio/dio.hpp"
#include "interfaces/dio.hpp"

#include "yaml-cpp/yaml.h"
#include "g3log/src/g2log.hpp"

class BlackHole : public device::Device {
    void Configure(device::DeviceProperties & config, const YAML::Node & options) override {
        config.set_resource("test", false);
    }
};

class BlackHoleDigitalOutInterface : public device::DeviceInterface<BlackHole,DigitalOut> {
public:
    
    // required interface methods
    virtual size_t digital_out_num_channels() const { return 8; }
    
    virtual DigitalState digital_out_state() const { return DigitalState(8); }
    
    virtual bool digital_out_write(const DigitalState & state) { return true; }
    virtual bool digital_out_write_channel(size_t channel, bool state) { return true; }
    
    virtual bool digital_out_toggle(const DigitalState & mask) { return true; }
    virtual bool digital_out_toggle_channel(size_t channel) { return true; }
};



#endif // BLACKHOLE_DEVICE_H
