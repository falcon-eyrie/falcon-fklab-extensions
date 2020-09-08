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
#include <iostream>

RippleSource::RippleSource(double offset, double frequency,double sampling_rate,
                           YAML::Node ripple_duration, double mean_amplitude,
                           double noise_stdev, bool convert_byte_order,
                           unsigned int nchannels)
    : offset_(offset), frequency_(frequency), sampling_rate_(sampling_rate),
      delta_(1000000 / sampling_rate), ripple_params_(ripple_duration),
      mean_amplitude_(mean_amplitude), poisson_distribution_(mean_amplitude),
      noise_stdev_(noise_stdev), distribution_(0.0, noise_stdev),
      convert_byte_order_(convert_byte_order), nchannels_(nchannels) {

  if (frequency_ < 150 || frequency_ > 250) {
    throw std::runtime_error(
        "Invalid frequency for a SWRs - should be between 150 and 250 Hz");
  }

  if (!ripple_params_ || !ripple_params_["*"]) {
    if (ripple_params_["duration"]) {
      ripple_params_["*"]["duration"] = ripple_params_["duration"];
      ripple_params_.remove("duration");
    } else {
      ripple_params_["*"]["duration"] = 100;
    }
    if (ripple_params_["interval"]) {
      ripple_params_["*"]["interval"] = ripple_params_["interval"];
      ripple_params_.remove("interval");
    } else {
      ripple_params_["*"]["interval"] = 50;
    }
  }

  for (unsigned int i = 0; i < nchannels; i++) {
    params_by_channel_t param{};
    param.counter = 0;

    if (ripple_duration[i]) {
      param.interval =
          ripple_duration[i]["interval"].as<double>() * sampling_rate_ / 1000;
      param.duration =
          ripple_duration[i]["duration"].as<double>() * sampling_rate_ / 1000;
    } else {
      param.interval =
          ripple_duration["*"]["interval"].as<double>() * sampling_rate_ / 1000;
      param.duration =
          ripple_duration["*"]["duration"].as<double>() * sampling_rate_ / 1000;
    }

    param.amplitude = poisson_distribution_(generator_);
    if (param.counter >= param.interval) {
      param.ripple = true;
    } else {
      param.ripple = false;
    }

    param.channel_number = i;
    params.push_back(param);
  }

  omega_ = 2 * 3.14159265358979 * frequency_ / sampling_rate_;

  record_.set_nchannels(nchannels_);
  record_.set_convert_byte_order(convert_byte_order_);
}

std::string RippleSource::string() {

  std::string ripples;
  std::string general_ripples;
  for (auto it = ripple_params_.begin(); it != ripple_params_.end(); ++it) {
    auto key = it->first;
    auto value = it->second;

    if (key.as<std::string>() == "*") {
      general_ripples += "\nFor all channels : ";
      general_ripples +=
          "mean ripple amplitude = " + to_string_n(mean_amplitude_) + " uV, " +
          "ripple frequency = " + to_string_n(frequency_) + " Hz, " +
          "ripple duration = " + value["duration"].as<std::string>() + " ms, " +
          "zero signal interval = " + value["interval"].as<std::string>() +
          " ms ";
    } else {
      ripples += "\nExcept for channel " + key.as<std::string>() + " : ";
      ripples +=
          "ripple duration = " + value["duration"].as<std::string>() + " ms, " +
          "zero signal interval = " + value["interval"].as<std::string>() +
          " ms ";
    }
  }

  return "ripple wave (fs = " + to_string_n(sampling_rate_) + " Hz, " +
         "offset = " + to_string_n(offset_) + " uV, " +
         "noise stdev = " + to_string_n(noise_stdev_) + " uV, " +
         "number of channels = " + std::to_string(nchannels_) + ", " +
         "convert byte order = " + std::to_string(convert_byte_order_) + ", " +

         general_ripples + ripples + ")";
}

int64_t RippleSource::Produce(char **data) {

  std::vector<double> generate_data;
  for (auto it = begin(params); it != end(params); ++it) {
    double dt = generate_one_signal(&*it);
    generate_data.push_back(dt);
  }

  // set ripple data + noise + offset
  record_.set_data(generate_data);
  record_.set_timestamp(timestamp_);
  timestamp_ = timestamp_ + delta_;

  auto n = record_.ToNetworkBuffer(buffer_);
  *data = buffer_.data();

  return n;
}

double RippleSource::generate_one_signal(params_by_channel_t *pparams) {
  double current_amplitude;

  if (!pparams->ripple and pparams->counter > pparams->interval) {
    // starting a ripple at the end of the interval segment
    pparams->amplitude = poisson_distribution_(generator_);
    pparams->counter = -pparams->duration / 2;
    pparams->ripple = true;
  }

  ++pparams->counter;
  if (pparams->ripple and
      pparams->counter < pparams->duration / 2) { // generating ripple values
    current_amplitude =
        pparams->amplitude *
        std::exp(-0.5 *
                 std::pow(4.0 * pparams->counter / pparams->duration, 2)) *
        std::cos(pparams->counter * omega_);
  } else { // interval segment = amplitude of the signal = 0
    current_amplitude = 0;

    if (pparams->ripple) {
      // starting an interval segment at the end of the ripple signal
      pparams->counter = 0;
      pparams->ripple = false;
    }
  }

  return (distribution_(generator_) + offset_ + current_amplitude);
}

YAML::Node RippleSource::to_yaml() const {

  YAML::Node node;

  node["offset"] = offset_;
  node["mean ripple amplitude"] = mean_amplitude_;
  node["frequency"] = frequency_;
  node["ripple param"] = ripple_params_;
  node["sampling_rate"] = sampling_rate_;
  node["noise_stdev"] = noise_stdev_;
  node["nchannels"] = nchannels_;
  node["convert_byte_order"] = convert_byte_order_;

  return node;
}

RippleSource *RippleSource::from_yaml(const YAML::Node node) {

  YAML::Node default_ripple_params;
  default_ripple_params['*']["interval"] = 50;
  default_ripple_params['*']["duration"] = 100;

  return new RippleSource(
      node["offset"].as<double>(0.0),
      node["ripple frequency"].as<double>(200),
      node["sampling_rate"].as<double>(32000),
      node["ripple param"].as<YAML::Node>(default_ripple_params),
      node["mean ripple amplitude"].as<double>(100),
      node["noise_stdev"].as<double>(0),
      node["convert_byte_order"].as<bool>(true),
      node["nchannels"].as<unsigned int>(128));

}