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

#ifndef INTERFACES_DIO_HPP
#define INTERFACES_DIO_HPP

#include "dio/dio.hpp"
#include "utilities/time.hpp"

class DigitalOut { 
public:
    
    // required interface methods
    virtual size_t digital_out_num_channels() const = 0;
    
    virtual DigitalState digital_out_state() const = 0;
    
    virtual bool digital_out_write(const DigitalState & state) = 0;
    virtual bool digital_out_write_channel(size_t channel, bool state) = 0;
    
    virtual bool digital_out_toggle(const DigitalState & mask) = 0;
    virtual bool digital_out_toggle_channel(size_t channel) = 0;
    
    // optional interface methods
    virtual bool digital_out_pulse(const DigitalState & mask, unsigned int width) {
        // default implementation
        // toggle selected bits
        digital_out_toggle(mask);
        // and after a delay
        custom_sleep_for(width);
        // toggle selected bits again
        digital_out_toggle(mask);
        
        return true;
    }
    
    virtual bool digital_out_pulse_channel(size_t channel, unsigned int width ) {
        digital_out_toggle_channel(channel);
        custom_sleep_for(width);
        digital_out_toggle_channel(channel);
        
        return true;
    }
};

#endif  // INTERFACES_DIO_HPP
