/* 
 * BehaviorSource: streams BehaviorData that can be loaded from two NPY files.
 * One NPY file contains the values of the linearized position that must be streamed:
 * if this file file is not listed, random positions values from a specified range
 * will be generated.
 * A second NPY file contains the corresponding speed values of each linear position; 
 * if this file file is not listed, random positions values from a specified range
 * will be generated.
 * 
 * input ports:
 * none
 * 
 * output ports:
 * data <BehaviorData> (1 slot)
 * 
 * exposed states:
 * none
 *
 * exposed methods:
 * none
 * 
 * options:
 * path_to_linear_position <string> - full path of the NPY file containing linearized position values
 * linear_position_range <array> - range for random generation of linear position
 * speed_range <array> - range for random generation of speed values
 * path_to_speed <string> - full path of the NPY file containing speed values
 * unit <string> - unit of BehaviorData to stream (cm or pixel)
 * sample_rate <double> - sample rate of the videostream used to measure
 * the original data samples
 * streaming_rate <double> - (approximate) streaming rate of the generated
 * behavioral data samples
 * initial_timestamp <uint64_t> - first timestamp of the stream
 * times_from_file <bool> - if True, times will be read from a NPY file
 * path_to_times <string> - full path to the NPY file containing an array of times,
 * if path is valid and the previous option is set to True, timestamps will be generated
 * using these values and not using the initial timestamp
 */

#ifndef BEHAVIOR_SOURCE_HPP
#define	BEHAVIOR_SOURCE_HPP

#include "iprocessor.hpp"
#include "behaviordata/behaviordata.hpp"
#include "npyreader/npyreader.h"
#include "neuralynx/nlx.hpp"
#include "utilities/math_numeric.hpp"
#include "options/options.hpp"
#include "options/units.hpp"

#include <array>


class BehaviorSource : public IProcessor {
    
public:
    BehaviorSource();
    virtual void Configure( const YAML::Node& node, const GlobalContext& context) override;
    virtual void CreatePorts() override;
    virtual void CompleteStreamInfo() override;
    virtual void Prepare( GlobalContext& context ) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override;
    virtual void Unprepare( GlobalContext& context ) override;  

protected:
    PortOut<BehaviorType>* data_out_port_;
    
    options::String path_to_linear_position_{NO_PATH};
    options::String path_to_speed_{NO_PATH};
    options::String path_to_times_{NO_PATH};
    options::String unit_{CM_S};


    options::Measurement<uint64_t,false> initial_timestamp_{
        0,
        "microseconds",
        options::positive<uint64_t>(false)
    };

    options::Double sample_rate_{
        DEFAULT_SAMPLE_RATE,
        options::positive<double>(false)
    };
    options::Double streaming_rate_{
        DEFAULT_STREAMING_RATE,
        options::positive<double>(false)
    };

    options::Value<std::vector<double>> default_linear_position_range_{{ 0, 200 }};
    options::Value<std::vector<double>> default_speed_range_{{ 0, 10 }};
    options::Bool times_from_file_{false};

    bool position_from_file_;
    bool speed_from_file_;

    std::unique_ptr<Range<double>> linear_position_range_;
    std::unique_ptr<Range<double>> speed_range_;
    BehaviorUnit behav_unit_;
    FILE* fp_position_;
    FILE* fp_speed_;
    double* loaded_positions_;
    double* loaded_speeds_;
    double* loaded_times_;
    
    std::vector<uint64_t> generated_hw_timestamps_;
    std::uint32_t n_packets_to_stream_;

public:
    const double DEFAULT_SAMPLE_RATE = nlx::NLX_VIDEO_SAMPLING_FREQUENCY;
    const double DEFAULT_STREAMING_RATE = DEFAULT_SAMPLE_RATE;
    const std::uint32_t N_MAX_PACKETS_TO_STREAM = 900000; // = 10h at 25Hz
    
protected:
    const std::string NO_PATH = "";
    const int RINGBUFFER_SIZE = 1e5;
};

#endif	// behaviorsource.hpp

