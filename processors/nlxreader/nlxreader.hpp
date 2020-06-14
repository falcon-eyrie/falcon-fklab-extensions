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

/* NlxReader: reads raw data from a Neuralynx Digilynx data acquisition 
 * system and turns it into multiple MultiChannelData output streams 
 * based on a channel mapping
 * 
 * input ports:
 * none
 *
 * output ports:
 * [configurable] <MultiChannelType> (1 slot)
 *
 * exposed states:
 * none
 *
 * exposed methods:
 * none
 *
 * options:
 * address <string> - IP address of Digilynx system
 * port <unsigned int> - port of Digilynx system
 * nchannels <unsigned int> - number of channels in Digilynx system
 * batch_size <unsigned int> - how many samples to pack into single
 *   MultiChannelData bucket
 * npackets <uint64_t> - number of raw data packets to read before
 *   exiting (0 = continuous streaming)
 * update_interval <unsigned int> - time interval (in seconds) between
 *   log updates
 * channelmap - mapping between AD channels and output ports
 * hardware_trigger <bool> - enable use of hardware triggered dispatching
 * hardware_trigger_channel <uint8> - which DIO channel to use as trigger
 * 
 * extra information:
 * The channelmap defines the output port names and for each port lists 
 * the AD channels that will be copied to the MultiChannelData buckets 
 * on that port. The channelmap option should be specified as follows:
 * 
 * channelmap:
 *   portnameA: [0,1,2,3,4]
 *   portnameB: [5,6]
 *   portnameC: [0,5]
 * 
 */

#ifndef NLXREADER_HPP
#define NLXREADER_HPP

#include <string>
#include <map>
#include <vector>
#include <cstdint>

#include <limits>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#include "iprocessor.hpp"
#include "multichanneldata/multichanneldata.hpp"

#include "neuralynx/nlx.hpp"
#include "utilities/time.hpp"

#include "options/options.hpp"


typedef std::map<std::string,std::vector<unsigned int>> ChannelMap;

struct NlxReaderStats {
    int64_t n_invalid;
    int64_t n_duplicated;
    int64_t n_outoforder;
    int64_t n_missed;
    int64_t n_gaps;
    
    void clear_stats();
};

class NlxReader : public IProcessor 
{
// CONSTRUCTOR and OVERLOADED METHODS
public:
    NlxReader();
    
    virtual void Configure( const YAML::Node  & node, const GlobalContext& context ) override;
    virtual void CreatePorts() override;
    virtual void CompleteStreamInfo() override;
    virtual void Prepare( GlobalContext& context ) override;
    virtual void Preprocess( ProcessingContext& context ) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override;

// methods
protected:
    bool CheckPacket(char * buffer, int recvlen);
    void print_stats( bool condition = true );

// constants
public:
    static constexpr uint16_t MAX_NCHANNELS = 128;
    static constexpr decltype(MAX_NCHANNELS) UDP_BUFFER_SIZE =
        NLX_PACKETBYTESIZE(MAX_NCHANNELS);

// variables
protected:
    fd_set file_descriptor_set_;
    int udp_socket_;
    struct sockaddr_in server_addr_; 
    
    unsigned int sample_counter_;
    uint64_t valid_packet_counter_;
    
    TimePoint first_valid_packet_arrival_time_;
    
    uint64_t timestamp_;
    decltype(timestamp_) last_timestamp_;
    
    struct timeval timeout_;
    
    char buffer_[UDP_BUFFER_SIZE]; //UDP_BUFFER_SIZE is in bytes, so divide by size of int32_t
    NlxSignalRecord nlxrecord_;
    
    NlxReaderStats stats_;
    decltype(timestamp_) delta_;
    
    std::map<std::string, PortOut<MultiChannelType<double>>*> data_ports_;

// constants
public:
    
    static constexpr decltype(NLX_SIGNAL_SAMPLING_FREQUENCY)
        SAMPLING_PERIOD_MICROSEC = 1e6 / NLX_SIGNAL_SAMPLING_FREQUENCY;

    const decltype(timeout_.tv_sec) TIMEOUT_SEC = 3;
    static constexpr decltype(delta_) MAX_ALLOWABLE_TIMEGAP_MICROSECONDS =
        trunc( SAMPLING_PERIOD_MICROSEC ) + 1;
    static constexpr decltype(timestamp_) INVALID_TIMESTAMP =
        std::numeric_limits<decltype(timestamp_)>::max();

// OPTIONS
protected:

    options::Value<ChannelMap,false> channelmap_;
    options::String address_{"127.0.0.1"};
    options::Value<unsigned int,false> port_{5000};
    options::Value<std::uint64_t,false> npackets_{
        0,
        options::zeroismax<std::uint64_t>()
    };
    options::Value<unsigned int,false> batch_size_{1};
    options::Value<unsigned int,false> nchannels_{128};
    options::Measurement<std::uint64_t,false> update_interval_{
        20,
        "second",
        options::zeroismax<std::uint64_t>() +
        options::multiplied<std::uint64_t>(NLX_SIGNAL_SAMPLING_FREQUENCY)
    };
    options::Bool dispatch_{true, options::invert()};
    options::Value<uint32_t,false> hardware_trigger_channel_{0};
    
};

#endif // nlxreader.hpp
