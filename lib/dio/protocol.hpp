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

#ifndef DIO_PROTOCOL_H
#define DIO_PROTOCOL_H

#include "dio.hpp"
#include "interfaces/dio.hpp"
#include <vector>

class DigitalOutputProtocol {
    
public:
    DigitalOutputProtocol( size_t nchannels, unsigned int pulse_width, DigitalOutputMode default_mode = DigitalOutputMode::NONE );
    
    DigitalOutputMode mode( size_t channel ) const;
    
    void set_mode( size_t channel, DigitalOutputMode mode = DigitalOutputMode::NONE);
    void set_mode( std::vector<size_t> channels, DigitalOutputMode mode = DigitalOutputMode::NONE);
    
    unsigned int pulse_width() const;
    void set_pulse_width( unsigned int value );
    
    void execute( DigitalOut* iface );
    
protected:
    
    size_t nchannels_;
    unsigned int pulse_width_;
    DigitalState pulse_;
    DigitalState set_;
    DigitalState toggle_;
};

#endif  // DIO_PROTOCOL_H
