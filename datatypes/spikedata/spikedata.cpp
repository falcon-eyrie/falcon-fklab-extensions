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

#include "spikedata.hpp"

#include <typeinfo>
#include <vector>

#include "channelvalidity.hpp"
#include "utilities/string.hpp"
#include "vector_operations/vector_io.hpp"

using namespace nsSpikeType;

void Data::Initialize(unsigned int nchannels, size_t max_nspikes,
                      double sample_rate) {
  n_channels_ = nchannels;
  n_detected_spikes_ = 0;
  sample_rate_ = sample_rate;

  // overestimates the maximum number of spike features in a buffer and
  // reserve enough space so that no memory allocation will take place during
  // run
  amplitudes_.reserve(nchannels * max_nspikes);
  hw_ts_detected_spikes_.reserve(max_nspikes);

  validity_mask_ = ChannelValidityMask(nchannels);
}

unsigned int Data::n_channels() const { return n_channels_; }

double Data::sample_rate() const { return sample_rate_; }

void Data::add_spike(const std::vector<double> &amplitudes,
                     uint64_t hw_timestamp) {
  assert(amplitudes.size() == n_channels_);
  for (unsigned int i = 0; i < n_channels_; ++i) {
    amplitudes_.push_back(amplitudes[i]);
  }
  ++n_detected_spikes_;
  hw_ts_detected_spikes_.push_back(hw_timestamp);
}

void Data::add_spike(double *amplitudes, uint64_t hw_timestamp) {
  for (unsigned int i = 0; i < n_channels_; ++i) {
    amplitudes_.push_back(amplitudes[i]);
  }
  ++n_detected_spikes_;
  hw_ts_detected_spikes_.push_back(hw_timestamp);
}

unsigned int Data::n_detected_spikes() const { return n_detected_spikes_; }

std::vector<double> &Data::amplitudes() {
  assert(n_detected_spikes_ == amplitudes_.size() / n_channels_);
  return amplitudes_;
}

void Data::ClearData() {
  n_detected_spikes_ = 0;
  amplitudes_.clear();
  hw_ts_detected_spikes_.clear();
  validity_mask_.reset();
}

ChannelValidityMask &Data::validity_mask() { return validity_mask_; }

const std::vector<uint64_t> &Data::ts_detected_spikes() const {
  return hw_ts_detected_spikes_;
}

const uint64_t Data::ts_detected_spikes(int index) const {
  assert(n_detected_spikes_ == ts_detected_spikes().size());
  assert(index < (static_cast<int>(n_detected_spikes_)));
  return hw_ts_detected_spikes_[index];
}

std::vector<double>::const_iterator
Data::spike_amplitudes(std::size_t spike_index) const {
  std::vector<double>::const_iterator it = amplitudes_.begin();
  it += spike_index * n_channels_;
  return it;
}

void Data::SerializeBinary(std::ostream &stream,
                           Serialization::Format format) const {
  Base::Data::SerializeBinary(stream, format);

  if (format == Serialization::Format::FULL) {
    int n_spikes_to_fill_buffer = MAX_N_SPIKES_IN_BUFFER - n_detected_spikes_;
    assert(n_spikes_to_fill_buffer >= 0);

    stream.write(reinterpret_cast<const char *>(&n_detected_spikes_),
                 sizeof(decltype(n_detected_spikes_)));
    stream.write(reinterpret_cast<const char *>(hw_ts_detected_spikes_.data()),
                 n_detected_spikes_ *
                     sizeof(decltype(hw_ts_detected_spikes_[0])));
    stream.write(reinterpret_cast<const char *>(zero_timestamps.data()),
                 n_spikes_to_fill_buffer *
                     sizeof(decltype(zero_timestamps[0])));
    stream.write(reinterpret_cast<const char *>(amplitudes_.data()),
                 n_detected_spikes_ * n_channels_ *
                     sizeof(decltype(amplitudes_[0])));
    stream.write(reinterpret_cast<const char *>(zero_amplitudes.data()),
                 n_spikes_to_fill_buffer * n_channels_ *
                     sizeof(decltype(zero_amplitudes[0])));
  }

  if (format == Serialization::Format::COMPACT) {
    for (decltype(n_detected_spikes_) sp = 0; sp < n_detected_spikes_; ++sp) {
      stream.write(reinterpret_cast<const char *>(&hw_ts_detected_spikes_[sp]),
                   sizeof(decltype(hw_ts_detected_spikes_[0])));
      stream.write(
          reinterpret_cast<const char *>(&amplitudes_[sp * n_channels_]),
          sizeof(decltype(amplitudes_[0])) * n_channels_);
    }
  }
}

void Data::SerializeYAML(YAML::Node &node, Serialization::Format format) const {
  Base::Data::SerializeYAML(node, format);

  if (format == Serialization::Format::FULL ||
      format == Serialization::Format::COMPACT) {
    node[N_CHANNELS] =
        static_cast<unsigned int>(n_channels_);  // TODO: move to preamble
    node[N_DETECTED_SPIKES] = static_cast<unsigned int>(n_detected_spikes_);
    if (n_detected_spikes_ > 0) {
      node[TS_DETECTED_SPIKES] = hw_ts_detected_spikes_;
      node[SPIKE_AMPLITUDES] = amplitudes_;
    }
  }
}

void Data::YAMLDescription(YAML::Node &node,
                           Serialization::Format format) const {
  Base::Data::YAMLDescription(node, format);

  if (format == Serialization::Format::FULL) {
    node.push_back(N_DETECTED_SPIKES + " " +
                   get_type_string<decltype(n_detected_spikes_)>() + " (1)");
    node.push_back(TS_DETECTED_SPIKES + " " + get_type_string<uint64_t>() +
                   " (" + std::to_string(MAX_N_SPIKES_IN_BUFFER) + ")");
    node.push_back(SPIKE_AMPLITUDES + " " + get_type_string<double>() + " (" +
                   std::to_string(MAX_N_SPIKES_IN_BUFFER) + "," +
                   std::to_string(n_channels_) + ")");
  }

  if (format == Serialization::Format::COMPACT) {
    node.push_back(TS_DETECTED_SPIKES + " " + get_type_string<uint64_t>() +
                   " (1)");
    node.push_back(SPIKE_AMPLITUDES + " " + get_type_string<double>() + " (" +
                   std::to_string(n_channels_) + ")");
  }
}

void Data::SerializeFlatBuffer(flexbuffers::Builder& flex_builder){
    Base::Data::SerializeFlatBuffer(flex_builder);

    flex_builder.TypedVector(SPIKE_AMPLITUDES.c_str(), [&]{
           for(auto samples: amplitudes_)
               flex_builder.Add(samples);
    });

    flex_builder.TypedVector(TS_DETECTED_SPIKES.c_str(), [&]{
           for(auto samples:  hw_ts_detected_spikes_)
               flex_builder.Add(samples);
    });

    flex_builder.UInt(N_DETECTED_SPIKES.c_str(), n_detected_spikes_);
    flex_builder.String("type", SpikeType::datatype());
}
