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


#ifndef NLXPUREREADER_HPP
#define NLXPUREREADER_HPP

#include "iprocessor.hpp"
#include "vectordata/vectordata.hpp"

#include "neuralynx/nlx.hpp"
#include "utilities/time.hpp"

#include "options/options.hpp"

#include <limits>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>


class NlxPureReader : public IProcessor {

// CONSTRUCTOR and OVERLOADED METHODS
public:
    NlxPureReader();
    virtual void CreatePorts() override;
    virtual void CompleteStreamInfo() override;
    virtual void Prepare( GlobalContext& context ) override;
    virtual void Preprocess( ProcessingContext& context ) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override;

// PORTS
protected:
    PortOut<VectorType<uint32_t>>* output_port_;

// STATES
protected:
    BroadcasterState<uint64_t>* n_invalid_;
    
// variables
protected:
    fd_set file_descriptor_set_;
    int udp_socket_;
    int udp_socket_select_;
    struct sockaddr_in server_addr_; 
    
    uint64_t valid_packet_counter_;
    struct timeval timeout_;
    TimePoint first_valid_packet_arrival_time_;
    
    ssize_t size_;
    int recvlen_;
    VectorType<uint32_t>::Data* data_out_;

// constants
public:
    const decltype(timeout_.tv_sec) TIMEOUT_SEC = 3;

//OPTIONS
protected:
    options::String address_{"127.0.0.1"};
    options::Value<unsigned int,false> port_{5000};
    options::Value<std::uint64_t,false> npackets_{
        0,
        options::zeroismax<std::uint64_t>()
    };
    options::Value<unsigned int,false> nchannels_{nlx::NLX_DEFAULT_NCHANNELS};
};

#endif // nlxpurereader.hpp
