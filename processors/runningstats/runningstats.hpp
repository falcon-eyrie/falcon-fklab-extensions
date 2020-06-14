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

/* RunningStats: computes running statistics
 * 
 * input ports:
 * data <MultiChannelType> (1 slot)
 *
 * output ports:
 * data <MultiChannelType> (1 slot)
 *
 * exposed states:
 * none
 *
 * exposed methods:
 * none
 *
 * options:
 * integration_time <double> - time window for exponential smoothing
 * outlier_protection <bool> - enable outlier protectection. Outliers are
 *   values larger than a predefined z-score. The contribution of an
 *   outlier is reduced by an amount that depends on the magnitude of
 *   the outlier.
 * outlier_zscore <double> - z-score that defines an outlier
 * outlier_half_life <double> - the number of standard deviations above
 *   the outlier z-score at which the influence of the outlier is halved.
 * 
 */
 
#ifndef RUNNINGSTATS_H
#define RUNNINGSTATS_H

#include "iprocessor.hpp"

#include <memory>

#include "multichanneldata/multichanneldata.hpp"
#include "dsp/algorithms.hpp"

#include "options/options.hpp"
#include "options/units.hpp"

class RunningStats : public IProcessor
{
// CONSTRUCTOR and OVERLOADED METHODS
public:
    RunningStats();
    
    virtual void CreatePorts( ) override;
    virtual void CompleteStreamInfo( ) override;
    virtual void Preprocess( ProcessingContext& context ) override;
    virtual void Process( ProcessingContext& context ) override;

// DATA PORTS
protected:
  
    PortIn<MultiChannelType<double>>* data_in_port_;
    PortOut<MultiChannelType<double>>* data_out_port_;

// OPTIONS
protected:

    options::Measurement<double,false> integration_time_{
        1.,
        "second",
        options::positive<double>(true)
    };

    options::Bool outlier_protection_{false};
    options::Double outlier_zscore_{6.0,};
    options::Double outlier_half_life_{2.0};

// OTHER
protected:
    std::unique_ptr<dsp::algorithms::RunningMeanMAD> stats_;

};

#endif
