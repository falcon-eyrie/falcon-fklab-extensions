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

#ifndef SPIKEDATA_H
#define SPIKEDATA_H

#include "idata.hpp"
#include "utilities/general.hpp"
//#include "neuralynx/nlx.hpp"

const std::string SPIKEDATA_S = "spikes"; // to be used for port names using spike data

static const double DEFAULT_BUFFER_SIZE_MS = 12.75;
static const unsigned int MAX_N_CHANNELS_SPIKE_DETECTION = 16;
static const unsigned int MAX_N_SPIKES_IN_BUFFER = 100;
static std::vector<uint64_t> zero_timestamps( MAX_N_SPIKES_IN_BUFFER, 0 );
static std::vector<double> zero_amplitudes(
    MAX_N_SPIKES_IN_BUFFER * MAX_N_CHANNELS_SPIKE_DETECTION, 0);

struct ChannelDetection {
    enum Validity {
    UNKNOWN,
    VALID,
    BROKEN,
    NO_PEAK
    };
};

class ChannelValidityMask {
public:
    ChannelValidityMask( unsigned int n_channels = MAX_N_CHANNELS_SPIKE_DETECTION,
        ChannelDetection::Validity validity = ChannelDetection::Validity::UNKNOWN ); 
    
    ~ChannelValidityMask() {}
    
    unsigned int n_channels() const;
    
    std::vector<ChannelDetection::Validity>& validity_mask();
    
    void set_validity( size_t channel_index, ChannelDetection::Validity value);
    
    bool is_channel_valid( size_t channel_index) const;
    
    bool all_channels_valid() const;
    
    void reset(ChannelDetection::Validity value = ChannelDetection::Validity::UNKNOWN);
            
protected:
    std::vector<ChannelDetection::Validity> mask_; 
    
};

class SpikeData : public IData {
public:
    
    struct Parameters : IData::Parameters {
        Parameters( double bufsize = 0., unsigned int nchan = 0,
                    double rate = 0. )
          : buffer_size(bufsize), nchannels(nchan), sample_rate(rate) {}
        
        double buffer_size;
        unsigned int nchannels;
        double sample_rate;
    };
    
    class Capabilities : public IData::Capabilities {
    public:
        Capabilities(ChannelRange channel_range = ChannelRange( 1, MAX_N_CHANNELS_SPIKE_DETECTION )) 
          : channel_range_(channel_range) {}
        
        ChannelRange channel_range() const { return channel_range_; }
        
        virtual void VerifyCompatibility( const Capabilities & capabilities ) const {
            IData::Capabilities::VerifyCompatibility( capabilities );
            if (!channel_range_.overlapping( capabilities.channel_range() )) {
                throw std::runtime_error("Channel ranges do not overlap ("
                                         + channel_range_.to_string() + " and "
                                         + capabilities.channel_range().to_string() + ")");
            }
        }
        virtual void Validate( const Parameters & parameters ) const {
            IData::Capabilities::Validate(parameters);
            if (parameters.nchannels == 0 || !channel_range_.inrange(parameters.nchannels)) {
                throw std::runtime_error("Number of channels cannot be zero and needs to be in range " + channel_range_.to_string());
            }
            if (parameters.buffer_size <= 0) {
                throw std::runtime_error("Buffer size cannot be smaller than or equal to zero.");
            }
            if (parameters.sample_rate <= 0) {
                throw std::runtime_error("Sample rate cannot be smaller than or equal to zero.");
            }
        }
        
    protected:
        ChannelRange channel_range_;
    };
    
    static const std::string datatype() { return "spike"; }

public:
    void Initialize( unsigned int nchannels, size_t max_nspikes, double sample_rate );
    void Initialize( const Parameters & parameters ) {
        
        unsigned int max_nspikes = round (parameters.buffer_size * parameters.sample_rate / 1000) / 2;
        Initialize( parameters.nchannels, max_nspikes, parameters.sample_rate);
    }
    
    virtual void ClearData() override;
	
    unsigned int n_channels() const;
    
    double sample_rate() const;
    
    void add_spike(const std::vector<double>& amplitudes, uint64_t hw_timestamp ); // 1st argument will change to a better interface for matrices
    
    void add_spike(double* amplitudes, uint64_t hw_timestamp);
	
    unsigned int n_detected_spikes() const;
    
    std::vector<double>& amplitudes();
    
    ChannelValidityMask& validity_mask();
    
    const std::vector<uint64_t>& ts_detected_spikes() const; 
    
    const uint64_t ts_detected_spikes( int index ) const;
    
    std::vector<double>::const_iterator spike_amplitudes( std::size_t spike_index ) const;
    
    virtual void SerializeBinary( std::ostream& stream,
        Serialization::Format format = Serialization::Format::FULL ) const override final;
    
    virtual void SerializeYAML( YAML::Node & node,
        Serialization::Format format = Serialization::Format::FULL ) const override final;
    
    virtual void YAMLDescription( YAML::Node & node,
        Serialization::Format format = Serialization::Format::FULL ) const override final;
    
protected:
    uint8_t n_channels_;
    unsigned int n_detected_spikes_;
    std::vector<double> amplitudes_;
    // std::vector<double> widths_;
    std::vector<uint64_t> hw_ts_detected_spikes_;
    double sample_rate_;
    ChannelValidityMask validity_mask_;
    ChannelValidityMask default_validity_mask_; // independent of spike detection outcome
    
    
public:
    static constexpr unsigned int DEFAULT_MAX_NSPIKES = MAX_N_SPIKES_IN_BUFFER; // max expected # of spikes in a buffer
    
protected:
    // for serialization
    const std::string N_CHANNELS_S = "n_channels";
    const std::string N_DETECTED_SPIKES_S = "n_detected_spikes";
    const std::string TS_DETECTED_SPIKES_S = "TS_detected_spikes";
    const std::string SPIKE_AMPLITUDES_S = "spike_amplitudes";
    
};


//class SpikeDataType : public AnyDataType {

//ASSOCIATED_DATACLASS(SpikeData);

//public:
	
    //SpikeDataType( double buffer_size_ms = DEFAULT_BUFFER_SIZE_MS,
    //ChannelRange channel_range = ChannelRange( 1, MAX_N_CHANNELS_SPIKE_DETECTION )) :
    //buffer_size_ms_(buffer_size_ms), channel_range_(channel_range) {}
    
    //ChannelRange channel_range() const;
    
    //unsigned int n_channels() const;
    
    //bool CheckCompatibility( const SpikeDataType& upstream ) const;
    
    //double buffer_size() const;
    
    //double sample_rate() const;
    
    //virtual void Finalize( unsigned int nchannels, double sample_rate = DEFAULT_SAMPLING_FREQUENCY);
    
    //virtual void Finalize( SpikeDataType& upstream );
	
    //virtual void InitializeData( SpikeData& item ) const ;

    //virtual std::string name() const { return "spike"; }

//protected:
    //double buffer_size_ms_;
    //ChannelRange channel_range_;
    //double sample_rate_; // in Hz
    //unsigned int n_channels_;
    
//public:
    //static constexpr double DEFAULT_SAMPLING_FREQUENCY = 32000;
    
//};

#endif // spikedata.hpp
