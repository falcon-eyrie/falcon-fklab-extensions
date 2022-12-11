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

    add_option("sample rate", sample_rate_,
               "Sample rate from Open-Ephys");

    add_option("nchannels", nchannels_,
               "The number of channels in the data packet sent by Open-Ephys.");
}

void OpenEphysZMQ::CreatePorts() {
    data_port_= create_output_port<TimeSeriesType<double>>(
          "data", TimeSeriesType<double>::Parameters(nchannels_(), 1, sample_rate_(), true),
          PortOutPolicy(SlotRange(1), 500, WaitStrategy::kBlockingStrategy));
}


void OpenEphysZMQ::CompleteStreamInfo() {
    data_port_->streaminfo(0).set_stream_rate(sample_rate_());

}

void OpenEphysZMQ::Preprocess(ProcessingContext &context) {

  auto tcp_address = "tcp://" + address_() + ":" + std::to_string(port_());
  try {
    socket_ = zmq::socket_t(context.run().global().zmq(), ZMQ_SUB);
    zmq_setsockopt(socket_, ZMQ_SUBSCRIBE, nullptr, 0);
    int t  = 3/(sample_rate_());
    zmq_setsockopt(socket_, ZMQ_RCVTIMEO, &t, sizeof(t));

    socket_.connect(tcp_address);
  } catch (...) {
        throw ProcessingPreprocessingError("Error when connecting the socket to the address: "+ tcp_address, name());
  }

  LOG(INFO) << name() << ". Falcon is connected to the address: " << tcp_address
            << " and is ready to receive data from OpenEphys.";

  missing_packets_counter_ = 0;
  valid_packets_counter_ = 0;
  invalid_packets_counter_=0;
  last_message_number_=0;
}

void OpenEphysZMQ::Process(ProcessingContext &context) {
  TimeSeriesType<double>::Data* data_out;
  const openephysflatbuffer::ContinuousData* data;
  unsigned int nmissed = 0;

  while (!context.terminated()  && valid_packets_counter_ < npackets_()) {
      zmq_msg_t message;
      zmq_msg_init (&message);

      if (zmq_msg_recv(&message, socket_, 0) != -1) {

          try {
              data = openephysflatbuffer::GetContinuousData(zmq_msg_data(&message));
          } catch (...) {
              LOG(DEBUG) << name() << ". Impossible to parse the packet received - skipping to the next.";
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

          nmissed = 0;
          if (valid_packets_counter_ == 1) {
            first_valid_packet_arrival_time_ = Clock::now();
            LOG(DEBUG) << name() << ". Received first valid data packet"
                      << " (OE TS = " << data->sample_num() << ")";
            last_message_number_ = data->sample_num();

          } else if (last_message_number_ !=
                     data->sample_num()) {
            LOG(INFO) << name() << ". "
                       <<  data->sample_num()  - last_message_number_
                       << " sample(s) losted - missing ts from " << last_message_number_
                       << " to " << data->sample_num();

            missing_packets_counter_+= data->sample_num()  - last_message_number_;
          }

          uint64_t n_samples = data->n_samples() ;
          LOG(DEBUG) << name() << ". Number of samples in the packet: " << n_samples;

          data_out = data_port_->slot(0)->ClaimData(true);
          if(n_samples > 0){
              data_out->set_nsamples(n_samples);

              std::copy(data->samples()->begin(), data->samples()->end(), data_out->data().begin());
              std::vector<uint64_t> ts(n_samples) ;
              std::iota (std::begin(ts), std::end(ts), data->sample_num());

              data_out->set_sample_timestamps(ts);

              data_out->set_hardware_timestamp(data->sample_num());
              data_out->set_source_timestamp();
              data_port_->slot(0)->PublishData();

              last_message_number_ = data->sample_num()+n_samples;
          }
      }
      zmq_msg_close(&message);
  }
}

void OpenEphysZMQ::Postprocess(ProcessingContext &context) {
  LOG_IF(UPDATE, (valid_packets_counter_ == npackets_()))
      << name() << ". Requested number of packets was read. You can now STOP processing.";

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

  LOG(INFO)  << name() << ". " << invalid_packets_counter_ << " packets were detected as invalid and could not be parse.";
  LOG(INFO)  << name() << ". " << missing_packets_counter_ << " samples were detected as missing.";

  socket_.close();
}

REGISTERPROCESSOR(OpenEphysZMQ)
