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

#ifndef RIPPLESOURCE_H
#define RIPPLESOURCE_H

#include <random>
#include "datasource.hpp"


class RippleSource : public DataSource {
private:
  struct params_by_channel_t {
    double counter;
    double interval;
    double duration;
    bool ripple;
    double amplitude;
    unsigned int channel_number;
  } paramsByChannel;

public:
  /**
   * Create the source and set the user options
   *
   * @param offset purely value added to the sample generated
   * @param amplitude mean of the amplitude of the ripple
   * @param frequency frequency of the ripple should be contain between 150 and
   * 250
   * @param duration duration of the ripple segment in secs
   * @param interval duration of the zero amplitude segment in secs
   * @param sampling_rate rate of generating and sending samples
   * @param noise_stdev standard deviation of the noise added to the signal
   * @param nchannels number of channels simulated
   * @params convert_byter_order
   */
  RippleSource(double offset, double mean_amplitude, double frequency,
               YAML::Node ripple_duration,
               double sampling_rate, double noise_stdev,
               unsigned int nchannels, bool convert_byte_order);

  std::string string() override;

  /**
   * Produce sample and timestamps to package and send in the network.
   *
   * Signal produced = ripple signal separated by segment of zero amplitude
   * signal
   *
   */
  int64_t Produce(char **data) override;

  YAML::Node to_yaml() const override;

  static RippleSource *from_yaml(YAML::Node node);
  double generate_one_signal(params_by_channel_t* params);




protected:
  double offset_;
  double frequency_;
  double sampling_rate_;
  YAML::Node ripple_params_;
  double noise_stdev_;
  uint64_t delta_;
  std::normal_distribution<double> distribution_;
  std::poisson_distribution<int> poisson_distribution_;
  double mean_amplitude_;
  unsigned int nchannels_;
  bool convert_byte_order_;

  uint64_t timestamp_ = 0;
  std::vector<params_by_channel_t> params;
  double omega_;

  std::vector<char> buffer_;

  std::default_random_engine generator_;
};

#endif // RIPPLESOURCE_H
