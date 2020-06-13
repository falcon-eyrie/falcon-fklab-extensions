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

/* Rebuffer: rebuffers and downsamples multiple MultiChannelData streams.
 * No anti-aliasing filter is applied before downsampling.
 * 
 * input ports:
 * data <MultiChannelType> (1-256 slots)
 *
 * output ports:
 * data <MultiChannelType> (1-256 slots)
 *
 * exposed states:
 * none
 *
 * exposed methods:
 * none
 *
 * options:
 * buffer_size : output buffer size in samples or time
 * buffer_unit : samples or seconds
 * downsample : downsample factor
 * 
 */

#ifndef REBUFFER_HPP
#define REBUFFER_HPP

#include "iprocessor.hpp"
#include "multichanneldata/multichanneldata.hpp"

//#include "neuralynx/nlx.hpp"
#include "utilities/time.hpp"

#include "options/options.hpp"
#include "options/units.hpp"

class Rebuffer : public IProcessor
{
// CONSTRUCTOR and OVERLOADED METHODS
public:

    Rebuffer();

    virtual void Configure( const YAML::Node  & node, const GlobalContext& context) override;
    virtual void CreatePorts( ) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void CompleteStreamInfo( ) override;

// DATA PORTS
protected:   
    PortIn<MultiChannelType<double>>* data_in_port_;
    PortOut<MultiChannelType<double>>* data_out_port_;

// variables
protected:
    std::vector<unsigned int> sample_buffer_;

// OPTIONS
protected:

    options::Value<unsigned int, false> downsample_factor_{
        1,
        options::positive<unsigned int>(true)
    };
    
    options::MultiMeasurement<double,false> buffer_size_{
        10.,
        units::precise::sample_units,
        {units::precise::second},
        options::positive<double>()
    };

};

#endif //rebuffer.hpp
