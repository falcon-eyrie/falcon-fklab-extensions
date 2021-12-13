// ---------------------------------------------------------------------
// This file is part of falcon-core.
//
// Copyright (C) 2021-now Neuro-Electronics Research Flanders
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

#include "columnsplitter.hpp"
#include "utilities/general.hpp"

ColumnSplitter::ColumnSplitter() : IProcessor(PRIORITY_MEDIUM) {
  add_option("ngroups", ngroups_,
             "The packet should be split and stream in N packets.", true);
}

void ColumnSplitter::CreatePorts() {
  input_port_ = create_input_port<TimeSeriesType<double>>(
      TimeSeriesType<double>::Capabilities(ChannelRange(1, MAX_N_CHANNELS)),
      PortInPolicy(SlotRange(1)));

   output_port_ = create_output_port<TimeSeriesType<double>>(
        "data",
        TimeSeriesType<double>::Parameters(),
        PortOutPolicy(SlotRange(ngroups_())));
}


void ColumnSplitter::CompleteStreamInfo() {
  incoming_batch_size_ = input_port_->prototype(0).nsamples();
  max_n_channels_ = input_port_->prototype(0).ncolumns();
  n_channel_by_group_ = max_n_channels_/ngroups_();
  auto sample_rate = input_port_->prototype(0).sample_rate();


  if(std::fmod(max_n_channels_, ngroups_()) !=0){
      throw ProcessingStreamInfoError("The packet (" + std::to_string(max_n_channels_)+
                                      ") should be split in packets with equal number of columns "
                                      "which is not possible when asking for " + std::to_string(ngroups_())+ " packets.");

  }

  LOG(INFO) << name() << ". Incoming batch size: " << incoming_batch_size_
            << ".";

  auto packet_labels_begin = input_port_->prototype(0).labels().begin();

  for(auto i=0; i< ngroups_(); i++){

      std::vector<std::string> labels(packet_labels_begin+i*n_channel_by_group_,
                                      packet_labels_begin+(i+1)*n_channel_by_group_);

      output_port_->streaminfo(i).set_parameters(
            TimeSeriesType<double>::Parameters(
                labels, incoming_batch_size_, sample_rate));

      output_port_->streaminfo(i).set_stream_rate(
            input_port_->streaminfo(0).stream_rate());
  }

}


void ColumnSplitter::Process(ProcessingContext &context) {
  TimeSeriesType<double>::Data *data_in = nullptr;
  unsigned int s;
  TimeSeriesType<double>::Data *data_out_;

  while (!context.terminated()) {
    // retrieve new data packet
    if (!input_port_->slot(0)->RetrieveData(data_in)) {
      break;
    }


    for (auto i=0; i < ngroups_(); i++) {
      data_out_ = output_port_->slot(i)->ClaimData(false);
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

      output_port_->slot(i)->PublishData();
    }
    // release input data bucket
    input_port_->slot(0)->ReleaseData();
  }
}

void ColumnSplitter::Postprocess(ProcessingContext &context) {

}

REGISTERPROCESSOR(ColumnSplitter)
