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

#include <iostream>
#include <memory>
#include <vector>

#include "datasource.hpp"
#include "options/options.hpp"
#include "utilities/configuration.hpp"

class TestBenchConfiguration : public Configuration {
  public:
    TestBenchConfiguration() : Configuration() {
        add_option("network/ip", ip_address,
                   "Network IP address to stream data packets to.");
        add_option("network/port", port,
                   "Network port to stream data packets to.");
        add_option("stream/rate", stream_rate, "Data stream rate in Hz");
        add_option("stream/npackets", npackets,
                   "The total number of data packets to stream. "
                   "A value of 0 means continuous streaming.");
        add_option(
            "stream/autostart", autostart,
            "The data source index that will be streamed immediately after "
            "start-up.");
        add_option("sources", sources, "A list of data source definitions.");
    }

  public:
    options::String ip_address{"127.0.0.1"};
    options::Int port{5000};
    options::Measurement<double> stream_rate{5000, "Hz",
                                             options::positive<double>(true)};
    options::Value<uint64_t, false> npackets{0};
    options::Int autostart{-1};
    options::Value<YAML::Node, true> sources{YAML::Load("class: default")};
};
