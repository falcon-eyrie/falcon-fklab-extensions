/* 
 * SpikeDataStreamer: generates SpikeData using data loaded from disk.
 * 
 * The user must enter a path to the flattened spike amplitudes (NPY 1D-array),
 * a path to the spike times (NPY 1D-array), the number of channels used for
 * detecting the spikes and a vector (NPY 1D-array) of starting times.
 * The vector of starting times must contain at least one element. If only one
 * element is present no restrictions are applied to the buffer size; otherwise,
 * it is assumed that the data has been previously binned (the bin size is extracted
 * from the first two samples) and therefore the buffer size cannot be greater
 * than the bin size of the loaded data. Each element of the vector corresponds
 * to the starting time of the bin in which the data have bin partitioned.
 * Times are expected to be in seconds,
 * while spike amplitudes are expected to be in microvolts.
 * 
 * 
 * output ports:
 * spikes <SpikeData> (1 slot)
 * 
 * exposed states:
 * none
 *
 * exposed methods:
 * none
 * 
 * options:
 * path_to_spikes <string> - full path of the NPY file containing the spike amplitudes,
 * if the path does not end in 'npy' an automatic completion will be done using the
 * last characters of the processor name (if they are digits) and the 'npy' extension
 * will be added
 * path_to_spike_times <string> - full path of the NPY file containing the spike times,
 * if the path does not end in 'npy' an automatic completion will be done using the
 * last characters of the processor name (if they are digits) and the 'npy' extension
 * will be added
 * path_to_initial_times <string> - full path of the NPY file containing the initial times,
 * if the path does not end in 'npy' an automatic completion will be done using the
 * last characters of the processor name (if they are digits) and the 'npy' extension
 * will be added
 * n_channels <int> - number of channels of SpikeData
 * path_to_nchannels <> - path to NPY file with the number of channels; it can be
 * used in alternative to n_channels (if n_channels is present,
 * the file will be ignored)
 * buffer_size_ms <double> - buffer size of the generated spike data item [ms]
 * sample_rate <double> - sample rate of the signal used for detecting the loaded spikes
 * streaming_rate <double> - (approximate) streaming rate of each generated SpikeData item 
 * 
 */

#ifndef SPIKE_STREAMER_HPP
#define	SPIKE_STREAMER_HPP

#include "iprocessor.hpp"
#include "spikedata/spikedata.hpp"
#include "neuralynx/nlx.hpp"
#include "npyreader/npyreader.h"
#include "options/options.hpp"

#include "options/units.hpp"
#include <limits>


class SpikeStreamer : public IProcessor {
    
public:
    SpikeStreamer( );
    virtual void Configure(const YAML::Node& node, const GlobalContext& context) override;
    virtual void CreatePorts() override;
    virtual void CompleteStreamInfo() override;
    virtual void Prepare( GlobalContext& context ) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Unprepare( GlobalContext& context ) override;  

protected:
    PortOut<SpikeType>* output_port_;
    
    options::String path_to_spikes_{};
    options::String path_to_spike_times_{};
    options::String path_to_initial_times_{};
    options::String path_to_nchannels_{};
    options::Value<unsigned int, false> n_channels_{};
    options::Measurement<double,false>  buffer_size_{
        DEFAULT_BUFFERSIZE,
        "ms",
        options::positive<double>(true)
    };
    options::Double sample_rate_{
        nlx::NLX_SIGNAL_SAMPLING_FREQUENCY,
        options::positive<double>(true)
    };
    options::Double streaming_rate_{
        nlx::NLX_SIGNAL_SAMPLING_FREQUENCY / (1000 * DEFAULT_BUFFERSIZE),
        options::inrange<double>(0, HIGH_DATA_STREAM_RATE*(buffer_size_()*1e3)/sample_rate_())
    };
    
    double* loaded_spike_amplitudes_;
    double* loaded_spike_times_;
    double* loaded_initial_times_;
    
    uint32_t n_spikes_;
    uint32_t n_packets_to_stream_;
    std::vector<double> time_limits_;
    uint32_t n_bins_;
    
    const double ERROR_ = 1e5 * std::numeric_limits<double>::epsilon();

public:
    static constexpr decltype(nlx::NLX_SIGNAL_SAMPLING_FREQUENCY) HIGH_DATA_STREAM_RATE =
        2 * nlx::NLX_SIGNAL_SAMPLING_FREQUENCY;
    
protected:
    // DEBUG update every time  UPDATE_PERC% of n_packets_to_stream_ have been streamed
    const unsigned int UPDATE_PERC = 10;
    const unsigned int NO_CHANNEL_NUMBER =
        std::numeric_limits<unsigned int>::max();
    
protected:
    const int RINGBUFFER_SIZE = 1e4;
    const double DEFAULT_BUFFERSIZE = 1.0;
};

#endif	// spikestreamer.hpp

