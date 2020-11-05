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
#include <algorithm>
#include <vector>
#include <string>

OpenEphysZMQ::OpenEphysZMQ() : IProcessor(PRIORITY_HIGH) {
  add_option("address", address_, "ZMQ network address to connect");

  add_option("data port", data_port_,
             "ZMQ network port to subscribe to the data stream");
  add_option("hearbeat port", heartbeat_port_,
             "ZMQ network port to send a heartbeat to OpenEphys");
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
    data_ports_[it.first] = create_output_port<MultiChannelType<float>>(
        it.first,
        MultiChannelType<float>::Capabilities(ChannelRange(it.second.size())),
        MultiChannelType<float>::Parameters(),
        PortOutPolicy(SlotRange(1), 500, WaitStrategy::kBlockingStrategy));
    for (auto &chan : it.second) {
      samples_[chan] = new std::vector<float>();

    }
    timestamps =  new std::vector<uint64_t>();
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

  try {
    data_socket_ = zmq::socket_t(context.run().global().zmq(), ZMQ_SUB);
    std::string source_id_ = "";
    data_socket_.setsockopt(ZMQ_SUBSCRIBE, source_id_.c_str(),
                            source_id_.length());
    data_socket_.connect("tcp://" + address_() + ":" +
                         std::to_string(data_port_()));
  } catch (...) {
    LOG(INFO) << "Error when connecting the socket to the address "
              << "tcp://" << address_() << ":" << std::to_string(data_port_());
  }

  LOG(INFO) << name() << ". Falcon is connected to the address tcp://"
            << address_() << ":" << std::to_string(data_port_())
            << " and is ready to receive data from OpenEphys.";

  data_corrupted_counter_ = 0;
  valid_packet_counter_ = 0;
}

void OpenEphysZMQ::Process(ProcessingContext &context) {

  sample_counter_ = 0;


  MultiChannelType<float>::Data *data_vector;

  while (!context.terminated()) {

    zmq_frames data_msg;
    s_nonblocking_recv_multi(data_socket_, data_msg);

    if (data_msg.size() >= 2) {
      json header = json::parse(data_msg[1]);

      // Header:
      // {"content":{"n_channels":16,"n_real_samples":928,"n_samples":1024,"sample_rate":40000,"timestamp":142912},
      // "data_size":65536,"message_no":197705,"type":"data"}
      LOG(DEBUG) << name() << ". Header: " << header;

      valid_packet_counter_++;

      if (valid_packet_counter_ == 1) { // first packet
        first_valid_packet_arrival_time_ = Clock::now();
        LOG(INFO) << name() << ": Received first valid data packet"
                  << " (TS = " << header["content"]["timestamp"] << ").";
      } else if (last_message_number + 1 !=
                 header["message_no"].get<int>()) { // Missed packet
        LOG(DEBUG) << name() << ". Message lost: "
                   << header["message_no"].get<int>() - last_message_number;
        data_corrupted_counter_++;
        continue;
      }
      last_message_number = header["message_no"].get<int>();

      // Data
      int n_sample = std::min(header["content"]["n_real_samples"].get<int>(),
                              header["content"]["n_samples"].get<int>());

      std::vector<uint32_t> buffer;
      buffer.reserve(header["content"]["n_channels"].get<int>()*header["content"]["n_samples"].get<int>());

      // copy data onto buffers for each configured channel group
      for (auto &it : samples_) {
        it.second->insert(it.second->end(), data_msg[2].begin()+n_sample * it.first,
                          data_msg[2].begin()+n_sample * (it.first+1));

      }
      float sample_rate_ = header["content"]["sample_rate"];
      std::vector<uint64_t> ts(n_sample);
      std::fill_n(ts.begin(), n_sample ,header["content"]["timestamp"]);
      std::transform (ts.begin(), ts.end(), ts.begin(), ts_increase);
      timestamps->insert(timestamps->end(), ts.begin(), ts.end());

      for (auto &it : channelmap_()) {

        if (samples_[it.second[0]]->size() < batch_size_()) {
          break;
        }
        LOG(DEBUG) << name() << " Send a new packet on the port " << it.first;
        data_vector = data_ports_[it.first]->slot(0)->ClaimData(false);
        // set data bucket metadata
        data_vector->set_hardware_timestamp(*timestamps->begin());
        data_vector->set_source_timestamp();
        // publish data buckets
        for (auto &channel : it.second) {
          std::vector<float> packet(samples_[channel]->begin(),
                                    samples_[channel]->begin()+batch_size_());
          data_vector->set_data_channel(channel, packet);
          samples_[channel]->erase(samples_[channel]->begin(),
                                  samples_[channel]->begin()+batch_size_());

        }
        std::vector<uint64_t> t(timestamps->begin(), timestamps->begin()+batch_size_());
        timestamps->erase(timestamps->begin(), timestamps->begin()+batch_size_());
        data_vector->set_sample_timestamps(t);
        data_ports_[it.first]->slot(0)->PublishData();
      }
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
}

uint64_t OpenEphysZMQ::extract_timestamps(std::string msg) {
  // TODO
  return 0;
}

REGISTERPROCESSOR(OpenEphysZMQ)