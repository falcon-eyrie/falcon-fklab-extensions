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

#include <cstdint>
#include <string>
#include <vector>

// to be used for port names using spike data
const std::string SPIKEDATA = "spikes";

static const double DEFAULT_BUFFER_SIZE = 12.75;
static const unsigned int MAX_N_CHANNELS_SPIKE_DETECTION = 16;
static const unsigned int MAX_N_SPIKES_IN_BUFFER = 100;
static std::vector<uint64_t> zero_timestamps(MAX_N_SPIKES_IN_BUFFER, 0);
static std::vector<double>
zero_amplitudes(MAX_N_SPIKES_IN_BUFFER *MAX_N_CHANNELS_SPIKE_DETECTION, 0);
