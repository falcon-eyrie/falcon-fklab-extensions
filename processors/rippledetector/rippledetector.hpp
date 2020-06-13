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

/* RippleDetector: detects ripples in a MultiChannelData stream and
 * emits an ripple event in response
 * 
 * input ports:
 * data <MultiChannelType> (1 slot)
 *
 * output ports:
 * events <EventType> (1 slot)
 *
 * exposed states:
 * threshold_dev <double> - threshold that needs to be crossed (read-only)
 * mean <double> - signal mean (read-only)
 * deviation <double> - signal deviation (read-only)
 * threshold_deviation <double> - multiplier for threshold (# of deviations)
 * detection_lockout_time_ms <unsigned int> - refractory period after threshold
 *   crossing detection that is not considered for updating of statistics
 *   and for detecting events
 * stream_events <bool> - enable/disable ripple event output
 * stream_statistics <bool> - enable/disable streaming of ripple detection statistics
 *
 * exposed methods:
 * none
 *
 * options:
 * threshold_dev <double> - default threshold multiplier
 * smooth_time <double> - integration time for signal statistics
 * detection_lockout_time_ms <double> - default lockout time ( no 
 * stream_events <bool> - default enable state for streaming events
 * stream_statistics <bool> - enable streaming of statistics
 * statistics_buffer_size <double> - Buffer size (in seconds) for
 *   statistics output buffers
 * statistics_downsample_factor <unsigned int> - downsample factor of streamed
 *   statistics signal
 * 
 */
 
#ifndef RIPPLEDETECTOR_HPP
#define RIPPLEDETECTOR_HPP

#include "iprocessor.hpp"

#include "multichanneldata/multichanneldata.hpp"
#include "eventdata/eventdata.hpp"
#include "dsp/algorithms.hpp"

#include "options/options.hpp"
#include "options/units.hpp"

#include <memory>

class RippleDetector : public IProcessor {

// CONSTRUCTOR and OVERLOADED METHODS
public:
    RippleDetector();

    virtual void CreatePorts( ) override;
    virtual void CompleteStreamInfo( ) override;
    virtual void Preprocess( ProcessingContext& context ) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override;

// methods    
protected:
    double compute_value( MultiChannelType<double>::Data* data_in, unsigned int sample );
    
// DATA PORTS
protected:
    PortIn<MultiChannelType<double>>* data_in_port_;
    PortOut<EventType>* event_out_port_;
    PortOut<MultiChannelType<double>>* stats_out_port_;
    
// STATES
protected:
    ProducerState<double>* threshold_;
    ProducerState<double>* signal_mean_;
    ProducerState<double>* signal_dev_;
    BroadcasterState<bool>* ripple_;
    
    StaticState<double>* threshold_dev_;
    StaticState<double>* detection_lockout_time_;
    StaticState<bool>* stream_events_;
    StaticState<double>* smooth_time_;
    StaticState<bool>* stats_out_;
    
// variables
protected:
    bool stats_out_enabled_;
    std::uint64_t stats_nsamples_;
    
    std::uint64_t block_;
    std::uint64_t burn_in_;
    double sample_rate_;
    
    double acc_;
    
    std::unique_ptr<dsp::algorithms::RunningMeanMAD> running_statistics_;
    std::unique_ptr<dsp::algorithms::ThresholdCrosser> threshold_detector_;

    const unsigned int N_STATS_OUT = 2;

// shared option and state names
public:
    const std::string THRESHOLD_DEV_S = "threshold_dev";
    const std::string SMOOTH_TIME_S = "smooth_time";
    const std::string DETECTION_LOCKOUT_TIME_S = "detection_lockout_time";
    const std::string STREAM_EVENTS_S = "stream_events";
    const std::string STREAM_STATISTICS_S = "stream_statistics";
    
// OPTIONS
protected:

    options::Double initial_threshold_dev_{6.};
    options::Measurement<double,false> initial_smooth_time_{
        10.,
        units::precise::second,
        options::positive<double>(true)
    };
    options::Measurement<double,false> initial_detection_lockout_time_{
        30.,
        units::precise::ms,
        options::positive<double>(true)
    };
    options::Bool default_stream_events_{true};
    options::Bool initial_stats_out_{true};
    options::Measurement<double,false> stats_buffer_size_{
        0.5,
        units::precise::second,
        options::positive<double>(true)
    };
    options::Value<unsigned int, false> stats_downsample_factor_{
        1,
        options::positive<unsigned int>(true)
    };
    options::Bool use_power_{true};
    
};

#endif // rippledetector.hpp
