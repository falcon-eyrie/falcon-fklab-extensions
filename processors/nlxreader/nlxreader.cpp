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
#include "nlxreader.hpp"

#include <chrono>
#include <limits>

constexpr uint16_t NlxReader::MAX_NCHANNELS;
constexpr decltype(NlxReader::MAX_NCHANNELS) NlxReader::UDP_BUFFER_SIZE;

NlxReader::NlxReader() : IProcessor(PRIORITY_HIGH) {
  add_option("address", address_, "IP address of Digilynx acquisition system.");
  add_option("port", port_,
             "Port number for communication with Digilynx acquisition system.");
  add_option("channelmap", channelmap_,
             "Mapping of channels to processor output ports.", true);
  add_option("npackets", npackets_,
             "The total number of data packets to read "
             "(0 means continuous recording).");
  add_option("batch size", batch_size_,
             "The number of data packets to concatenate into "
             "single multi-channel data bucket.");
  add_option("nchannels", nchannels_,
             "The number of channels of the Digilynx acquisition system.");
  add_option("update interval", update_interval_,
             "The time interval for updates on the received data from "
             "the Digilynx acquisition system.");
  add_option("trigger/enable", triggered_,
             "Whether or not to wait for hardware trigger to start "
             "streaming data packets.");
  add_option("trigger/channel", hardware_trigger_channel_,
             "Digital input channel to use as hardware trigger");
}

void NlxReader::Configure(const GlobalContext &context) {
  nlxrecord_.set_nchannels(nchannels_());
}

void NlxReader::CreatePorts() {
  for (auto &it : channelmap_()) {
    data_ports_[it.first] = create_output_port<MultiChannelType<double>>(
        it.first,
        MultiChannelType<double>::Capabilities(ChannelRange(it.second.size())),
        MultiChannelType<double>::Parameters(),
        PortOutPolicy(SlotRange(1), 500, WaitStrategy::kBlockingStrategy));
  }
}

void NlxReader::CompleteStreamInfo() {
  for (auto &it : data_ports_) {
    // finalize data type with nsamples == batch_size and nchannels taken from
    // channel map
    it.second->streaminfo(0).set_parameters(
        MultiChannelType<double>::Parameters(
            channelmap_().at(it.first).size(), batch_size_(),
            nlx::NLX_SIGNAL_SAMPLING_FREQUENCY));
    it.second->streaminfo(0).set_stream_rate(
        nlx::NLX_SIGNAL_SAMPLING_FREQUENCY / batch_size_());
  }
}

void NlxReader::Prepare(GlobalContext &context) {
  memset(reinterpret_cast<char *>(&server_addr_), 0, sizeof(server_addr_));
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_addr.s_addr = inet_addr(address_().c_str());
  server_addr_.sin_port = htons(port_());
}

void NlxReader::Preprocess(ProcessingContext &context) {
  sample_counter_ = batch_size_();
  valid_packet_counter_ = 0;
  const int y = 1;

  timestamp_ = nlx::INVALID_TIMESTAMP;
  last_timestamp_ = nlx::INVALID_TIMESTAMP;

  stats_.clear();

  if (context.test()) {
    prepare_latency_test(context);
  }

  sleep(1);  // reduces probability of missed packets when connecting to ongoing
            // stream

  if ((udp_socket_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    throw ProcessingPrepareError("Unable to create socket.", name());
  }
  LOG(UPDATE) << name() << ". Socket created.";
  setsockopt(udp_socket_, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
  if (bind(udp_socket_, (struct sockaddr *)&server_addr_,
           sizeof(server_addr_)) < 0) {
    throw ProcessingPreprocessingError("Socket binding failed.", name());
  }
  LOG(UPDATE) << name() << ". Socket binding successful.";
}

void NlxReader::Process(ProcessingContext &context) {
  bool update_time = false;
  int data_index = 0;
  MultiChannelType<double>::Data::sample_iterator data_iter;
  std::vector<MultiChannelType<double>::Data *> data_vector(data_ports_.size());

  while (!context.terminated() && valid_packet_counter_ < npackets_()) {
    // check if packets have arrived (with time-out)
    FD_ZERO(&file_descriptor_set_);  // clear the file descriptor set
    FD_SET(udp_socket_, &file_descriptor_set_);
    // add the socket (it's basically acting like a filedescriptor) to the set

    // set time-out
    timeout_.tv_sec = TIMEOUT_SEC;
    timeout_.tv_usec = 0;

    // packets available?
    ssize_t size =
        select(udp_socket_ + 1, &file_descriptor_set_, 0, 0, &timeout_);

    if (size == 0) {
      LOG(DEBUG) << name() << ": Timed out waiting for data. Connection lost?";
      continue;
    } else if (size == -1) {
      LOG(DEBUG) << name() << ": Select error on UDP socket.";
      continue;
    } else if (size > 0) {  // receive packet
      int recvlen =
          recvfrom(udp_socket_, buffer_, UDP_BUFFER_SIZE, 0, NULL, NULL);

      int rc = nlxrecord_.FromNetworkBuffer(buffer_, recvlen);

      if (rc != 0) {
        ++stats_.n_invalid;

        LOG(INFO) << name() << ": Received invalid record.";

        LOG(DEBUG) << name()<< ". STX field: " << nlxrecord_.buffer_[nlx::NLX_FIELD_STX]
                   << " instead of " << nlx::NLX_STX;
        LOG(DEBUG) << name() << ". Raw packet id:"<< nlxrecord_.buffer_[nlx::NLX_FIELD_RAWPACKETID]
                   << " instead of " << nlx::NLX_RAWPACKETID;
        LOG(DEBUG) << name() << ". Packet size field: "<< "Actual size: " << recvlen
                   << " \nReported size in the packet: " << nlxrecord_.buffer_[nlx::NLX_FIELD_PACKETSIZE]
                   << " \nExpected size: " << nlxrecord_.nlx_packetsize_;
        LOG_IF(DEBUG, rc == nlx::ERROR_BAD_CRC) << name() <<". Error Bad CRC";

        continue;
      }

      timestamp_ = nlx::CheckTimestamp(nlxrecord_, last_timestamp_, stats_);
      valid_packet_counter_++;

      if (valid_packet_counter_ == 1) {
        first_valid_packet_arrival_time_ = Clock::now();
        LOG(UPDATE) << name() << ": Received first valid data packet"
                    << " (TS = " << timestamp_ << ").";
      }

      update_time = valid_packet_counter_ % update_interval_() == 0;
      LOG_IF(UPDATE, update_time)
          << name() << ": " << valid_packet_counter_ << " packets ("
          << valid_packet_counter_ / nlx::NLX_SIGNAL_SAMPLING_FREQUENCY
          << " s) received.";
      print_stats(update_time);

      if (triggered_()) {
        LOG_IF(UPDATE, (valid_packet_counter_ == 1))
            << name() << ". Waiting for hardware trigger on channel "
            << hardware_trigger_channel_() << ".";
        if (nlxrecord_.parallel_port() & (1 << hardware_trigger_channel_())) {
          triggered_ = true;
          LOG(UPDATE) << name() << ". Dispatching starts now.";
        } else {
          continue;
        }
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
        data_vector[data_index]->set_sample_timestamp(sample_counter_,
                                                      nlxrecord_.timestamp());
        data_iter = data_vector[data_index]->begin_sample(sample_counter_);
        for (auto &channel : it.second) {
          (*data_iter) = nlxrecord_.sample_microvolt(channel);
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

void NlxReader::Postprocess(ProcessingContext &context) {
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
  print_stats();

  close(udp_socket_);

  if (context.test()) {
    save_source_timestamps_to_disk(valid_packet_counter_);
  }
}

void NlxReader::print_stats(bool condition) {
  LOG_IF(UPDATE, condition)
      << name() << ". Stats report: " << stats_.n_invalid << " invalid, "
      << stats_.n_duplicated << " duplicated, " << stats_.n_outoforder
      << " out of order, " << stats_.n_missed << " missed, " << stats_.n_gaps
      << " gaps.";
}

REGISTERPROCESSOR(NlxReader)
