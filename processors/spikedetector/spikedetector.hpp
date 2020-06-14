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

/* SpikeDetector: detects spikes on any of the incoming MultiChannelData stream;
 * sends SpikeData on the output port spikes and an event "spike"/"spikes"
 *  on the output port events if one or more spikes have been detected in the received buffer
 * 
 * input ports:
 * data <MultiChannelType> (1 slot)
 *
 * output ports:
 * spikes <SpikeType> (1 slot)
 * events <EventType> (1 slot)
 *
 * exposed states:
 * threshold <double> - threshold that a single channel must cross
 * peak_lifetime <unsigned int> - number of samples that will be used to look for a peak;
 *
 *
 * exposed methods:
 * none
 *
 * options:
 * buffer_size <double> - amount of data that will be used to look for spikes [ms]
 * strict_time_bin_check <bool> - whether the buffer size will be strictly or loosely checked
 * for compatibility with the upstream processor
 * threshold <double> - initial threshold state
 * invert_signal <bool> - whether the signal does (true) or does not (false) need
 * to be inverted when detecting spikes
 * peak_lifetime <unsigned int> - initial peak_lifetime value
 * 
 */

#ifndef SPIKEDETECTOR_H
#define SPIKEDETECTOR_H

#include "iprocessor.hpp"
#include "dsp/algorithms.hpp"
#include "spikedata/spikedata.hpp"
#include "eventdata/eventdata.hpp"
#include "multichanneldata/multichanneldata.hpp"
#include "options/options.hpp"

class SpikeDetector : public IProcessor {

// CONSTRUCTOR and OVERLOADED METHODS
public:
    SpikeDetector();
    virtual void CreatePorts( ) override;
    virtual void CompleteStreamInfo() override;
    virtual void Prepare( GlobalContext& context ) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override; 

// PORTS
protected:
    PortIn<MultiChannelType<double>>* data_in_port_;
    PortOut<SpikeType>* data_out_port_spikes_;
    PortOut<EventType>* data_out_port_events_;

// STATES
protected:
    StaticState<double>* threshold_;
    StaticState<unsigned int>* peak_lifetime_;

// variables
protected:
    unsigned int n_channels_;

    size_t n_incoming_;
    size_t incoming_buffer_size_samples_;
    
    std::unique_ptr<dsp::algorithms::SpikeDetector> spike_detector_;
    
    MultiChannelType<double>::Data* data_in_;
    std::unique_ptr<MultiChannelType<double>::Data> inverted_signals_;
    SpikeType::Data* spike_data_out_;
    EventType::Data* event_data_out_;
    
    uint64_t n_streamed_events_;

// constants  
public:
    const decltype(n_channels_) MAX_N_CHANNELS = 8;

    const std::string PEAK_LIFETIME_S = "peak_lifetime";
    const std::string THRESHOLD_S = "threshold";
    
protected:
    const int RINGBUFFER_SIZE = 1e5;

// OPTIONS
protected:
    options::Double initial_threshold_{60.};
    options::Bool invert_signal_{true};
    options::Measurement<double,false> buffer_size_{
        0.5,
        "ms",
        options::positive<double>(true)
    };
    options::Bool strict_time_bin_check_{true};
    options::Measurement<unsigned int,false> initial_peak_lifetime_{
        8,
        "sample"
    };
    
};

#endif // spikedetector.h
