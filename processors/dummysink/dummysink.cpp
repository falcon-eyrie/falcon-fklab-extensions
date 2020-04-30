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

#include <chrono>

#include "utilities/time.hpp"

#include "dummysink.hpp"

void DummySink::CreateStates() {
    tickle_state_ = create_readable_shared_state( "tickle", false, Permission::READ, Permission::WRITE);
    expose_method( "kick", &DummySink::Kick );
}

YAML::Node DummySink::Kick( const YAML::Node & node ) {
    LOG(INFO) << name() << " says: I got kicked!";
    return YAML::Node();
}

bool DummySink::Process_start(ProcessingContext& context) {
    packet_counter = 0;
    retrieve_counter = 0;

    auto address = data_port_->slot(0)->upstream_address();
    LOG(DEBUG) << "slot is connected to " << address.string();

    eos = false;
    tickling = false;
    return true;
}

bool DummySink::Process_loop(ProcessingContext& context) {
        
        for (auto & it : data_in ) {
            if ( it->eos() ) {
                LOG(DEBUG) << name() << " received end of stream signal.";
                eos = false;
                return false;
            }
        }
        
        if (eos) { return false; }

        ++retrieve_counter;
        packet_counter += data_in.size();

        if (tickle_state_->get()!=tickling) {
            tickling=!tickling;
            if (tickling) {
                LOG(INFO) << "Hi hi, that tickles!";
            } else {
                LOG(INFO) << "Why stop tickling?";
            }
        }

        return true;
}

void DummySink::Postprocess( ProcessingContext& context )  {
    std::chrono::milliseconds runtime(std::chrono::duration_cast<std::chrono::milliseconds>(duration) );
    LOG(UPDATE) << name() << ": retrieved " << packet_counter << " packets in " << retrieve_counter << " batches over " << (double)runtime.count()/1000.0 << "seconds (" << (double)packet_counter/retrieve_counter << " packets/batch and " << packet_counter/((double)runtime.count()/1000.0) << " packets/second).";
}

REGISTERPROCESSOR(DummySink)
