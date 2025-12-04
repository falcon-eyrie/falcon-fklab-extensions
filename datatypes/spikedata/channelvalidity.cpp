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

#include "channelvalidity.hpp"
#include <vector>

ChannelValidityMask::ChannelValidityMask(unsigned int n_channels,
                                         ChannelDetection::Validity validity) {
    mask_.assign(n_channels, validity);
}

unsigned int ChannelValidityMask::n_channels() const { return mask_.size(); }

std::vector<ChannelDetection::Validity> &ChannelValidityMask::validity_mask() {
    return mask_;
}

void ChannelValidityMask::set_validity(size_t channel_index,
                                       ChannelDetection::Validity value) {
    mask_[channel_index] = value;
}

bool ChannelValidityMask::is_channel_valid(size_t channel_index) const {
    return mask_[channel_index] == ChannelDetection::Validity::VALID;
}

bool ChannelValidityMask::all_channels_valid() const {
    for (auto m : mask_) {
        if (m != ChannelDetection::Validity::VALID) {
            return false;
        }
    }
    return true;
}

void ChannelValidityMask::reset(ChannelDetection::Validity validity_value) {
    mask_.assign(this->n_channels(), validity_value);
}
