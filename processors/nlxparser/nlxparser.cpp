#include "nlxparser.hpp"

#include "g3log/src/g2log.hpp"

#include <limits>
#include <memory>

constexpr uint16_t NlxParser::MAX_NCHANNELS;
constexpr decltype(NlxParser::MAX_NCHANNELS) NlxParser::UDP_BUFFER_SIZE;
constexpr decltype(NLX_SIGNAL_SAMPLING_FREQUENCY) NlxParser::SAMPLING_PERIOD_MICROSEC;
constexpr decltype(NlxParser::delta_) NlxParser::MAX_ALLOWABLE_TIMEGAP_MICROSECONDS;
constexpr decltype(NlxParser::timestamp_) NlxParser::INVALID_TIMESTAMP;

NlxParser::NlxParser() : IProcessor(PRIORITY_HIGH) {
    
    add_option("batch_size", batch_size_,
        "The number of data packets to concatenate into "
        "single multi-channel data bucket.");
    add_option("nchannels", nchannels_,
        "The number of channels of the Digilynx acquisition system.");
    add_option("update_interval", update_interval_,
        "The time interval for updates on the received data from "
        "the Digilynx acquisition system.");
    add_option("hardware_trigger", dispatch_, 
        "Whether or not to wait for hardware trigger to start "
        "streaming data packets.");
    add_option("hardware_trigger_channel", hardware_trigger_channel_, 
        "Digital input channel to use as hardware trigger");
}

void NlxParser::Configure( const YAML::Node & node, const GlobalContext& context ) {
    
    // number of AD channels of the system
    //nchannels_ = node["nchannels"].as<decltype(nchannels_)>( DEFAULT_NCHANNELS );
    nlxrecord_.set_nchannels( nchannels_() );
    
    // how many packets to pack into single multi-channel data bucket
    //batch_size_ = node["batch_size"].as<decltype(batch_size_)>( DEFAULT_BATCHSIZE );
    
    // whether or not to fill missed packets with the last available sample
    gaps_filling_ = node["gaps_filling"].as<decltype(gaps_filling_)>( DEFAULT_GAPS_FILLING );
    if ( gaps_filling_!="none" and gaps_filling_!="asap" and gaps_filling_!="distributed") {
        auto msg = "Unrecognized gaps filling option (must be none, asap or distributed).";
        throw ProcessingConfigureError( msg, name() );
    }
    
    // how often updates about data stream will be sent out
    //decltype(update_interval_) value = node["update_interval"].as<decltype(
    //    update_interval_)>(DEFAULT_UPDATE_INTERVAL_SEC);
    //update_interval_ = value * NLX_SIGNAL_SAMPLING_FREQUENCY;
    //if (update_interval_==0) {
    //    update_interval_ = std::numeric_limits<uint64_t>::max();
    //}
    
    // whether or not to wait for hardware trigger to start dispatching
    //hardware_trigger_ = node["hardware_trigger"].as<decltype(hardware_trigger_)>(
    //    DEFAULT_HARDWARE_TRIGGER );
    //dispatch_ = !hardware_trigger_;
    // digital input channel to use as hardware trigger
    //hardware_trigger_channel_ = node["hardware_trigger_channel"].as<decltype(
    //    hardware_trigger_channel_)>(DEFAULT_HARDWARE_TRIGGER_CHANNEL);
}

void NlxParser::CreatePorts() {
    
    data_in_port_ = create_input_port<VectorType<char>>(
        "udp",
        VectorType<char>::Capabilities(),
        PortInPolicy(SlotRange(1))
    );
    
    output_port_signal_ = create_output_port<MultiChannelType<double>>(
        "data",
        MultiChannelType<double>::Capabilities( ChannelRange(1,NlxParser::MAX_NCHANNELS) ),
        MultiChannelType<double>::Parameters(),
        PortOutPolicy(SlotRange(1),500));
    
    output_port_ttl_ = create_output_port<MultiChannelType<uint32_t>>(
        "ttl",
        MultiChannelType<uint32_t>::Capabilities( ChannelRange(1) ),
        MultiChannelType<uint32_t>::Parameters(),
        PortOutPolicy(SlotRange(1), 500));
    
    n_invalid_ = create_broadcaster_state<uint64_t>(
        "n_invalid",
        0,
        Permission::READ,
        "The number of invalid packets that were received.");

}

void NlxParser::CompleteStreamInfo() {
    
    output_port_signal_->streaminfo(0).set_parameters(
        MultiChannelType<double>::Parameters(
            nchannels_(),
            batch_size_(), 
            data_in_port_->slot(0)->streaminfo().stream_rate()
        )
    );
    
    output_port_signal_->streaminfo(0).set_stream_rate(
        data_in_port_->slot(0)->streaminfo().stream_rate() / batch_size_() );
    
    output_port_ttl_->streaminfo(0).set_parameters(
        MultiChannelType<double>::Parameters(
            1,
            batch_size_(), 
            data_in_port_->slot(0)->streaminfo().stream_rate()
        )
    );

    output_port_ttl_->streaminfo(0).set_stream_rate(
        data_in_port_->slot(0)->streaminfo().stream_rate() / batch_size_() );
}

void NlxParser::Prepare( GlobalContext& context ) {

    // create channel list
    channel_list_.resize( NlxParser::MAX_NCHANNELS );
    for (unsigned int i=0; i<NlxParser::MAX_NCHANNELS; i++ ) {
        channel_list_[i] = i;
    }
}

void NlxParser::Preprocess( ProcessingContext& context ) {

    sample_counter_ = batch_size_();
    valid_packet_counter_ = 0;
    
    timestamp_ = INVALID_TIMESTAMP;
    last_timestamp_ = INVALID_TIMESTAMP;
    
    stats_.clear_stats();
    n_filling_packets_ = 0;
}

void NlxParser::Process( ProcessingContext& context ) {
      
    bool update_time = false;
    unsigned int i=0;
    int b=0;
    decltype(n_filling_packets_) packets_lag = 0;
    
    VectorType<char>::Data* data_in = nullptr;
    MultiChannelType<double>::Data::sample_iterator data_iter;
    MultiChannelType<double>::Data* data_out = nullptr;
    MultiChannelType<uint32_t>::Data* ttl_data_out = nullptr;
    
    
    while ( !context.terminated() ) {
            
        // if ( context.test() and roundtrip_latency_test_ ) {
        //     test_source_timestamps_[valid_packet_counter_] = Clock::now();
        // }

        if (!data_in_port_->slot(0)->RetrieveData(data_in)) {break;}

        if ( !CheckPacket( data_in->data().data() ) ) {continue;}
        valid_packet_counter_ ++;
        
        data_in_port_->slot(0)->ReleaseData();

        if (valid_packet_counter_==1) {
            first_valid_packet_arrival_time_ = Clock::now();
            LOG(UPDATE) << name() << ". Received first valid data packet" <<
                " (TS = " << timestamp_ << ").";
        }

        if (!dispatch_()) {
            LOG_IF(UPDATE, (valid_packet_counter_ == 1)) << name() <<
                ". Waiting for hardware trigger on channel "
                << hardware_trigger_channel_() << ".";
            if (nlxrecord_.parallel_port() & (1<<hardware_trigger_channel_()) ) {
                dispatch_=true;
                LOG(UPDATE) << name() << ". Dispatching starts now.";
            } else { continue; }
        }

        update_time = valid_packet_counter_%update_interval_()== 0;
        LOG_IF(UPDATE, update_time ) << name() << ": " <<
            valid_packet_counter_ << " packets (" <<
            valid_packet_counter_/data_in_port_->streaminfo(0).stream_rate() <<
                " s) received.";
        print_stats( update_time );

        if (sample_counter_ == batch_size_()) {
            data_out = output_port_signal_->slot(0)->ClaimData(false);
            data_out->set_hardware_timestamp( timestamp_ );
            //data_out->mark_as_authentic();
            ttl_data_out = output_port_ttl_->slot(0)->ClaimData(false);
            ttl_data_out->set_hardware_timestamp( timestamp_ );
            //ttl_data_out->mark_as_authentic();
            sample_counter_ = 0;
        }

        // copy data from current packet onto buffer for each channel 
        data_out->set_sample_timestamp( sample_counter_, timestamp_ );
        ttl_data_out->set_sample_timestamp( sample_counter_, timestamp_ );
        data_iter = data_out->begin_sample( sample_counter_ );
        for (auto & channel : channel_list_) {
            (*data_iter) = nlxrecord_.sample_microvolt(channel);
            ++data_iter;
        }
        ttl_data_out->set_data_sample( sample_counter_, 0, nlxrecord_.parallel_port() );
        ++sample_counter_;

        if (sample_counter_ == batch_size_()) {
            output_port_signal_->slot(0)->PublishData();
            output_port_ttl_->slot(0)->PublishData();
        }
        
        // stream additional packets if there were missed packets
        if ( gaps_filling_ != "none" and sample_counter_ == batch_size_() ) {
            packets_lag = stats_.n_missed - n_filling_packets_;
            if ( packets_lag >= batch_size_() ) {
                for ( b=0; b<packets_lag/batch_size_(); ++b ) {
                    data_out = output_port_signal_->slot(0)->ClaimData(false);
                    LOG(DEBUG) << name() << ". mcd packet timestamp_: " << timestamp_;
                    data_out->set_hardware_timestamp( timestamp_ );
                    //data_out->mark_as_duplicate();
                    ttl_data_out = output_port_ttl_->slot(0)->ClaimData(false);
                    ttl_data_out->set_hardware_timestamp( timestamp_ );
                    //ttl_data_out->mark_as_duplicate();
                    LOG(DEBUG) << name() << ". mcd packet timestamp_: " << timestamp_;
                    for ( i=0; i<batch_size_(); i++ ) {
                        data_out->set_sample_timestamp( i, timestamp_ );
                        ttl_data_out->set_sample_timestamp( i, timestamp_ );
                        data_iter = data_out->begin_sample( i );
                        for (auto & channel : channel_list_) {
                            (*data_iter) = nlxrecord_.sample_microvolt(channel);
                            ++data_iter;
                        }
                        ttl_data_out->set_data_sample( i, 0, nlxrecord_.parallel_port() );
                        LOG(DEBUG) << name() << ". timestamp_: " << timestamp_ << "; i=" << i;
                    }
                    output_port_signal_->slot(0)->PublishData();
                    output_port_ttl_->slot(0)->PublishData();
                    LOG( UPDATE ) << name() << ". Streamed " << batch_size_() <<
                        " duplicated samples to fill missed packets.";
                    n_filling_packets_ +=  batch_size_();
                    if (gaps_filling_ == "distributed" ) {break;}
                }
            }
        }
    }
}

void NlxParser::Postprocess( ProcessingContext& context ) {
    
    std::chrono::milliseconds runtime( 
        std::chrono::duration_cast<std::chrono::milliseconds>(
        Clock::now() - first_valid_packet_arrival_time_) );
    
    LOG(UPDATE) << name()
        << ". Finished reading : "
        << valid_packet_counter_ << " packets received over "
        << static_cast<double>(runtime.count())/1000 << " seconds at a rate of " 
        << valid_packet_counter_/static_cast<double>(runtime.count())/1000 <<
            " packets/second."; 
    print_stats();
    
    LOG(UPDATE) << name() << ". Streamed " << output_port_signal_->slot(0)->nitems_produced()
        << " multi-channel data items.";
    
    // if ( context.test() and roundtrip_latency_test_ ) {
    //     save_source_timestamps_to_disk( valid_packet_counter_ );
    // }
}

void NlxParser::print_stats( bool condition ) {
    
    LOG_IF(UPDATE, condition) << name() << ". Stats report: "
        << n_invalid_->get() <<  " invalid, " 
        << stats_.n_duplicated << " duplicated, " 
        << stats_.n_outoforder << " out of order, " 
        << stats_.n_missed << " missed, " 
        << stats_.n_gaps << " gaps. "
        << n_filling_packets_ << " packets were filled. Synchronous lag: "
        << (stats_.n_missed - n_filling_packets_)/
            data_in_port_->slot(0)->streaminfo().stream_rate() * 1e3 << " ms."; 
}

void NlxParserStats::clear_stats() {
    
    n_duplicated = 0;
    n_outoforder = 0;
    n_missed = 0;
    n_gaps = 0;
}

bool NlxParser::CheckPacket(char * buffer) {
    
    //if (!nlxrecord_.FromNetworkBuffer( buffer, UDP_BUFFER_SIZE, false )) {
    if (!nlxrecord_.FromNetworkBuffer( buffer, UDP_BUFFER_SIZE )) {
        n_invalid_->set( n_invalid_->get() + 1 );
        LOG(UPDATE) << name() << ": Received invalid record.";
        return false;
    }
    
    timestamp_ = nlxrecord_.timestamp();
    
    if ( last_timestamp_ == INVALID_TIMESTAMP ) {
        last_timestamp_ = timestamp_;
    } else if ( timestamp_ == last_timestamp_ ) {
        ++stats_.n_duplicated;
    } else if ( timestamp_ < last_timestamp_ ) {
        ++stats_.n_outoforder;
    } else {
        delta_ = timestamp_ - last_timestamp_;
        if ( delta_ > MAX_ALLOWABLE_TIMEGAP_MICROSECONDS ) {
            int64_t n_missed = round ( delta_ / SAMPLING_PERIOD_MICROSEC ) - 1;
            stats_.n_missed += n_missed;
            ++stats_.n_gaps;
            LOG(DEBUG) << n_missed << " timestamps were found to be missing. ";
        }
        last_timestamp_ = timestamp_;
    }
    
    return true;
}
