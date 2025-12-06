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

#pragma once

#include <string>
#include <vector>

#include "channelvalidity.hpp"
#include "idata.hpp"
#include "spikedata_common.hpp"
#include "utilities/general.hpp"

namespace nsSpikeType {

using ParentType = AnyType;

struct Parameters {
    Parameters(double bufsize = 0., unsigned int nchan = 0, double rate = 0.)
        : buffer_size(bufsize), nchannels(nchan), sample_rate(rate) {}

    double buffer_size;
    unsigned int nchannels;
    double sample_rate;
};

class Data : public IData<Data, ParentType> {
   public:
    using BaseClass = IData<Data, ParentType>;

    Data(unsigned int nchannels, double buffer_size, double sample_rate, size_t max_nspikes = 0);

    Data(const Parameters& parameters)
        : Data(parameters.nchannels, parameters.buffer_size, parameters.sample_rate) {}

    static const std::string static_datatype() { return "spike"; }
    static const std::string static_dataname() { return "spikes"; }

    Parameters parameters() const { return Parameters(buffer_size_, n_channels_, sample_rate_); }

    void ClearData() override;

    unsigned int n_channels() const;

    double sample_rate() const;
    double buffer_size() const { return buffer_size_; }

    void add_spike(const std::vector<double>& amplitudes,
                   uint64_t hw_timestamp);  // 1st argument will change to a
                                            // better interface for matrices

    void add_spike(double* amplitudes, uint64_t hw_timestamp);

    unsigned int n_detected_spikes() const;

    std::vector<double>& amplitudes();

    ChannelValidityMask& validity_mask();

    const std::vector<uint64_t>& ts_detected_spikes() const;

    const uint64_t ts_detected_spikes(int index) const;

    std::vector<double>::const_iterator spike_amplitudes(std::size_t spike_index) const;

    void SerializeBinary(std::ostream& stream,
                         Serialization::Format format = Serialization::Format::FULL) const final;

    void SerializeYAML(YAML::Node& node,
                       Serialization::Format format = Serialization::Format::FULL) const final;

    void YAMLDescription(YAML::Node& node,
                         Serialization::Format format = Serialization::Format::FULL) const final;

    void SerializeFlatBuffer(flexbuffers::Builder& flex_builder) final;

   protected:
    uint8_t n_channels_;
    unsigned int n_detected_spikes_;
    std::vector<double> amplitudes_;
    // std::vector<double> widths_;
    std::vector<uint64_t> hw_ts_detected_spikes_;
    double buffer_size_;
    double sample_rate_;
    ChannelValidityMask validity_mask_;
    ChannelValidityMask default_validity_mask_;  // independent of spike detection outcome

   public:
    static constexpr unsigned int DEFAULT_MAX_NSPIKES =
        MAX_N_SPIKES_IN_BUFFER;  // max expected # of spikes in a buffer

   protected:
    // for serialization
    const std::string N_CHANNELS = "n_channels";
    const std::string N_DETECTED_SPIKES = "n_detected_spikes";
    const std::string TS_DETECTED_SPIKES = "TS_detected_spikes";
    const std::string SPIKE_AMPLITUDES = "spike_amplitudes";
};

class Capabilities {
   public:
    Capabilities(ChannelRange channel_range = ChannelRange(1, MAX_N_CHANNELS_SPIKE_DETECTION))
        : channel_range_(channel_range) {}

    ChannelRange channel_range() const { return channel_range_; }

    void Validate(const Data& prototype) const {
        if (!channel_range_.inrange(prototype.n_channels())) {
            throw std::runtime_error("Number of channels cannot be zero and needs to be in range " +
                                     channel_range_.to_string());
        }
    }

   protected:
    ChannelRange channel_range_;
};

}  // namespace nsSpikeType

using SpikeType = DefineType<nsSpikeType::Data, AnyType, true, nsSpikeType::Capabilities,
                             nsSpikeType::Parameters>;
