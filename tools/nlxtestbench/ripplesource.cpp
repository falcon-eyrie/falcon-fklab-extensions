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

#include "ripplesource.hpp"
#include "utilities/string.hpp"

RippleSource::RippleSource(double offset, double amplitude, double frequency,
                           double duration, double interval,
                           double sampling_rate, double noise_stdev,
                           unsigned int nchannels, bool convert_byte_order)
    :

      offset_(offset), frequency_(frequency), duration_(duration),
      interval_(interval), sampling_rate_(sampling_rate),
      noise_stdev_(noise_stdev), delta_(1000000 / sampling_rate),
      distribution_(0.0, noise_stdev), ripple_(false),
      poisson_distribution_(amplitude), nchannels_(nchannels),
      convert_byte_order_(convert_byte_order) {

  if (frequency_ < 150 || frequency_ > 250) {
    throw std::runtime_error(
        "Invalid frequency for a SWRs - should be between 150 and 250 Hz");
  }

  duration_ = duration_ * sampling_rate_;
  interval_ = interval_ * sampling_rate_;
  amplitude_ = poisson_distribution_(generator_);
  counter_ = 0;
  omega_ = 2 * 3.14159265358979 * frequency_ / sampling_rate_;

  record_.set_nchannels(nchannels_);
  record_.set_convert_byte_order(convert_byte_order_);
}

std::string RippleSource::string() {

  return "ripple wave (fs = " + to_string_n(sampling_rate_) + " Hz, " +
         "offset = " + to_string_n(offset_) + " uV, " +
         "amplitude = " + to_string_n(amplitude_) + " uV, " +
         "ripple frequency = " + to_string_n(frequency_) + " Hz, " +
         "ripple duration = " + to_string_n(duration_ / sampling_rate_) +
         " secs, " +
         "zero signal duration = " + to_string_n(interval_ / sampling_rate_) +
         " secs, " + "noise stdev = " + to_string_n(noise_stdev_) + " uV, " +
         "number of channels = " + std::to_string(nchannels_) + ", " +
         "convert byte order = " + std::to_string(convert_byte_order_) + ")";
}

int64_t RippleSource::Produce(char **data) {

  if (!ripple_ and
      counter_ >
          interval_) { // starting a ripple at the end of the interval segment
    amplitude_ = poisson_distribution_(generator_);
    counter_ = -duration_ / 2;
    ripple_ = true;
  }

  ++counter_;
  if (ripple_ and counter_ < duration_ / 2) { // generating ripple values
    current_amplitude_ =
        amplitude_ * std::exp(-0.5 * std::pow(4.0 * counter_ / duration_, 2)) *
        std::cos(counter_ * omega_);
  } else { // interval segment = amplitude of the signal = 0
    current_amplitude_ = 0;

    if (ripple_) { // starting an interval segment at the end of the ripple
                   // signal

      counter_ = 0;
      ripple_ = false;
    }
  }

  // set ripple data + noise + offset
  record_.set_data(distribution_(generator_) + offset_ + current_amplitude_);
  record_.set_timestamp(timestamp_);
  timestamp_ = timestamp_ + delta_;

  auto n = record_.ToNetworkBuffer(buffer_);
  *data = buffer_.data();

  return n;
}

YAML::Node RippleSource::to_yaml() const {

  YAML::Node node;

  node["offset"] = offset_;
  node["amplitude"] = amplitude_;
  node["frequency"] = frequency_;
  node["duration"] = duration_;
  node["interval"] = interval_;
  node["sampling_rate"] = sampling_rate_;
  node["noise_stdev"] = noise_stdev_;
  node["nchannels"] = nchannels_;
  node["convert_byte_order"] = convert_byte_order_;

  return node;
}

RippleSource *RippleSource::from_yaml(const YAML::Node node) {

  return new RippleSource(
      node["offset"].as<double>(0.0), node["amplitude"].as<double>(100),
      node["frequency"].as<double>(200), node["duration"].as<double>(0.1),
      node["interval"].as<double>(0.5), node["sampling_rate"].as<double>(32000),
      node["noise_stdev"].as<double>(0),
      node["nchannels"].as<unsigned int>(128),
      node["convert_byte_order"].as<bool>(true));
}