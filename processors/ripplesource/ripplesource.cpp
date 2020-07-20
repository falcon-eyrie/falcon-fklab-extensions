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

#include "ripplesource.hpp"

RippleSource::RippleSource() : IProcessor() {
    add_option("amplitude", amplitude_, "Spike amplitudes");
    add_option("interfall", interfall_, "Minimal time (in ms) between both generated ripples.");
    add_option("distribution", distribution_, "ripple distribution");
    add_option("rate", rate_, "data generation rate")
}

void RippleSource::CreatePorts() {
    event_port_ = create_output_port<MultiChannelType<double>>(
        "data",
        MultiChannelType<double>::Capabilities( ChannelRange(1,256) ),
        MultiChannelType<double>::Parameters(),
        PortOutPolicy( SlotRange(0,256) ) );
}

void EventSource::Process( ProcessingContext& context ) {

    MultiChannelType::Data *data = nullptr;

    auto delay = std::chrono::milliseconds( static_cast<unsigned int>(1000.0/rate_()) );

    while (!context.terminated()) {

        std::this_thread::sleep_for( delay );

        data = event_port_->slot(0)->ClaimData(false);

        // publish data here
        event_port_->slot(0)->PublishData();

    }
}

REGISTERPROCESSOR(RippleSource)
