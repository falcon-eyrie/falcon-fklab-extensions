/* Dispatcher: reads raw data from a Neuralynx Digilynx data acquisition 
 * system and turns it into multiple MultiChannelData output streams 
 * based on a channel mapping
 * 
 * input ports:
 * data <MultiChannelData> (1 slot)
 *
 * output ports:
 * [configurable] <MultiChannelData> (1 slot)
 *
 * exposed states:
 * none
 *
 * exposed methods:
 * none
 *
 * options:
 * batch_size <unsigned int> - how many samples to pack into single
 *   MultiChannelData bucket
 * channelmap - mapping between AD channels and output ports
 * hardware_trigger <bool> - enable use of hardware triggered dispatching
 * hardware_trigger_channel <uint8> - which DIO channel to use as trigger
 * 
 * extra information:
 * The channelmap defines the output port names and for each port lists 
 * the AD channels that will be copied to the MultiChannelData buckets 
 * on that port. The channelmap option should be specified as follows:
 * 
 * channelmap:
 *   portnameA: [0,1,2,3,4]
 *   portnameB: [5,6]
 *   portnameC: [0,5]
 * 
 */

#ifndef DISPATCHER_HPP
#define DISPATCHER_HPP

#include <map>
#include <vector>
 
#include "iprocessor.hpp"
#include "multichanneldata/multichanneldata.hpp"

#include "options/options.hpp"


typedef std::map<std::string,std::vector<unsigned int>> ChannelMap;


class Distributor : public IProcessor {
public:
    Distributor();
    
    virtual void Configure( const YAML::Node  & node, const GlobalContext& context ) override;
    virtual void CreatePorts() override;
    virtual void CompleteStreamInfo() override;
    virtual void Prepare( GlobalContext& context ) override;
    virtual void Preprocess( ProcessingContext& context ) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override;
    
protected:
    PortIn<MultiChannelType<double>>* input_port_;
    std::map<std::string, PortOut<MultiChannelType<double>>*> data_ports_;
    
    //ChannelMap channelmap_;
//    unsigned int n_samples_;
    unsigned int incoming_batch_size_;
    unsigned int max_n_channels_;
    
//public:
//    const decltype(batch_size_) DEFAULT_BATCHSIZE = 1;

// OPTIONS
protected:
    options::Value<ChannelMap,false> channelmap_;
  
};

#endif // dispatcher.hpp
