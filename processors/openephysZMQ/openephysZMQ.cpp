// ---------------------------------------------------------------------
// This file is part of falcon-core.
//
// Copyright (C) 2021 - present Neuro-Electronics Research Flanders
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
#include "openephysZMQ.hpp"

#include <algorithm>
#include <string>
#include <vector>

OpenEphysZMQ::OpenEphysZMQ() : IProcessor(PRIORITY_HIGH), builder_(flatbuilder_) {
    add_option("address", address_, "IP address of Open-Ephys zmq communication");
    add_option("port", port_,"Port of Open-Ephys zmq communication");
    add_option("npackets", npackets_,
               "The total number of data packets to read "
               "(0 means continuous recording).");
    add_option("batch size", batch_size_,
               "The number of data packets to concatenate into "
               "single multi-channel data bucket.");
    add_option("nchannels", nchannels_,
               "The number of channels in the data packet sent by Open-Ephys.");
}

void OpenEphysZMQ::CreatePorts() {
    data_port_= create_output_port<MultiChannelType<double>>(
          "data",
          MultiChannelType<double>::Capabilities(ChannelRange(nchannels_())),
          MultiChannelType<double>::Parameters(),
          PortOutPolicy(SlotRange(1), 500, WaitStrategy::kBlockingStrategy));
}


void OpenEphysZMQ::CompleteStreamInfo() {
    data_port_->streaminfo(0).set_parameters(
          MultiChannelType<double>::Parameters(
              nchannels_(), batch_size_()));
    data_port_->streaminfo(0).set_stream_rate(IRREGULARSTREAM);

}

void OpenEphysZMQ::Preprocess(ProcessingContext &context) {

  auto tcp_address = "tcp://" + address_() + ":" + std::to_string(port_());
  try {
    socket_ = zmq::socket_t(context.run().global().zmq(), ZMQ_SUB);
    zmq_setsockopt(socket_, ZMQ_SUBSCRIBE, nullptr, 0);
    socket_.connect(tcp_address);
  } catch (...) {
        throw ProcessingPreprocessingError("Error when connecting the socket to the address: "+ tcp_address, name());
  }

  LOG(INFO) << name() << ". Falcon is connected to the address: " << tcp_address
            << " and is ready to receive data from OpenEphys.";

  missing_packets_counter_ = 0;
  valid_packets_counter_ = 0;
  invalid_packets_counter_=0;
}

void OpenEphysZMQ::Process(ProcessingContext &context) {
  unsigned int sample_counter_ = batch_size_();
  MultiChannelType<double>::Data::sample_iterator data_out_iter;
  flatbuffers::VectorIterator<float, float> data_in_iter;
  MultiChannelType<double>::Data* data_out;
  const openephysflatbuffer::ContinuousData* data;

  while (!context.terminated()  && valid_packets_counter_ < npackets_()) {
      zmq_msg_t message;
      zmq_msg_init (&message);

      if (zmq_msg_recv(&message, socket_, ZMQ_DONTWAIT) != -1) {

          try {
              data = openephysflatbuffer::GetContinuousData(zmq_msg_data(&message));
          } catch (...) {
              LOG(DEBUG) << "Impossible to parse the packet received - skipping to the next.";
              invalid_packets_counter_++;
              continue;
          }

          if(data->n_channels() != nchannels_()){
              throw ProcessingError("The number of channels (" + std::to_string(data->n_channels())
                                    + ") received in the Open-Ephys packet is different "
                                    + "from the number of channels expected by Falcon ("
                                    + std::to_string(nchannels_()) + ").", name());
          }

          valid_packets_counter_++;
          uint64_t init_ts = data->timestamp();

          if (valid_packets_counter_ == 1) {
            first_valid_packet_arrival_time_ = Clock::now();
            LOG(INFO) << name() << ". Received first valid data packet"
                      << " (OE TS = " << data->timestamp() << ")";

          } else if (last_message_number_ + 1 !=
                     data->message_id()) {
            LOG(DEBUG) << name() << ". Message lost: "
                       <<  data->message_id() - last_message_number_;
            missing_packets_counter_++;
          }

          last_message_number_ =  data->message_id();

          uint64_t n_samples = data->n_samples();
          LOG(DEBUG) << name() << ". Number of samples in the packet: " << n_samples;
          data_in_iter = data->samples()->begin();

          for(uint64_t sample=0; sample<n_samples; sample++){
              if (sample_counter_ == batch_size_()) {
                 data_out= data_port_->slot(0)->ClaimData(false);
                 // set data bucket metadata
                 data_out->set_hardware_timestamp(init_ts);
                 data_out->set_source_timestamp();
                 sample_counter_ = 0;
              }

              data_out->set_sample_timestamp(sample_counter_, init_ts+sample);

              data_out_iter = data_out->begin_sample(sample_counter_);

              for(uint64_t channel=0; channel<nchannels_(); channel++){
                  (*data_out_iter) = *(data_in_iter + n_samples*channel);
                  ++data_out_iter;
              }

              ++data_in_iter;
              ++sample_counter_;

              if (sample_counter_ == batch_size_()) {
                  data_port_->slot(0)->PublishData();
              }
          }
      }
      zmq_msg_close(&message);
  }
}

void OpenEphysZMQ::Postprocess(ProcessingContext &context) {
  LOG_IF(UPDATE, (valid_packets_counter_ == npackets_()))
      << "Requested number of packets was read. You can now STOP processing.";

  std::chrono::milliseconds runtime(
      std::chrono::duration_cast<std::chrono::milliseconds>(
          Clock::now() - first_valid_packet_arrival_time_));

  LOG(UPDATE) << name() << ". Finished reading : " << valid_packets_counter_
              << " packets received over "
              << static_cast<double>(runtime.count()) / 1000
              << " seconds at a rate of "
              << valid_packets_counter_ /
                     (static_cast<double>(runtime.count()) / 1000)
              << " packets/second.";

  LOG(DEBUG)  << name() << ". " << invalid_packets_counter_ << " packets were detected as invalid and could not be parse.";
  LOG(DEBUG)  << name() << ". " << missing_packets_counter_ << " packets were detected as missing.";

  socket_.close();
}

REGISTERPROCESSOR(OpenEphysZMQ)
