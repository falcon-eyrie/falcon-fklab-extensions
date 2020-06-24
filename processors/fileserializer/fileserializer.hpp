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

/* FileSerializer: serializes data streams to file
 * 
 * input ports:
 * data <AnyType> (1-256 slots)
 *
 * output ports:
 * none
 *
 * exposed states:
 * none
 *
 * exposed methods:
 * none
 *
 * options:
 * path <string> - server-side path
 * encoding <string> - binary/yaml
 * format <string> - full/nodata/compact
 * overwrite <bool> - overwrite existing file
 * throttle <bool> - throttle saving if we can't keep up
 * throttle_threshold <double> - upstream ringbuffer fill fraction (0-1)
 *   at which throttling takes effect
 * throttle_smooth <double> - smooth level of throttle level (0-1)
 * 
 */

#ifndef FILESERIALIZER_HPP
#define FILESERIALIZER_HPP

#include "iprocessor.hpp"
#include "serializer.hpp"
#include "options/options.hpp"

class FileSerializer : public IProcessor
{
// CONSTRUCTOR and OVERLOADED METHODS
public:
    FileSerializer();

    virtual void CreatePorts() override;
    virtual void Configure(const YAML::Node& node, const GlobalContext& context) override;
    virtual void Preprocess(ProcessingContext& context) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override;

// methods
protected:
    void create_preamble( std::ostream & out, int slot );
    
// DATA PORTS
protected:
    PortIn<AnyType>* data_port_;

// variables
protected:
    std::unique_ptr<Serialization::Serializer> serializer_;
    std::vector<std::unique_ptr<std::ostream>> streams_;
    std::vector<uint64_t> packetid_;
    std::vector<unsigned int> upstream_buffer_size_;
    
    double throttle_level_;
    std::vector<uint64_t> nskipped_;

// OPTIONS
protected:
    options::String path_{"run://"};
    options::Value<Serialization::Encoding,false> encoding_{Serialization::Encoding::BINARY};
    options::Value<Serialization::Format,false> format_{Serialization::Format::FULL};
    options::Bool overwrite_{false};
    options::Bool throttle_{false};
    options::Double throttle_threshold_{
        0.3,
        options::inrange<double>(0.,1.)
    };
    options::Double throttle_smooth_{
        0.5,
        options::inrange<double>(0.,1.)
    };
    options::Bool preamble_{true};
};

#endif //fileserializer.hpp
