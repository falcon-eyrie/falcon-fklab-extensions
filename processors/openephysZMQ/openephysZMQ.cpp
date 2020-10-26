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
#include "openephysZMQ.hpp"

OpenEphysZMQ::OpenEphysZMQ() : IProcessor(PRIORITY_HIGH) {
  add_option("address", address_, "ZMQ network address to connect");

  add_option("data port", data_port_, "ZMQ network port to subscribe to the data stream");
  add_option("hearbeat port", heartbeat_port_, "ZMQ network port to send a heartbeat to OpenEphys");
  add_option("channelmap", channelmap_,
             "Mapping of channels to processor output ports.", true);
  add_option("npackets", npackets_,
             "The total number of data packets to read "
             "(0 means continuous recording).");
  add_option("batch size", batch_size_,
             "The number of data packets to concatenate into "
             "single multi-channel data bucket.");
}


void OpenEphysZMQ::CreatePorts() {
  for (auto &it : channelmap_()) {
    data_ports_[it.first] = create_output_port<MultiChannelType<double>>(
        it.first,
            MultiChannelType<double>::Capabilities(ChannelRange(it.second.size())),
            MultiChannelType<double>::Parameters(),
            PortOutPolicy(SlotRange(1), 500, WaitStrategy::kBlockingStrategy));
  }
}

void OpenEphysZMQ::CompleteStreamInfo() {
  for (auto &it : data_ports_) {
    // finalize data type with nsamples == batch_size and nchannels taken from
    // channel map
    it.second->streaminfo(0).set_parameters(
        MultiChannelType<double>::Parameters(
            channelmap_().at(it.first).size(), batch_size_(),
            OPEN_EPHYS_SIGNAL_SAMPLING_FREQUENCY));
    it.second->streaminfo(0).set_stream_rate(
        OPEN_EPHYS_SIGNAL_SAMPLING_FREQUENCY / batch_size_());
  }
}

void OpenEphysZMQ::Preprocess(ProcessingContext &context) {

  try{
    data_socket_ = zmq::socket_t(context.run().global().zmq(), ZMQ_SUB);
    data_socket_.connect("tcp://" + address_() + ":" + std::to_string(data_port_()));
  }catch(...) {
    LOG(INFO) << "Error when connecting the socket to the address "
              << "tcp://" << address_() << ":" << std::to_string(data_port_());
  }


  try{
    heartbeat_socket_ = zmq::socket_t(context.run().global().zmq(), ZMQ_REQ);
    heartbeat_socket_.connect("tcp://" + address_() + ":" + std::to_string(heartbeat_port_()));
  }catch(...) {
    LOG(INFO) << "Error when connecting the socket to the address "
              << "tcp://" + address_() + ":" + std::to_string(heartbeat_port_());
  }

  LOG(INFO) << name() << ". Falcon is connected to the address tcp://" << address_() << ":" << std::to_string(data_port_())
            << " and is ready to receive data from OpenEphys.";
}

void OpenEphysZMQ::Process(ProcessingContext &context) {

  int data_index = 0;
  MultiChannelType<double>::Data::sample_iterator data_iter;
  std::vector<MultiChannelType<double>::Data *> data_vector(data_ports_.size());

  while (!context.terminated() && valid_packet_counter_ < npackets_()) {
    // Send a hearbeat to openEphys to say still alive

      d = {'application': self.app_name, 'uuid': self.uuid, 'type': 'heartbeat'}
      j_msg = json.dumps(d)
      logger.info("sending heartbeat")
      self.event_socket.send(j_msg.encode('utf-8'));

      zmq_frames data_msg;
      bool rcv = s_nonblocking_recv_multi(data_socket_, data_msg);
      if(rcv){


//zmq_frames event_msg = s_blocking_recv_multi(event_socket_);

      for(auto data: data_msg){
        LOG(INFO) << "data: " << data;
      }

   //   LOG(INFO) << "event: " << event_msg;
      // Check validity of the message
      // Load message in a record structure - timestamp + data
      // Extract timestamps
      timestamp_ = extract_timestamps(data_msg[2]);
      valid_packet_counter_++;

      if (valid_packet_counter_ == 1) {
        first_valid_packet_arrival_time_ = Clock::now();
        LOG(UPDATE) << name() << ": Received first valid data packet"
                    << " (TS = " << timestamp_ << ").";
      }


      // claim new data buckets
      if (sample_counter_ == batch_size_()) {
        data_index = 0;
        for (auto &it : data_ports_) {
          data_vector[data_index] = it.second->slot(0)->ClaimData(false);
          // set data bucket metadata
          data_vector[data_index]->set_hardware_timestamp(timestamp_);
          data_vector[data_index]->set_source_timestamp();
          data_index++;
        }
        sample_counter_ = 0;
      }

      // copy data onto buffers for each configured channel group
      data_index = 0;
      for (auto &it : channelmap_()) {
        //data_vector[data_index]->set_sample_timestamp(sample_counter_,
        //                                              nlxrecord_.timestamp());
        data_iter = data_vector[data_index]->begin_sample(sample_counter_);
        for (auto &channel : it.second) {
          //(*data_iter) = nlxrecord_.sample_microvolt(channel);
          ++data_iter;
        }
        data_index++;
      }

      ++sample_counter_;

      // publish data buckets
      if (sample_counter_ == batch_size_()) {
        for (auto &it : data_ports_) {
          it.second->slot(0)->PublishData();
        }
      }


  SlotType s;
  for (auto &it : data_ports_) {
    for (s = 0; s < it.second->number_of_slots(); ++s) {
      LOG(INFO) << name() << ". Port " << it.first << ". Slot " << s
                << ". Streamed " << it.second->slot(s)->nitems_produced()
                << " data packets. ";
    }
  }
  }else{
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  }
}

void OpenEphysZMQ::Postprocess(ProcessingContext &context) {
  LOG_IF(UPDATE, (valid_packet_counter_ == npackets_()))
    << "Requested number of packets was read. You can now STOP processing.";

  std::chrono::milliseconds runtime(
      std::chrono::duration_cast<std::chrono::milliseconds>(
          Clock::now() - first_valid_packet_arrival_time_));

  LOG(UPDATE) << name() << ". Finished reading : " << valid_packet_counter_
              << " packets received over "
              << static_cast<double>(runtime.count()) / 1000
              << " seconds at a rate of "
              << valid_packet_counter_ /
                 (static_cast<double>(runtime.count()) / 1000)
              << " packets/second.";

  data_socket_.close();
  event_socket_.close();
  heartbeat_socket_.close();

}


uint64_t OpenEphysZMQ::extract_timestamps(std::string msg){
  //TODO
  return 0;
}

REGISTERPROCESSOR(OpenEphysZMQ)