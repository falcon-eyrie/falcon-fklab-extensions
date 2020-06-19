/* NlxPureReader: reads raw data of a Neuralynx Digilynx data acquisition 
 * on a UDP buffer
 * 
 * input ports:
 * none
 *
 * output ports:
 * udp <VectorData<char>> (1 slot)
 *
 * states:
 * n_invalid (broadcast) number of received invalid data packets
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
