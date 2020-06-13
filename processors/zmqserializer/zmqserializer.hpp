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

/* ZMQSerializer: serializes data streams to cloud
 * 
 * input ports:
 * data <IData> (1-256 slots)
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
 * port <unsigned int> - port
 * encoding <string> - binary/yaml
 * format <string> - full/nodata/compact
 * interleaved <bool>
 * 
 */

#ifndef ZMQSERIALIZER_HPP
#define ZMQSERIALIZER_HPP

#include <zmq.hpp>
#include "yaml-cpp/yaml.h"
#include "iprocessor.hpp"
#include "serializer.hpp"

#include "options/options.hpp"

class ZMQSerializer : public IProcessor
{
// CONSTRUCTOR and OVERLOADED METHODS
public:
    ZMQSerializer();

    virtual void CreatePorts() override;
    virtual void Preprocess( ProcessingContext& context ) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override;
    
// DATA PORTS
protected:
    PortIn<AnyType>* data_port_;
    
// OPTIONS
protected:
    options::Value<unsigned int, false> port_{7777};
    options::Value<Serialization::Encoding,false> encoding_{Serialization::Encoding::BINARY};
    options::Value<Serialization::Format,false> format_{Serialization::Format::FULL};
    options::Bool interleave_{false};

// OTHER
protected:
    std::vector<std::unique_ptr<zmq::socket_t>> sockets_;
    std::vector<uint64_t> packetid_;
    std::unique_ptr<Serialization::Serializer> serializer_;
};

#endif //zmqserializer.hpp
