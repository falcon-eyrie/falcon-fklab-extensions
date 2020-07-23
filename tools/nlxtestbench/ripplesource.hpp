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

#include "common.hpp"
#include "datasource.hpp"


class RippleSource : public DataSource {
public:

    /**
     * Create the source and set the user options
     *
     * @param offset purely value added to the sample generated
     * @param amplitude mean of the amplitude of the ripple
     * @param frequency frequency of the ripple should be contain between 150 and 250
     * @param duration duration of the ripple segment in secs
     * @param interval duration of the zero amplitude segment in secs
     * @param sampling_rate rate of generating and sending samples
     * @param noise_stdev standard deviation of the noise added to the signal
     * @param nchannels number of channels simulated
     * @params convert_byter_order
     */
    RippleSource( double offset = 0.0, double amplitude = 1.0, double frequency = 200,
        double duration=10, double interval = 10, double sampling_rate = 32000, double noise_stdev = 0,
        unsigned int nchannels=128, bool convert_byte_order=true );

    virtual std::string string();

    /**
     * Produce sample and timestamps to package and send in the network.
     *
     * Signal produced = ripple signal separated by segment of zero amplitude signal
     *
     */
    virtual int64_t Produce( char** data );

    virtual YAML::Node to_yaml() const;

    static RippleSource* from_yaml( const YAML::Node node );

protected:
    double offset_;
    double amplitude_;
    double duration_;
    double interval_;
    double frequency_;
    double probability_;
    double sampling_rate_;
    double noise_stdev_;

    uint64_t timestamp_ = 0;
    uint64_t delta_;

    uint64_t counter_;
    double current_amplitude_;
    double dist_amplitude_;
    double omega_;
    bool ripple_;
    nlx::NlxSignalRecord record_;

    std::vector<char> buffer_;

    std::default_random_engine generator_;
    std::normal_distribution<double> distribution_;

    std::poisson_distribution<int> poisson_distribution_;
    unsigned int nchannels_;
    bool convert_byte_order_;
};

#endif // RIPPLESOURCE_H
