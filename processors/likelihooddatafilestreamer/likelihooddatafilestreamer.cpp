#include "likelihooddatafilestreamer.hpp"

#include "g3log/src/g2log.hpp"
#include "utilities/math_numeric.hpp"
#include "utilities/string.hpp"

LikelihoodDataFileStreamer::LikelihoodDataFileStreamer(){

    add_option("path to likelihood", path_to_likelihood_,"");
    add_option("path to n spikes", path_to_n_spikes_, "");
    add_option("time bin", time_bin_, "");
    add_option("sample rate", sample_rate_, "");
    add_option("streaming rate", streaming_rate_, "");
    add_option("initial timestamp", initial_timestamp_, "");

}


void LikelihoodDataFileStreamer::Configure(const YAML::Node& node, const GlobalContext& context) {
    
    path_to_likelihood_.set_value(context.resolve_path(path_to_likelihood_()));
    path_to_n_spikes_.set_value( context.resolve_path(path_to_n_spikes_()) );
    
    // open the likelihood NPY file and 
    // extract the grid size and the number of items that have to be streamed
    if ( (fp_likelihood_ = fopen(path_to_likelihood_().c_str(), "r")) == NULL ) {
        throw ProcessingConfigureError(
            "Cannot open the NPY file. Check the file path " + path_to_likelihood_(),
            name());
    }
    
    if ( get_2D_matrix_shape(fp_likelihood_, &n_packets_to_stream_, &grid_size_) != 0 ) {
        throw ProcessingPrepareError("Cannot read the shape of the NPY file.",
            name());
    } else {
        LOG(INFO) << name() << ". Loaded " << n_packets_to_stream_ <<
            " likelihoods having a grid size of " << grid_size_ << ".";
    }
}

void LikelihoodDataFileStreamer::CreatePorts() {
    
    data_out_port_ = create_output_port<LikelihoodType>(
        "estimates",
        LikelihoodType::Capabilities(),
        LikelihoodType::Parameters(grid_size_),
        PortOutPolicy( SlotRange(1) ) );
}

void LikelihoodDataFileStreamer::CompleteStreamInfo( ) {
    data_out_port_->streaminfo(0).set_parameters(LikelihoodType::Parameters(grid_size_));
    data_out_port_->streaminfo(0).Finalize();
}

void LikelihoodDataFileStreamer::Prepare( GlobalContext& context) {
    
    // open the n_spikes NPY file
    if ( (fp_n_spikes_ = fopen(path_to_n_spikes_().c_str(), "r")) == NULL ) {
        throw ProcessingPrepareError(
            "Cannot open the NPY file. Check the file path " + path_to_likelihood_(),
            name());
    }
    
    // check the consistency of the number of items that have to be streamed
    uint32_t n_bins = 0;
    if ( get_1D_array_len( fp_n_spikes_, &n_bins ) != 0 ) {
        throw ProcessingPrepareError(
            "Cannot read the length of the NPY file", name());
    } else if (n_bins != n_packets_to_stream_) {
        throw ProcessingPrepareError(
            "Number of likelihood bins is inconsistent between in the two files",
            name());
    }
    
    // load the data
    if ( (loaded_log_likelihoods_ = get_2D_matrix_f64( fp_likelihood_ )) == NULL ) {
        throw ProcessingPrepareError(
            "Cannot load the likelihood data from the NPY file", name());
    }
    if ( (loaded_n_spikes_ = retrieve_npy_int32( fp_n_spikes_ )) == NULL ) {
        throw ProcessingPrepareError(
            "Cannot load the number of spikes from the NPY file", name());
    }
    LOG(INFO) << name() << ". Data was loaded correctly from the two files.";
    
    // generate hardware timestamps
    double last_time_us =   initial_timestamp_() +
                            (n_packets_to_stream_ - 1) * time_bin_() * 1e3;
    
    std::vector<double> hw_timestamps =
        linspace(   static_cast<double>(initial_timestamp_()), last_time_us,
                    n_packets_to_stream_);
    for (auto& el: hw_timestamps) {
        generated_hw_timestamps_.push_back( static_cast<uint64_t>( std::round(el) ) );
    }
    LOG(DEBUG) << name() << ". First generated TS: " << generated_hw_timestamps_[0]
        << ". Last generated TS: " << generated_hw_timestamps_[n_packets_to_stream_-1];
    LOG(DEBUG) << name() << ". First n_spike: " << loaded_n_spikes_[0]
        << ". Last n_spike: " << loaded_n_spikes_[n_packets_to_stream_-1];
}

void LikelihoodDataFileStreamer::Process(ProcessingContext& context) {
    
    LikelihoodType::Data* data = nullptr;
    auto delay = std::chrono::microseconds( static_cast<unsigned int>(
        1e6 / streaming_rate_() ) );
    decltype(n_packets_to_stream_) i = 0;
    
    while ( !context.terminated() &&
            data_out_port_->slot(0)->nitems_produced() < n_packets_to_stream_ ) {
        
        data = data_out_port_->slot(0)->ClaimData( true );
        
        data->set_time_bin( time_bin_() );
        data->add_spikes( loaded_n_spikes_[i] );
        data->set_log_likelihood( loaded_log_likelihoods_[i], grid_size_ );
        data->set_hardware_timestamp( generated_hw_timestamps_[i] );
        data->set_source_timestamp();
        
        ++ i;
        
        data_out_port_->slot(0)->PublishData();
        
        std::this_thread::sleep_for( delay );
    }   
}

void LikelihoodDataFileStreamer::Postprocess( ProcessingContext& context ) {
    
    LOG(INFO) << name()<< ". STREAMED " << data_out_port_->slot(0)->nitems_produced()
        << " data packets. PRESS 's' to STOP THE GRAPH";
}

void LikelihoodDataFileStreamer::Unprepare( GlobalContext& context) {

    fclose(fp_likelihood_); fp_likelihood_ = nullptr;
    fclose(fp_n_spikes_); fp_n_spikes_ = nullptr;
    for (decltype(n_packets_to_stream_) i=0; i < n_packets_to_stream_; ++i) {
        free(loaded_log_likelihoods_[i]); loaded_log_likelihoods_[i] = nullptr;
    }
    free(loaded_n_spikes_); loaded_n_spikes_ = nullptr;
    free(loaded_log_likelihoods_); loaded_log_likelihoods_ = nullptr;
}

REGISTERPROCESSOR(LikelihoodDataFileStreamer)
