/* Distributor
 * 
 * reads multi-channel data stream and splits the data across
 * multiple output streams based on a channel mapping
 * 
 * input ports:
 * data <MultiChannelType<double>> (1 slot)
 *
 * output ports:
 * [configurable] <MultiChannelType<double>> (1 slot)
 *
 * options:
 * channelmap - mapping between input channels and output ports
 * 
 * extra information:
 * The channelmap defines the output port names and for each port lists 
 * the channels that will be copied to the data buckets on that port.
 * The channelmap option should be specified as follows:
 * 
 * channelmap:
 *   portnameA: [0,1,2,3,4]
 *   portnameB: [5,6]
 *   portnameC: [0,5]
 * 
 */

#ifndef DISTRIBUTOR_HPP
#define DISTRIBUTOR_HPP

#include <map>
#include <vector>
 
#include "iprocessor.hpp"
#include "multichanneldata/multichanneldata.hpp"

#include "options/options.hpp"


typedef std::map<std::string,std::vector<unsigned int>> ChannelMap;


class Distributor : public IProcessor {

// CONSTRUCTOR and OVERLOADED METHODS
public:
    Distributor();    
    virtual void CreatePorts() override;
    virtual void CompleteStreamInfo() override;
    virtual void Prepare( GlobalContext& context ) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override;

// PORTS
protected:
    PortIn<MultiChannelType<double>>* input_port_;
    std::map<std::string, PortOut<MultiChannelType<double>>*> data_ports_;

// variables
protected:
    //ChannelMap channelmap_;
    //unsigned int n_samples_;
    unsigned int incoming_batch_size_;
    unsigned int max_n_channels_;
    
//public:
//    const decltype(batch_size_) DEFAULT_BATCHSIZE = 1;
// constants
protected:
    const unsigned int MAX_N_CHANNELS = 4096; // maximum number of channels that the distributor can handle
    const int BUFFER_SIZE = 2000; // ring buffer size on the output ports
    const WaitStrategy WAIT_STRATEGY = WaitStrategy::kBlockingStrategy;

// OPTIONS
protected:
    options::Value<ChannelMap,false> channelmap_;
  
};

#endif // distributor.hpp
