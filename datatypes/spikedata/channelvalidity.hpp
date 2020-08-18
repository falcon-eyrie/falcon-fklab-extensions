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

#include <vector>

#include "spikedata_common.hpp"

struct ChannelDetection {
  enum Validity { UNKNOWN, VALID, BROKEN, NO_PEAK };
};

class ChannelValidityMask {
 public:
  ChannelValidityMask(unsigned int n_channels = MAX_N_CHANNELS_SPIKE_DETECTION,
                      ChannelDetection::Validity validity =
                          ChannelDetection::Validity::UNKNOWN);

  ~ChannelValidityMask() {}

  unsigned int n_channels() const;

  std::vector<ChannelDetection::Validity> &validity_mask();

  void set_validity(size_t channel_index, ChannelDetection::Validity value);

  bool is_channel_valid(size_t channel_index) const;

  bool all_channels_valid() const;

  void reset(ChannelDetection::Validity value =
             ChannelDetection::Validity::UNKNOWN);

 protected:
  std::vector<ChannelDetection::Validity> mask_;
};
