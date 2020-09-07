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
#include <string>
#include <vector>

#include "datasource.hpp"

class SquareSource : public DataSource {
public:
  SquareSource(double offset = 0.0, double amplitude = 1.0,
               double frequency = 1.0, double duty_cycle = 0.5,
               double sampling_rate = 1.0, double noise_stdev = 0,
               unsigned int nchannels = 128, bool convert_byte_order = true);

  std::string string() override;
  int64_t Produce(char **data) override;
  YAML::Node to_yaml() const override;

  static SquareSource *from_yaml(YAML::Node node);

protected:
  double offset_;
  double amplitude_;
  double frequency_;
  double duty_cycle_;
  double sampling_rate_;
  double noise_stdev_;

  uint64_t timestamp_ = 0;
  uint64_t delta_;

  uint64_t counter_;
  double current_amplitude_;

  std::vector<char> buffer_;

  std::default_random_engine generator_;
  std::normal_distribution<double> distribution_;

  unsigned int nchannels_;
  bool convert_byte_order_;
};
