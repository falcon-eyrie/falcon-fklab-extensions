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

#include <random>

#include "datasource.hpp"

class RippleSource : public DataSource {

public:
  /**
   * Create the source and set the user options
   *
   * @param offset purely value added to the sample generated
   * @param mean_amplitude mean of the amplitude of the ripple
   * @param frequency frequency of the ripple should be contain between 150 and
   * 250
   * @param ripple_duration Yaml Node containing duration of a ripple
   *        and zero interval value in msecs. The key specify for which channel
   * apply this set of parameters. If the key is *, apply it for all channel.
   * @param sampling_rate rate of generating and sending samples
   * @param noise_stdev standard deviation of the noise added to the signal
   * @param nchannels number of channels simulated
   * @params convert_byter_order
   */
  RippleSource(double offset, double frequency,double sampling_rate,
               YAML::Node ripple_duration, double mean_amplitude,
               double noise_stdev, bool convert_byte_order,
               unsigned int nchannels);

public:
  std::string string() override;

  /**
   * Produce sample and timestamps for each channel to package and send in the
   * network.
   *
   * Signal produced = ripple signal separated by segment of zero amplitude
   * signal
   *
   */
  int64_t Produce(char **data) override;
  YAML::Node to_yaml() const override;
  static RippleSource *from_yaml(YAML::Node node);

private:
  struct params_by_channel_t {
    double counter;
    double interval;
    double duration;
    bool ripple;
    double amplitude;
    unsigned int channel_number;
  };

  double generate_one_signal(params_by_channel_t *params);

protected:
  double offset_;
  double frequency_;
  double sampling_rate_;
  uint64_t delta_;
  uint64_t timestamp_ = 0;

  YAML::Node ripple_params_;

  double mean_amplitude_;
  std::poisson_distribution<int> poisson_distribution_;
  std::default_random_engine generator_;

  double noise_stdev_;
  std::normal_distribution<double> distribution_;

  bool convert_byte_order_;
  std::vector<char> buffer_;

  unsigned int nchannels_;
  std::vector<params_by_channel_t> params;
  double omega_;


};
