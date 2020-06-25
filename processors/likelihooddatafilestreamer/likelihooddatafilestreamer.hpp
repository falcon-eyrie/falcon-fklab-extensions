/* 
 * LikelihoodDataFileStreamer: streams LikelihoodData loaded from two NPY files.
 * One NPY file must contain the log_likelihood values that have to be streamed 
 * organized as a 2D array of size (m, n), with:
 * m : number of time bins
 * n : grid size
 * The other NPY file must contain the number of spikes related to each likelihood
 * in the previous file; the values should be organized as a 1D array of size n
 * (the numbers must be save as int32).
 * The time bin is assumed constant through the streaming and must be set with
 * a dedicated option.
 * 
 * input ports:
 * none
 * 
 * output ports:
 * data <LikelihoodData> (1 slot)
 * 
 * exposed states:
 * none
 *
 * exposed methods:
 * none
 * 
 * options:
 * path_to_likelihood <string> - full path of the NPY file containing log likelihood values
 * path_to_n_spikes <string> - full path of the NPY file containing the  number
 * of spikes for each time bin
 * time_bin_ms <double> - time bin of the likelihoods that will be streamed [ms]
 * sample_rate <double> - sample rate of the signal used for detecting the spikes
 * that generated the likelihood
 * streaming_rate <double> - (approximate) streaming rate of the each
 * generated LikelihoodData item 
 * initial_timestamp <uint64_t> - timestamp of the first streamed LikelihoodData packet
 */

#ifndef LIKELIHOODDATA_FILESTREAMER_HPP
#define	LIKELIHOODDATA_FILESTREAMER_HPP

#include "iprocessor.hpp"
#include "likelihooddata/likelihooddata.hpp"
#include "npyreader/npyreader.h"
#include "neuralynx/nlx.hpp"
#include "options/options.hpp"
#include "options/units.hpp"

class LikelihoodDataFileStreamer : public IProcessor {
    
public:
    LikelihoodDataFileStreamer();
    virtual void Configure( const YAML::Node& node, const GlobalContext& context) override;
    virtual void CreatePorts() override;
    virtual void CompleteStreamInfo() override;
    virtual void Prepare( GlobalContext& context ) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override;
    virtual void Unprepare( GlobalContext& context ) override;  

protected:
    PortOut<LikelihoodType>* data_out_port_;
    
    options::String path_to_likelihood_{"resources://datatest"};
    options::String path_to_n_spikes_{"resources://datatest"};
    options::Measurement<double,false>  time_bin_{
        DEFAULT_TIMEBIN,
        "ms",
        options::positive<double>(false)
    };
    options::Measurement<uint64_t,false> initial_timestamp_{
        0,
        "microseconds",
        options::positive<uint64_t>(false)
    };
    options::Double sample_rate_{
        nlx::NLX_SIGNAL_SAMPLING_FREQUENCY,
        options::positive<double>(false)
    };
    options::Double streaming_rate_{
        nlx::NLX_SIGNAL_SAMPLING_FREQUENCY / (1000 * DEFAULT_TIMEBIN),
        options::positive<double>(false)
    };

    FILE* fp_likelihood_;
    FILE* fp_n_spikes_;
    double** loaded_log_likelihoods_;
    int32_t* loaded_n_spikes_;

    uint32_t grid_size_;
    std::vector<uint64_t> generated_hw_timestamps_;
    uint32_t n_packets_to_stream_;

public:
    static constexpr double DEFAULT_TIMEBIN = 10.0;

};

#endif	// likelihooddatafilestreamer.hpp

