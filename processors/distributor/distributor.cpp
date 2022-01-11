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
               "Mapping of columns in different datastreams.", true);

    add_option("port distribution", distribution_type_, "Distribution over the ports or over the slots");

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
        data_ports_[0] = create_output_port<TimeSeriesType<double>>(
                                     TimeSeriesType<double>::Parameters(),
                                      PortOutPolicy(SlotRange(channelmap_().size())));
    }

}

void Distributor::CompleteStreamInfo() {
    incoming_batch_size_ = input_port_->prototype(0).nsamples();
    unsigned int s = 0;

    LOG(INFO) << name() << ". Incoming batch size: " << incoming_batch_size_
              << ".";

    if(distribution_type_()){
        for (auto &it : data_ports_) {
            for (s = 0; s < it.second->number_of_slots(); ++s) {
                it.second->streaminfo(s).set_parameters(
                            TimeSeriesType<double>::Parameters(
                                channelmap_().at(it.first).get_labels(),
                                incoming_batch_size_,
                                input_port_->prototype(0).sample_rate()));

                // when implemented, set datastream name in the packet to it->first

                it.second->streaminfo(s).set_stream_rate(
                            input_port_->streaminfo(0).stream_rate());
            }
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
    unsigned int s;

    std::vector<TimeSeriesType<double>::Data *> data_out_vector(
                channelmap_().size());


    while (!context.terminated()) {
        // retrieve new data packet
        if (!input_port_->slot(0)->RetrieveData(data_in)) {
            break;
        }

        for (auto &it : data_ports_) {
            for (s = 0; s < it.second->number_of_slots(); ++s) {
                data_out_vector.push_back(it.second->slot(s)->ClaimData(false));
            }
        }


        for (auto &data_out_ : data_out_vector) {
            data_out_->set_hardware_timestamp(
                        data_in->hardware_timestamp());
            data_out_->set_source_timestamp(
                        data_in->source_timestamp());
            data_out_->set_sample_timestamps(
                        data_in->sample_timestamps());

            for (auto ch: data_out_->labels()) {
                for (s = 0; s < incoming_batch_size_; s++) {
                    data_out_->set_data_sample(
                                s, ch,  data_in->data_sample(s, ch));
                }
            }
        }

        // publish data buckets
        for (auto &it : data_ports_) {
            for (s = 0; s < it.second->number_of_slots(); ++s) {
                it.second->slot(s)->PublishData();
            }
        }

        // release input data bucket
        input_port_->slot(0)->ReleaseData();
    }
}

void Distributor::Postprocess(ProcessingContext &context) {
    SlotType s;
    for (auto &it : data_ports_) {
        for (s = 0; s < it.second->number_of_slots(); ++s) {
            LOG(INFO) << name() << ". Port " << it.first << ". Slot " << s
                      << ". Streamed " << it.second->slot(s)->nitems_produced()
                      << " data packets. ";
        }
    }
}

REGISTERPROCESSOR(Distributor)
