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

#include "runningstats.hpp"

RunningStats::RunningStats() : IProcessor() {
  add_option("integration time", integration_time_,
             "Time window for exponential smoothing.");

  add_option("outlier/protection", outlier_protection_,
             "enable outlier protection. "
             "Outliers are values larger than a predefined z-score. "
             "The contribution of an outlier is reduced by an amount "
             "that depends on the magnitude of the outlier.");

  add_option("outlier/zscore", outlier_zscore_,
             "z-score that defines an outlier.");

  add_option("outlier/half life", outlier_half_life_,
             "The number of standard deviations above the outlier "
             "z-score at which the influence of the outlier is halved.");
}

void RunningStats::CreatePorts() {
  data_in_port_ = create_input_port<MultiChannelType<double>>(
      "data", MultiChannelType<double>::Capabilities(ChannelRange(1, 256)),
      PortInPolicy(SlotRange(1)));

  data_out_port_ = create_output_port<MultiChannelType<double>>(
      "data", MultiChannelType<double>::Capabilities(ChannelRange(1, 256)),
      MultiChannelType<double>::Parameters(), PortOutPolicy(SlotRange(1)));
}

void RunningStats::CompleteStreamInfo() {
  for (int k = 0; k < data_in_port_->number_of_slots(); ++k) {
    data_out_port_->streaminfo(k).set_parameters(
        data_in_port_->streaminfo(k).parameters());
    data_out_port_->streaminfo(k).set_stream_rate(data_in_port_->streaminfo(k));
  }
}

void RunningStats::Preprocess(ProcessingContext &context) {
  double sample_rate =
      data_in_port_->slot(0)->streaminfo().parameters().sample_rate;
  double alpha = 1.0 / (integration_time_() * sample_rate);

  stats_.reset(new dsp::algorithms::RunningMeanMAD(
      alpha, integration_time_() * sample_rate, outlier_protection_(),
      outlier_zscore_(), outlier_half_life_()));
}

void RunningStats::Process(ProcessingContext &context) {
  MultiChannelType<double>::Data *data_in;
  MultiChannelType<double>::Data *data_out;
  unsigned N = 100;

  while (!context.terminated()) {
    // retrieve new data
    if (!data_in_port_->slot(0)->RetrieveData(data_in)) {
      break;
    }

    data_out = data_out_port_->slot(0)->ClaimData(false);
    // loop through each sample
    for (unsigned int sample = 0; sample < data_in->nsamples(); ++sample) {
      stats_->add_sample(data_in->data_sample(sample, 0));
      data_out->set_data_sample(sample, 0, stats_->center());
      data_out->set_data_sample(sample, 1, stats_->dispersion());
      data_out->set_sample_timestamp(sample, data_in->sample_timestamp(sample));
    }

    data_out->CloneTimestamps(*data_in);
    data_out_port_->slot(0)->PublishData();
    data_in_port_->slot(0)->ReleaseData();

    N--;
    if (N == 0) {
      LOG(UPDATE) << "center = " << stats_->center()
                  << ", dispersion = " << stats_->dispersion();
      N = 100;
    }
  }
}

REGISTERPROCESSOR(RunningStats)
