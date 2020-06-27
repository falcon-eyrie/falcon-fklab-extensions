#include "behaviorsource.hpp"

#include <random>

BehaviorSource::BehaviorSource(){

    add_option("path to linear position", path_to_linear_position_, "");
    add_option("path to speed", path_to_speed_, "");
    add_option("speed range", default_speed_range_, "");
    add_option("linear position range", default_linear_position_range_, "");
    add_option("unit", unit_, "");
    add_option("sample rate", sample_rate_, "");
    add_option("streaming rate", streaming_rate_, "");
    add_option("initial timestamp", initial_timestamp_, "");
    add_option("times from file", times_from_file_, "");
    LOG(INFO) <<"Launch the processor.";

}

void BehaviorSource::Configure(const YAML::Node& node, const GlobalContext& context) {

    if ( path_to_linear_position_() == NO_PATH ) {
        position_from_file_ = false;
    } else {
        position_from_file_ = true;
        path_to_linear_position_.set_value(context.resolve_path( path_to_linear_position_()));
    }

    if ( path_to_speed_() == NO_PATH ) {
        speed_from_file_ = false;
    } else {
        speed_from_file_ = true;
        path_to_speed_.set_value(context.resolve_path( path_to_speed_() ));
    }

    linear_position_range_.reset( new Range<double>(default_linear_position_range_()));
    speed_range_.reset( new Range<double>(default_speed_range_()));
    
    // check configuration parameters and define how behavior data will be generated
    if ( position_from_file_ and !path_exists( path_to_linear_position_() ) ) {
        auto err_msg = "This file path doesn't exit: " + path_to_linear_position_();
        throw ProcessingConfigureError(err_msg, name());
    } else if ( not position_from_file_ ) {
        LOG(INFO) << name() << ". Linear position values will be randomly generated in the range "
            << "[" << linear_position_range_->lower() << "," <<
            linear_position_range_->upper() << "].";
    } else {
        LOG(INFO) << name() << ". Linear position will be loaded from " <<
            path_to_linear_position_() << ".";
    }

    if ( (path_to_speed_() != NO_PATH) and !path_exists( path_to_speed_() ) ) {
        auto err_msg = "This file path doesn't exit: " + path_to_speed_();
        throw ProcessingConfigureError(err_msg, name());
    } else if ( not speed_from_file_ ) {
        LOG(INFO) << name() << ". Speed values will be randomly generated in the range "
            << "[" << speed_range_->lower() << "," << speed_range_->upper() << "].";
    } else {
        LOG(INFO) << name() << ". Speed will be loaded from " << path_to_speed_()
            << ".";
    }
    
    // check origin of timestamps
    if ( times_from_file_() ) {
        path_to_times_.set_value(context.resolve_path( path_to_times_() ));
        if ( !path_exists( path_to_times_() ) ) {
            throw ProcessingConfigureError( "Non existent path inserted: " + path_to_times_(), name() );
        } 
        LOG(INFO) << name() << ". Timestamps will be loaded from " << path_to_times_()
            << ".";
    } else {
        LOG(INFO) << name() << ". Timestamps will be generated from " <<
            initial_timestamp_() << ".";
    }
    
    // check and read remaining options
    if ( not ( (unit_() == PIXEL_S) or (unit_() == CM_S) ) ) {
        auto err_msg = "Invalid behavior unit. Must be either " + PIXEL_S +
            " or " + CM_S + ".";
        throw ProcessingConfigureError( err_msg, name() );
    }
    if ( unit_() == CM_S ) {
        behav_unit_ = BehaviorUnit::CM;
    } else {
        behav_unit_ = BehaviorUnit::PIXEL;
    }

}

void BehaviorSource::CreatePorts() {
    
    data_out_port_ = create_output_port<BehaviorType>(
        BehaviorType::Capabilities(),
        BehaviorType::Parameters(),
        PortOutPolicy( SlotRange(1), RINGBUFFER_SIZE ) );
}

void BehaviorSource::CompleteStreamInfo( ) {
    
    data_out_port_->streaminfo(0).set_parameters( BehaviorType::Parameters(behav_unit_, sample_rate_()) );
    data_out_port_->streaminfo(0).set_stream_rate(streaming_rate_());
    data_out_port_->streaminfo(0).Finalize();
}

void BehaviorSource::Prepare( GlobalContext& context ) {
    
    FILE* fp_linear_position = nullptr;
    FILE* fp_speed = nullptr;
    FILE* fp_time = nullptr;
    
    // initialize with different values (they will be compared for equality)
    auto n_positions = std::numeric_limits<uint32_t>::max();
    auto n_speeds = std::numeric_limits<uint32_t>::max() - 1; 
    auto n_times = std::numeric_limits<uint32_t>::max() - 2;
    
    if ( position_from_file_ ) {
        
        if ( (fp_linear_position = fopen(path_to_linear_position_().c_str(), "r")) == nullptr ) {
            auto err_msg = "Cannot open the NPY file. Check the file path "
                + path_to_linear_position_();
            throw ProcessingPrepareError(err_msg, name());
        }
        
        if ( get_1D_array_len( fp_linear_position, &n_positions ) != 0 ) {
            throw ProcessingPrepareError(
                "Cannot read the length of the NPY file.", name());
        }
        LOG( INFO ) << name() << ". " << n_positions <<
            " position values will be streamed from file.";
    }
    
    if ( speed_from_file_ ) {
        
        if ( (fp_speed = fopen( path_to_speed_().c_str(), "r")) == nullptr ) {
            auto err_msg = "Cannot open the NPY file. Check the file path "
                + path_to_speed_();
            throw ProcessingPrepareError(err_msg, name());
        }
        if ( get_1D_array_len( fp_speed, &n_speeds ) != 0 ) {
            throw ProcessingPrepareError(
                "Cannot read the length of the NPY file.", name());
        }
        LOG( INFO ) << name() << ". " << n_speeds <<
            " speed values will be streamed from file.";
    }
    
    if ( times_from_file_() ) {
        
        if ( (fp_time = fopen( path_to_times_().c_str(), "r")) == nullptr ) {
            auto err_msg = "Cannot open the NPY file. Check the file path "
                + path_to_times_();
            throw ProcessingPrepareError(err_msg, name());
        }
        if ( get_1D_array_len( fp_time, &n_times ) != 0 ) {
            throw ProcessingPrepareError(
                "Cannot read the length of the NPY file.", name());
        }
        LOG( INFO ) << name() << ". " << n_times <<
            " timestamps values will be read from file.";
    }
    
    // check the consistency of the behavior data from file
    if ( position_from_file_ and speed_from_file_ and ( n_positions != n_speeds ) ) {
        throw ProcessingPrepareError(
            "Number of loaded linear positions do not match the number of speed values",
            name());               
    }
    
    // check the consistency of the behavior data from file
    if ( position_from_file_ and speed_from_file_ and times_from_file_()
    and ( n_positions != n_times ) ) {
        throw ProcessingPrepareError(
            "Number of loaded linear positions does not match the number of times",
            name());               
    }
    
    if ( position_from_file_ or speed_from_file_ or times_from_file_() ) {
        LOG(INFO) << name() << ". Behavior data loaded from file is consistent.";
    }
    
    // load data from files
    if ( position_from_file_ ) {
        if ( (loaded_positions_ = get_1D_array_f64( fp_linear_position )) == nullptr ) {
            throw ProcessingPrepareError(
                "Cannot read positions from NPY file.", name());
        }
        LOG(UPDATE) << name() << ". Positions loaded from file.";
    }
    if ( speed_from_file_ ) {
        if ( (loaded_speeds_ = get_1D_array_f64( fp_speed )) == nullptr ) {
            throw ProcessingPrepareError(
                "Cannot read speed values from NPY file.", name());
        }
        LOG(UPDATE) << name() << ". Speed values loaded from file.";
    }
    if ( times_from_file_() ) {
        if ( (loaded_times_ = get_1D_array_f64( fp_time )) == nullptr ) {
            throw ProcessingPrepareError(
                "Cannot read time values from NPY file.", name());
        }
        LOG(UPDATE) << name() << ". Times loaded from file.";
    }
    
    // update user
    if ( position_from_file_ or speed_from_file_ ) {
        n_packets_to_stream_ = std::min( n_positions, n_speeds );
        LOG(INFO) << name() << ". " << n_packets_to_stream_ <<
            " BehaviorData elements will be generated and streamed.";
    } else {
        n_packets_to_stream_ = N_MAX_PACKETS_TO_STREAM;
    }
    
    // close files
    if ( position_from_file_ ) {
        fclose( fp_linear_position ); fp_linear_position = nullptr;
    }
    if ( speed_from_file_ ) {
        fclose( fp_speed ); fp_speed = nullptr;
    }
    
    // generate hardware timestamps
    if ( not times_from_file_() ) {
        
        double last_time_us =   initial_timestamp_() +
                                (n_packets_to_stream_ - 1) * 1e6 / sample_rate_() ;
        std::vector<double> hw_timestamps =
            linspace(   static_cast<double>(initial_timestamp_()), last_time_us,
                        n_packets_to_stream_);
        for (auto& el: hw_timestamps) {
            generated_hw_timestamps_.push_back( static_cast<uint64_t>( std::round(el) ) );
        }
        LOG(DEBUG) << name() << ". First generated TS: " << generated_hw_timestamps_[0]
            << ". Last generated TS: " << generated_hw_timestamps_[n_packets_to_stream_-1];
        LOG_IF(DEBUG, position_from_file_) << name() << ". First position: " <<
            loaded_positions_[0] << ". Last position: " << loaded_positions_[ n_positions-1 ];
        LOG_IF(DEBUG, speed_from_file_) << name() << ". First speed value: " <<
            loaded_positions_[0] << ". Last speed value: " << loaded_speeds_[ n_speeds-1 ];
    
    } else {
        
        generated_hw_timestamps_.resize( n_times );
        for ( unsigned int i=0; i<n_times; ++i ) {
            generated_hw_timestamps_[i] = static_cast<uint64_t>(
                std::round( loaded_times_[i]*1e6 ) ); 
        }
    }
}

void BehaviorSource::Process(ProcessingContext& context) {
    
    BehaviorType::Data* data = nullptr;
    auto delay = std::chrono::microseconds( static_cast<unsigned int>(
        1e6 / streaming_rate_() ) );
    decltype(n_packets_to_stream_) i = 0;
    
    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution_position(
        linear_position_range_->lower(), linear_position_range_->upper() );
    std::uniform_real_distribution<double> distribution_speed(
        speed_range_->lower(), speed_range_->upper() );
    
    while ( !context.terminated() and
            data_out_port_->slot(0)->nitems_produced() < n_packets_to_stream_ ) {
        
        data = data_out_port_->slot(0)->ClaimData( true );
        
        if ( position_from_file_ ) {
            data->set_linear_position( loaded_positions_[i] );
        } else {
            data->set_linear_position( distribution_position(generator) );
        }
        if ( speed_from_file_ ) {
            data->set_speed( loaded_speeds_[i] );
        } else {
            data->set_speed( distribution_speed(generator) );
        }
        
        data->set_hardware_timestamp( generated_hw_timestamps_[i] );
        data->set_source_timestamp();
        
        ++ i;
        
        data_out_port_->slot(0)->PublishData();
        
        std::this_thread::sleep_for( delay );
    }   
}

void BehaviorSource::Postprocess( ProcessingContext& context ) {
    
    auto nstreamed = data_out_port_->slot(0)->nitems_produced();
    if ( ( nstreamed == n_packets_to_stream_ ) ) {
        LOG(INFO) << name() << ". Streamed all " << data_out_port_->slot(0)->nitems_produced()
        << " behavior data packets. Stop the graph (if not already done).";
    } else {
        LOG(INFO) << name() << ". Streamed " << nstreamed << " behavior data packets.";
    }
}

void BehaviorSource::Unprepare( GlobalContext& context ) {

    if ( position_from_file_ ) {
        free( loaded_positions_ ); loaded_positions_ = nullptr;
    }
    if ( speed_from_file_ ) {
        free( loaded_speeds_ ); loaded_speeds_ = nullptr;
    }
}

REGISTERPROCESSOR(BehaviorSource)

