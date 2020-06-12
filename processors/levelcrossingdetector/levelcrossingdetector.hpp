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

/* LevelCrossingDetector: detects a threshold crossing on any of the
 * channels in the incoming MultiChannelData stream and emits an event
 * in response
 * 
 * input ports:
 * data <MultiChannelType> (1 slot)
 *
 * output ports:
 * events <EventType> (1 slot)
 *
 * exposed states:
 * threshold <double> - threshold that needs to be crossed
 * upslope <bool> - whether to look for upward (true) or downward (false)
 *   threshold crossings
 * post_detect_block <unsigned int> - refractory period after threshold 
 *   crossing detection (in number of samples )
 *
 * exposed methods:
 * none
 *
 * options:
 * threshold <double> - default threshold state
 * upslope <bool> - default upslope state
 * post_detect_block <unsigned int> - default post_detect_block state
 * event <string> - event to emit upon detection of threshold crossing
 * 
 */

#ifndef LEVELCROSSINGDETECTOR_HPP
#define LEVELCROSSINGDETECTOR_HPP

#include "iprocessor.hpp"
#include "eventdata/eventdata.hpp"
#include "multichanneldata/multichanneldata.hpp"


class LevelCrossingDetector : public IProcessor {

public:

    LevelCrossingDetector() : IProcessor() {
        add_option("threshold", initial_threshold_, "Threshold (in data units) that needs to be crossed.");
        add_option("upslope", initial_upslope_,"Either detect upward (true) or downward (false) threshold crossings.");
        add_option("post_detect_block", initial_post_detect_block_, "Refractory period after threshold crossing detection (in number of samples).");
        add_option("event", event_prototype_, "The event to emit when the input signal crosses the threshold.");
    }

    virtual void Configure( const YAML::Node  & node, const GlobalContext& context) override;
    virtual void CreatePorts( ) override;
    virtual void Preprocess( ProcessingContext& context) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override;
    
protected:
    PortIn<MultiChannelType<double>>* data_in_port_;
    PortOut<EventType>* data_out_port_;
    
    StaticState<double>* threshold_;
    StaticState<bool>* upslope_;
    StaticState<unsigned int>* post_detect_block_;
    
    std::vector<double> previous_sample_;
    uint64_t n_detections_;
    
    MultiChannelType<double>::Data* data_in_;
    EventType::Data* data_out_;
    
protected:
    void post_detection_block_update(
        unsigned int post_detection_block );

public:

    const std::string DEFAULT_EVENT = "threshold_crossing";  
    const unsigned int LOW_POST_DETECTION_BLOCK_US = 30;

    const std::string THRESHOLD_S = "threshold";
    const std::string UPSLOPE_S = "upslope";
    const std::string POST_DETECT_BLOCK_S = "post_detect_block";

// OPTIONS
protected:

    options::Double initial_threshold_{0.0};
    options::Bool initial_upslope_{true};
    options::Value<unsigned int,false> initial_post_detect_block_{2};
    
    options::Value<EventType::Data,false> event_prototype_{
        DEFAULT_EVENT, 
        options::notempty<EventType::Data>()
    };

};

#endif // levelcrossingdetector.hpp
