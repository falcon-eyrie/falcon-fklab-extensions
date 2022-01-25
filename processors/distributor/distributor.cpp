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

#include "distributor.hpp"
#include "utilities/general.hpp"

Distributor::Distributor() : IProcessor(PRIORITY_MEDIUM) {
    add_option("channelmap", channelmap_,
               "Mapping of columns in different datastreams.");

    add_option("channelmap file", channelmap_file_,
               "Mapping of columns in different datastreams.");
    add_option("port distribution", distribution_type_,
               "Distribution over the ports or over the slots");

}

void Distributor::Configure(const GlobalContext &context){

    if(channelmap_file_() != ""){
        auto path = context.resolve_path(channelmap_file_());
        YAML::Node config = YAML::LoadFile(path);
        try{
            channelmap_.from_yaml(config);
        }catch (...){
            throw std::runtime_error(". The channelmap is not valid.");
        }
    }
}

void Distributor::CreatePorts() {
    input_port_ = create_input_port<TimeSeriesType<double>>(
                  TimeSeriesType<double>::Capabilities(ChannelRange(1, MAX_N_CHANNELS)),
                  PortInPolicy(SlotRange(1)));

    if(distribution_type_()){   // N port with 1 slot each (N = channelmap size)
        for (auto &it : channelmap_()) {
            data_ports_[it.first] = create_output_port<TimeSeriesType<double>>(
                                    it.first,
                                    TimeSeriesType<double>::Parameters(),
                                    PortOutPolicy(SlotRange(1), BUFFER_SIZE, WAIT_STRATEGY));
        }
    }else{  // 1 port with N slots (N = channelmap size)
        data_ports_["data"] = create_output_port<TimeSeriesType<double>>(
                                     TimeSeriesType<double>::Parameters(),
                                     PortOutPolicy(SlotRange(channelmap_().size()), BUFFER_SIZE, WAIT_STRATEGY));
    }
}

void Distributor::CompleteStreamInfo() {
    auto incoming_batch_size = input_port_->prototype(0).nsamples();

    LOG(INFO) << name() << ". Incoming batch size: " << incoming_batch_size << ".";

    slot_ = 0;
    for (auto &it : channelmap_()) {
        if(distribution_type_()){   // N port with 1 slot each (N = channelmap size)

            data_ports_[it.first]->streaminfo(0).set_parameters(
                        TimeSeriesType<double>::Parameters(
                            it.second.get_labels(),
                            incoming_batch_size,
                            input_port_->prototype(0).sample_rate()));

             data_ports_[it.first]->streaminfo(0).set_stream_parameters(input_port_->streaminfo(0));


        }else{  // 1 port with N slots (N = channelmap size)
            data_ports_["data"]->streaminfo(slot_).set_parameters(
                        TimeSeriesType<double>::Parameters(
                            it.second.get_labels(),
                            incoming_batch_size,
                            input_port_->prototype(0).sample_rate()));

             data_ports_["data"]->streaminfo(slot_).set_stream_parameters(
                        input_port_->streaminfo(0).stream_rate(), it.first);

             slot_++;
        }
    }
}

void Distributor::Prepare(GlobalContext &context) {
    // check channel map
    for (auto const &it : channelmap_()) {
        if (it.second.size() == 0) {
            throw ProcessingPrepareError(
                        "Channel map entry " + it.first + " has zero channels.", name());
        }

        if(!it.second.is_subset(input_port_->prototype(0).labels())){
            throw ProcessingPrepareError(
                        "Channel list " + it.first + ": " + it.second.to_string() + " is invalid",
                        name());
        }

        if (!it.second.is_unique()){
            throw ProcessingPrepareError(
                        "Channel list " + it.first + ": " + it.second.to_string() + " cannot have duplicate channels",
                        name());
        }
    }
}

void Distributor::Process(ProcessingContext &context) {
    TimeSeriesType<double>::Data *data_in = nullptr;
    unsigned int s = 0;
    std::vector<TimeSeriesType<double>::Data *> data_out_vector;


    while (!context.terminated()) {
        // retrieve new data packet
        if (!input_port_->slot(0)->RetrieveData(data_in)) {
            break;
        }

        for (auto &it : data_ports_) {
            for (slot_ = 0; slot_ < it.second->number_of_slots(); slot_++) {
                data_out_vector.push_back(it.second->slot(slot_)->ClaimData(false));
            }
        }

        for (auto &data_out : data_out_vector) {
            data_out->CloneTimestamps(*data_in);
            data_out->set_sample_timestamps(
                        data_in->sample_timestamps());


            // Note for later = would be nice to implement a helper to copy in once
            // whole column from one packet to the other
            for (auto ch: data_out->labels()) {
                // publish data buckets
                for (s = 0; s < data_in->nsamples(); s++) {
                     data_out->set_data_sample(s, ch,  data_in->data_sample(s, ch));
                }
            }
        }

        // publish data buckets
        for (auto &it : data_ports_) {
            for (slot_ = 0; slot_ < it.second->number_of_slots(); slot_++) {
                it.second->slot(slot_)->PublishData();
            }
        }

        // release input data bucket
        input_port_->slot(0)->ReleaseData();
        data_ports_.clear();
    }
}

void Distributor::Postprocess(ProcessingContext &context) {
    for (auto &it : data_ports_) {
        LOG(INFO) << name() << ". Port " << it.first
                      << ". Streamed " << it.second->slot(0)->nitems_produced()
                      << " data packets. ";

    }
}

REGISTERPROCESSOR(Distributor)
