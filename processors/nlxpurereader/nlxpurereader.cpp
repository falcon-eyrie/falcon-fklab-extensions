#include "nlxpurereader.hpp"

#include "g3log/src/g2log.hpp"

#include <limits>
#include <memory>

constexpr uint16_t NlxPureReader::MAX_NCHANNELS;
constexpr decltype(NlxPureReader::MAX_NCHANNELS) NlxPureReader::UDP_BUFFER_SIZE;

 NlxPureReader::NlxPureReader() : IProcessor( PRIORITY_HIGH ) {
    add_option("address", address_, 
        "IP address of Digilynx acquisition system.");
    add_option("port", port_,
        "Port number for communication with Digilynx acquisition system.");
    add_option("npackets", npackets_,
        "The total number of data packets to read "
        "(0 means continuous recording).");
 }

void NlxPureReader::Configure( const YAML::Node & node, const GlobalContext& context ) {
    
    // address_ = node["address"].as<decltype(address_)>(DEFAULT_ADDRESS);
    
    // port_ = node["port"].as<decltype(port_)>(DEFAULT_PORT);
    
    // // npackets : int (number of packets to read, 0 means continuous recording)
    // npackets_ = node["npackets"].as<decltype(npackets_)>(DEFAULT_NPACKETS);
    // if (npackets_==0) {
    //     npackets_ = std::numeric_limits<decltype(npackets_)>::max();
    // }
    
    // roundtrip_latency_test_ = node["roundtrip_latency_test"].as<decltype(roundtrip_latency_test_)>(
    //     DEFAULT_LATENCY_TEST );
}

void NlxPureReader::CreatePorts() {
    
    output_port_ = create_output_port<VectorType<char>>(
        "udp",
        VectorType<char>::Capabilities(),
        VectorType<char>::Parameters(UDP_BUFFER_SIZE),
        PortOutPolicy(SlotRange(1), 500, WaitStrategy::kBlockingStrategy));
    
    n_invalid_ = create_broadcaster_state<uint64_t>(
        "n_invalid",
        0,
        Permission::READ,
        "The number of invalid packets that were received.");
}

void NlxPureReader::CompleteStreamInfo() {
    output_port_->streaminfo(0).set_stream_rate(NLX_SIGNAL_SAMPLING_FREQUENCY);
}

void NlxPureReader::Prepare( GlobalContext& context ) {
    
    LOG(INFO) << name() << ". Data will be read from " << address_() << ":" << port_() << ".";
    
    memset((char *)&server_addr_, 0, sizeof(server_addr_));
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_addr.s_addr = inet_addr(address_().c_str());
    server_addr_.sin_port = htons(port_());
}

void NlxPureReader::Preprocess( ProcessingContext& context ) {

    valid_packet_counter_ = 0;
    const int reuse_address = 1;
    
    n_invalid_->set( 0 );
    
    sleep(1); // reduces probability of missed packets when connecting to ongoing stream
    
    if ( (udp_socket_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        throw ProcessingPrepareError( "Unable to create socket.", name() );
    }
    
    LOG(UPDATE) << name() << ". Socket created.";
    
    setsockopt(udp_socket_, SOL_SOCKET, SO_REUSEADDR, &reuse_address, sizeof(int));
    if ( bind(udp_socket_, (struct sockaddr *)&server_addr_, sizeof(server_addr_)) < 0 ) {
        throw ProcessingPreprocessingError( "Socket binding failed.", name() );
    }
    
    LOG(UPDATE) << name() << ". Socket binding successful.";
    
    udp_socket_select_ = udp_socket_ + 1;
}

void NlxPureReader::Process( ProcessingContext& context ) {
    
    while (!context.terminated() && valid_packet_counter_<npackets_() ) {
        
        // check if packets have arrived (with time-out)
        FD_ZERO (&file_descriptor_set_); //clear the file descriptor set
        FD_SET  (udp_socket_, &file_descriptor_set_); //add the socket (it's basically acting like a filedescriptor) to the set
        
        // set time-out
        timeout_.tv_sec = TIMEOUT_SEC;
        timeout_.tv_usec = 0;
        
        // packets available?
        size_ = select(udp_socket_select_, &file_descriptor_set_, 0, 0, &timeout_);
        
        if (size_ == 0) {
            LOG(DEBUG) << name() << ": Timed out waiting for data. Connection lost?";
            continue;
        } else if (size_ == -1) {
            LOG(DEBUG) << name() << ": Select error on UDP socket.";
            continue;
        } else if (size_ > 0) { // received packet
            
            data_out_ = output_port_->slot(0)->ClaimData(false);
            
            recvlen_ = recvfrom( udp_socket_, data_out_->data().data(),
                UDP_BUFFER_SIZE, 0, NULL, NULL);
            
            if ( recvlen_ != UDP_BUFFER_SIZE ) {
                n_invalid_->set( n_invalid_->get() + 1 );
                LOG(UPDATE) << name() << ". Received invalid record.";
                continue;
            } else {
                valid_packet_counter_++;
            }
            
            
            if (valid_packet_counter_==1) {
                first_valid_packet_arrival_time_ = Clock::now();
                LOG(UPDATE) << name() << ". Received first UDP data packet.";
            }
            
            data_out_->set_source_timestamp();          
            output_port_->slot(0)->PublishData();
            
        } else {
            throw ProcessingError( "Unexpected size value returned.", name());
        }
    }
    
    LOG(UPDATE ) << name() <<
        ". Requested number of packets was read. You can now STOP the processing.";
}

void NlxPureReader::Postprocess( ProcessingContext& context ) {
    
    std::chrono::milliseconds runtime( 
        std::chrono::duration_cast<std::chrono::milliseconds>(
        Clock::now() - first_valid_packet_arrival_time_) );
    
    LOG(UPDATE) << name()
        << ". Finished reading : "
        << valid_packet_counter_ << " packets received over "
        << static_cast<double>(runtime.count())/1000 << " seconds at a rate of " 
        << valid_packet_counter_/static_cast<double>(runtime.count())/1000
        << " packets/second."; 
    
    close( udp_socket_ );
    
    LOG(UPDATE) << name() << ". Streamed " << output_port_->slot(0)->nitems_produced()
        << " multi-channel data items.";
}
