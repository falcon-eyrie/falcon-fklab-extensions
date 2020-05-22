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

#include "multichannelfilter.hpp"

#include <thread>
#include <chrono>
#include <exception>

void MultiChannelFilter::CreatePorts( ) {
    
    data_in_port_ = create_input_port<MultiChannelData<double>>(
        "data",
        MultiChannelData<double>::Capabilities( ChannelRange(1,256) ),
        PortInPolicy( SlotRange(0,256) ) );
    
    data_out_port_ = create_output_port<MultiChannelData<double>>(
        "data",
        MultiChannelData<double>::Capabilities( ChannelRange(1,256) ),
        MultiChannelData<double>::Parameters(),
        PortOutPolicy( SlotRange(0,256) ) );
}

void MultiChannelFilter::Preprocess( ProcessingContext& context ) {}

void MultiChannelFilter::Process( ProcessingContext& context ) {
    
    
    MultiChannelData<double>* data_in = nullptr;
    MultiChannelData<double>* data_out = nullptr;
    
    auto nslots = data_in_port_->number_of_slots();
    decltype(nslots) k=0;
    
    while (!context.terminated()) {
        
        // go through all slots
        for (k=0; k<nslots; ++k) {
            
            // retrieve new data
            if (!data_in_port_->slot(k)->RetrieveData( data_in )) {break;}
            
            // claim output data buckets
            data_out = data_out_port_->slot(k)->ClaimData(false);
            
            // filter incoming data               
            filters_[k]->process_by_channel(
                data_in->nsamples(), data_in->data(), data_out->data() );
            
            data_out->set_sample_timestamps( data_in->sample_timestamps() );
            
            data_out->CloneTimestamps( *data_in );
            
            // publish and release data
            data_out_port_->slot(k)->PublishData();
            data_in_port_->slot(k)->ReleaseData();
            
        }
                
    }
}

void MultiChannelFilter::Postprocess( ProcessingContext& context ) {}

REGISTERPROCESSOR( MultiChannelFilter )
