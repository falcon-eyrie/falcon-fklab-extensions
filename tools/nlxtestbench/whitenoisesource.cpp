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

#include "whitenoisesource.hpp"
#include "utilities/string.hpp"

WhiteNoiseSource::WhiteNoiseSource(double mean, double stdev,
                                   double sampling_rate, unsigned int nchannels,
                                   bool convert_byte_order)
    : mean_(mean), stdev_(stdev), sampling_rate_(sampling_rate),
      delta_(1000000 / sampling_rate), distribution_(mean, stdev),
      nchannels_(nchannels), convert_byte_order_(convert_byte_order) {
    record_.set_nchannels(nchannels_);
    record_.set_convert_byte_order(convert_byte_order_);
}

std::string WhiteNoiseSource::string() {
    return "gaussian white noise (fs = " + to_string_n(sampling_rate_) +
           " Hz, " + "mean = " + to_string_n(mean_) + " uV, " +
           "stdev = " + to_string_n(stdev_) + "uV, " +
           "nchannels = " + std::to_string(nchannels_) + ", " +
           "convert byte order = " + std::to_string(convert_byte_order_) + ")";
}

int64_t WhiteNoiseSource::Produce(char **data) {
    record_.set_data(distribution_(generator_));
    record_.set_timestamp(timestamp_);
    timestamp_ = timestamp_ + delta_;
    auto n = record_.ToNetworkBuffer(buffer_);
    *data = buffer_.data();
    return n;
}

YAML::Node WhiteNoiseSource::to_yaml() const {
    YAML::Node node;
    node["mean"] = mean_;
    node["stdev"] = stdev_;
    node["sampling_rate"] = sampling_rate_;
    node["nchannels"] = nchannels_;
    node["convert_byte_order"] = convert_byte_order_;
    return node;
}

WhiteNoiseSource *WhiteNoiseSource::from_yaml(const YAML::Node node) {
    return new WhiteNoiseSource(node["mean"].as<double>(0.0),
                                node["stdev"].as<double>(1.0),
                                node["sampling_rate"].as<double>(32000),
                                node["nchannels"].as<unsigned int>(128),
                                node["convert_byte_order"].as<bool>(true));
}
