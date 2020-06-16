/* NlxPureReader: reads raw data of a Neuralynx Digilynx data acquisition 
 * on a UDP buffer
 * 
 * input ports:
 * none
 *
 * output ports:
 * udp <VectorData<char>> (1 slot)
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
 * npackets <uint64_t> - number of raw data packets to read before
 *   exiting (0 = continuous streaming)
 * 
 */

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
    
public:
    NlxPureReader();
    
    virtual void Configure( const YAML::Node  & node, const GlobalContext& context ) override;
    virtual void CreatePorts() override;
    virtual void CompleteStreamInfo() override;
    virtual void Prepare( GlobalContext& context ) override;
    virtual void Preprocess( ProcessingContext& context ) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override;
    
public:
    static constexpr uint16_t MAX_NCHANNELS = 128;
    static constexpr decltype(MAX_NCHANNELS) UDP_BUFFER_SIZE =
        NLX_PACKETBYTESIZE(MAX_NCHANNELS);
    
// config options
protected:
    //std::string address_;
    //unsigned int port_;
    //std::uint64_t npackets_;

// internals
protected:
    PortOut<VectorType<char>>* output_port_;
    BroadcasterState<uint64_t>* n_invalid_;
    
    fd_set file_descriptor_set_;
    int udp_socket_;
    int udp_socket_select_;
    struct sockaddr_in server_addr_; 
    
    uint64_t valid_packet_counter_;
    struct timeval timeout_;
    TimePoint first_valid_packet_arrival_time_;
    
    ssize_t size_;
    int recvlen_;
    VectorType<char>::Data* data_out_;
    
public:
    static constexpr decltype(NLX_SIGNAL_SAMPLING_FREQUENCY)
        SAMPLING_PERIOD_MICROSEC = 1e6 / NLX_SIGNAL_SAMPLING_FREQUENCY;
    //const std::string DEFAULT_ADDRESS = "127.0.0.1"; //testbench
    //const decltype(port_) DEFAULT_PORT = 5000;
    //const decltype(npackets_) DEFAULT_NPACKETS = 0;
    const decltype(timeout_.tv_sec) TIMEOUT_SEC = 3;

//OPTIONS
protected:
    options::String address_{"127.0.0.1"};
    options::Value<unsigned int,false> port_{5000};
    options::Value<std::uint64_t,false> npackets_{
        0,
        options::zeroismax<std::uint64_t>()
    };
};

#endif // nlxpurereader.hpp
