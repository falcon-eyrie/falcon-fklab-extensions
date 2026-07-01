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

#include "multichannelfilter.hpp"

#include <chrono>
#include <exception>
#include <string>
#include <thread>

MultiChannelFilter::MultiChannelFilter() : IProcessor() {
    add_option("filter_file", filter_file_path_, "Filter definition.", true);
}

void MultiChannelFilter::Configure(const GlobalContext& context) {
    // Direct filter coefficients definitions via graph YAML is disabled
    // for the moment. Will enable it again if needed in the future.
    // if (!filter_def_()["file"]) {
    //     filter_template_.reset(dsp::filter::construct_from_yaml(filter_def_()));
    // } else {
    std::string filter_file_path = context.resolve_path(filter_file_path_(), "filters");
    filter_template_.reset(dsp::filter::construct_from_file(filter_file_path));
}

void MultiChannelFilter::CreatePorts() {
    in_port_ = create_input_port<TimeSeriesType<double>>(
        "input", TimeSeriesType<double>::Capabilities(ChannelRange(1, 256)),
        PortInPolicy(SlotRange(1, 1)));

    out_port_ = create_output_port<TimeSeriesType<double>>(
        "output", TimeSeriesType<double>::Parameters(), PortOutPolicy());
}

void MultiChannelFilter::CompleteStreamInfo() {
    for (int k = 0; k < in_port_->number_of_slots(); ++k) {
        out_port_->streaminfo(k).set_stream_parameters(in_port_->streaminfo(k));

        out_port_->streaminfo(k).set_parameters(in_port_->prototype(k).parameters());
    }
}

void MultiChannelFilter::Prepare(GlobalContext& _) {
    auto upstream_data = in_port_->prototype(0);

    filter_ = std::unique_ptr<dsp::filter::IFilter>(filter_template_->clone());
    filter_->realize(upstream_data.ncolumns());

    auto input_buffer_size = upstream_data.ncolumns() * upstream_data.nsamples();
    filtered_signal_buffer_.resize(input_buffer_size);
}

void MultiChannelFilter::Process(ProcessingContext& context) {
    TimeSeriesType<double>::Data* data_in = nullptr;
    ;
    auto n_out_slots = out_port_->number_of_slots();

    while (!context.terminated()) {
        uint64_t sync_start = __rdtsc();

        bool has_data = in_port_->slot(0)->RetrieveData(data_in);

        if (!has_data) break;

        uint64_t sync_end = __rdtsc();

        uint64_t work_start = __rdtsc();

        filter_->process_by_channel(data_in->nsamples(), data_in->data(), filtered_signal_buffer_);

        for (unsigned k = 0; k < n_out_slots; k++) {
            auto data_out = out_port_->slot(k)->ClaimData(false);
            data_out->data() = filtered_signal_buffer_;

            data_out->set_sample_timestamps(data_in->sample_timestamps());
            data_out->CloneTimestamps(*data_in);
            data_out->forward_ingestion_tsc(*data_in);
            out_port_->slot(k)->PublishData();
        }

        in_port_->slot(0)->ReleaseData();

        uint64_t work_end = __rdtsc();

        record_metrics(sync_end - sync_start, work_end - work_start);
    }
}

void MultiChannelFilter::Postprocess(ProcessingContext& _) {
    dump_benchmarks();
}

REGISTERPROCESSOR(MultiChannelFilter)
