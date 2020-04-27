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

#include "protocol.hpp"

DigitalOutputProtocol::DigitalOutputProtocol( size_t nchannels, unsigned int pulse_width, DigitalOutputMode default_mode )
    : nchannels_(nchannels), pulse_width_(pulse_width),
      pulse_(nchannels,0), set_(nchannels,0), toggle_(nchannels,0) {
    
    // disable all channels
    pulse_.set_mask_set();
    set_.set_mask_set();
    toggle_.set_mask_set();
    
    if (default_mode == DigitalOutputMode::HIGH) {
        set_.reset_mask();
        set_.set(default_mode == DigitalOutputMode::HIGH);
    } else if (default_mode == DigitalOutputMode::PULSE) {
        pulse_.reset_mask();
        pulse_.set();
    } else if (default_mode == DigitalOutputMode::TOGGLE) {
        toggle_.reset_mask();
        toggle_.set();
    }
}

DigitalOutputMode DigitalOutputProtocol::mode( size_t channel ) const {
    
    if (set_.mask()[channel]) {
        if (set_[channel]) { return DigitalOutputMode::HIGH; }
        else { return DigitalOutputMode::LOW; }
    } else if (pulse_.mask()[channel]) {
        return DigitalOutputMode::PULSE;
    } else if (toggle_.mask()[channel]) {
        return DigitalOutputMode::TOGGLE;
    } else {
        return DigitalOutputMode::NONE;
    }
}

void DigitalOutputProtocol::set_mode( size_t channel, DigitalOutputMode mode ) {
    
    // disable channel
    set_.set_mask(channel, false);
    pulse_.set_mask(channel, false);
    toggle_.set_mask(channel, false);
    
    if (mode == DigitalOutputMode::HIGH || mode == DigitalOutputMode::LOW) {
        set_.set_mask(channel, true);
        set_.set(channel, mode == DigitalOutputMode::HIGH);
    } else if (mode == DigitalOutputMode::PULSE) {
        pulse_.set_mask(channel, true);
        pulse_.set(channel);
    } else if (mode == DigitalOutputMode::TOGGLE) {
        toggle_.set_mask(channel, true);
        toggle_.set(channel);
    }
}

unsigned int DigitalOutputProtocol::pulse_width() const {
    
    return pulse_width_;
}

void DigitalOutputProtocol::set_pulse_width( unsigned int value ) {
    
    pulse_width_ = value;
}

void DigitalOutputProtocol::set_mode( std::vector<size_t> channels, DigitalOutputMode mode ) {
    
    for (const size_t & c : channels) {
        set_mode(c, mode);
    }
}

void DigitalOutputProtocol::execute( DigitalOut * iface ) {
    
    if (set_.mask_any()) { iface->digital_out_write(set_); }
    if (toggle_.mask_any()) { iface->digital_out_toggle(toggle_); }
    if (pulse_.mask_any()) { iface->digital_out_pulse(pulse_, pulse_width_); }
}

