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

#include "nlxparser.hpp"

#include <algorithm>
#include <limits>
#include <memory>

std::string gapfill_to_string(GapFill x) {
  std::string s;
#define MATCH(p)                                                               \
  case (GapFill::p):                                                           \
    s = #p;                                                                    \
    break;
  switch (x) {
    MATCH(NONE)
    MATCH(ASAP);
    MATCH(DISTRIBUTED);
  }
#undef MATCH
  return s;
}

GapFill string_to_gapfill(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), (int (*)(int))std::toupper);
#define MATCH(p)                                                               \
  if (s == #p) {                                                               \
    return GapFill::p;                                                         \
  }
  MATCH(NONE)
  MATCH(ASAP);
  MATCH(DISTRIBUTED);
  throw std::runtime_error("Invalid GapFill value.");
#undef MATCH
}

NlxParser::NlxParser() : IProcessor(PRIORITY_HIGH) {
  add_option("batch size", batch_size_,
             "The number of data packets to concatenate into "
             "single multi-channel data bucket.");
  add_option("update interval", update_interval_,
             "The time interval for updates on the received data from "
             "the Digilynx acquisition system.");
  add_option("trigger/enabled", triggered_,
             "Whether or not to wait for hardware trigger to start "
             "streaming data packets.");
  add_option("trigger/channel", hardware_trigger_channel_,
             "Digital input channel to use as hardware trigger");
  add_option(
      "gap fill", gap_fill_,
      "Method of filling in missing data packets. If 'none', no filling of "
      "missed packets is performed. If 'asap', all missed packets will be "
      "filled with last available batch of samples. If 'distributed', "
      "missed packets will be filled with the last available batch of samples "
      "at each iteration.");
}

void NlxParser::CreatePorts() {
  data_in_port_ = create_input_port<VectorType<uint32_t>>(
      "udp", VectorType<uint32_t>::Capabilities(), PortInPolicy(SlotRange(1)));

  output_port_signal_ = create_output_port<MultiChannelType<double>>(
      "data",
      MultiChannelType<double>::Capabilities(
          ChannelRange(1, nlx::NLX_MAX_NCHANNELS)),
      MultiChannelType<double>::Parameters(), PortOutPolicy(SlotRange(1), 500));

  output_port_ttl_ = create_output_port<MultiChannelType<uint32_t>>(
      "ttl", MultiChannelType<uint32_t>::Capabilities(ChannelRange(1)),
      MultiChannelType<uint32_t>::Parameters(),
      PortOutPolicy(SlotRange(1), 500));

  n_invalid_ = create_broadcaster_state<uint64_t>(
      "n_invalid", 0, Permission::READ,
      "The number of invalid packets that were received.");
}

void NlxParser::CompleteStreamInfo() {
  nchannels_ = nlx::NLX_NCHANNELS_FROM_NFIELDS(
      data_in_port_->slot(0)->streaminfo().parameters().size);

  LOG(INFO) << name() << ": parsing " << nchannels_
            << " channels raw digilynx data.";

  nlxrecord_.set_nchannels(nchannels_);

  output_port_signal_->streaminfo(0).set_parameters(
      MultiChannelType<double>::Parameters(
          nchannels_, batch_size_(),
          data_in_port_->slot(0)->streaminfo().stream_rate()));

  output_port_signal_->streaminfo(0).set_stream_rate(
      data_in_port_->slot(0)->streaminfo().stream_rate() / batch_size_());

  output_port_ttl_->streaminfo(0).set_parameters(
      MultiChannelType<double>::Parameters(
          1, batch_size_(),
          data_in_port_->slot(0)->streaminfo().stream_rate()));

  output_port_ttl_->streaminfo(0).set_stream_rate(
      data_in_port_->slot(0)->streaminfo().stream_rate() / batch_size_());
}

void NlxParser::Prepare(GlobalContext &context) {
  // create channel list
  channel_list_.resize(nchannels_);
  for (unsigned int i = 0; i < nchannels_; i++) {
    channel_list_[i] = i;
  }
}

void NlxParser::Preprocess(ProcessingContext &context) {
  sample_counter_ = batch_size_();
  valid_packet_counter_ = 0;
  timestamp_ = nlx::INVALID_TIMESTAMP;
  last_timestamp_ = nlx::INVALID_TIMESTAMP;
  stats_.clear();
  n_filling_packets_ = 0;
}

void NlxParser::Process(ProcessingContext &context) {
  bool update_time = false;
  unsigned int i = 0;
  int b = 0;
  decltype(n_filling_packets_) packets_lag = 0;

  VectorType<uint32_t>::Data *data_in = nullptr;
  MultiChannelType<double>::Data::sample_iterator data_iter;
  MultiChannelType<double>::Data *data_out = nullptr;
  MultiChannelType<uint32_t>::Data *ttl_data_out = nullptr;

  while (!context.terminated()) {
    if (!data_in_port_->slot(0)->RetrieveData(data_in)) {
      break;
    }

    int rc = nlxrecord_.FromNetworkBuffer(data_in->data());
    if (rc != 0) {
      ++stats_.n_invalid;

      LOG(INFO) << name() << ": Received invalid record.";

      LOG(DEBUG) << name()<< ". STX field: " << nlxrecord_.buffer_[nlx::NLX_FIELD_STX]
                 << " instead of " << nlx::NLX_STX;
      LOG(DEBUG) << name() << ". Raw packet id:"<< nlxrecord_.buffer_[nlx::NLX_FIELD_RAWPACKETID]
                 << " instead of " << nlx::NLX_RAWPACKETID;
      LOG(DEBUG) << name() << ". Packet size field: "
                 << " \nReported size in the packet: " << nlxrecord_.buffer_[nlx::NLX_FIELD_PACKETSIZE]
                 << " \nExpected size: " << nlxrecord_.nlx_packetsize_;
      LOG_IF(DEBUG, rc == nlx::ERROR_BAD_CRC) << name() <<". Error Bad CRC";

      continue;
    }

    timestamp_ = nlx::CheckTimestamp(nlxrecord_, last_timestamp_, stats_);
    valid_packet_counter_++;
    data_in_port_->slot(0)->ReleaseData();

    if (valid_packet_counter_ == 1) {
      first_valid_packet_arrival_time_ = Clock::now();
      LOG(UPDATE) << name() << ". Received first valid data packet"
                  << " (TS = " << timestamp_ << ").";
    }

    if (triggered_()) {
      LOG_IF(UPDATE, (valid_packet_counter_ == 1))
          << name() << ". Waiting for hardware trigger on channel "
          << hardware_trigger_channel_() << ".";
      if (nlxrecord_.parallel_port() & (1 << hardware_trigger_channel_())) {
        triggered_ = false;
        LOG(UPDATE) << name() << ". Dispatching starts now.";
      } else {
        continue;
      }
    }

    update_time = valid_packet_counter_ % update_interval_() == 0;
    LOG_IF(UPDATE, update_time)
        << name() << ": " << valid_packet_counter_ << " packets ("
        << valid_packet_counter_ / data_in_port_->streaminfo(0).stream_rate()
        << " s) received.";
    print_stats(update_time);

    if (sample_counter_ == batch_size_()) {
      data_out = output_port_signal_->slot(0)->ClaimData(false);
      data_out->set_hardware_timestamp(timestamp_);
      ttl_data_out = output_port_ttl_->slot(0)->ClaimData(false);
      ttl_data_out->set_hardware_timestamp(timestamp_);
      sample_counter_ = 0;
    }

    // copy data from current packet onto buffer for each channel
    data_out->set_sample_timestamp(sample_counter_, timestamp_);
    ttl_data_out->set_sample_timestamp(sample_counter_, timestamp_);
    data_iter = data_out->begin_sample(sample_counter_);
    for (auto &channel : channel_list_) {
      (*data_iter) = nlxrecord_.sample_microvolt(channel);
      ++data_iter;
    }
    ttl_data_out->set_data_sample(sample_counter_, 0,
                                  nlxrecord_.parallel_port());
    ++sample_counter_;

    if (sample_counter_ == batch_size_()) {
      output_port_signal_->slot(0)->PublishData();
      output_port_ttl_->slot(0)->PublishData();
    }

    // stream additional packets if there were missed packets
    if (gap_fill_() != GapFill::NONE && sample_counter_ == batch_size_()) {
      packets_lag = stats_.n_missed - n_filling_packets_;
      if (packets_lag >= batch_size_()) {
        for (b = 0; b < packets_lag / batch_size_(); ++b) {
          data_out = output_port_signal_->slot(0)->ClaimData(false);
          LOG(DEBUG) << name() << ". mcd packet timestamp_: " << timestamp_;
          data_out->set_hardware_timestamp(timestamp_);
          ttl_data_out = output_port_ttl_->slot(0)->ClaimData(false);
          ttl_data_out->set_hardware_timestamp(timestamp_);
          LOG(DEBUG) << name() << ". mcd packet timestamp_: " << timestamp_;

          for (i = 0; i < batch_size_(); i++) {
            data_out->set_sample_timestamp(i, timestamp_);
            ttl_data_out->set_sample_timestamp(i, timestamp_);
            data_iter = data_out->begin_sample(i);

            for (auto &channel : channel_list_) {
              (*data_iter) = nlxrecord_.sample_microvolt(channel);
              ++data_iter;
            }

            ttl_data_out->set_data_sample(i, 0, nlxrecord_.parallel_port());
            LOG(DEBUG) << name() << ". timestamp_: " << timestamp_
                       << "; i=" << i;
          }
          output_port_signal_->slot(0)->PublishData();
          output_port_ttl_->slot(0)->PublishData();
          LOG(UPDATE) << name() << ". Streamed " << batch_size_()
                      << " duplicated samples to fill missed packets.";
          n_filling_packets_ += batch_size_();
          if (gap_fill_() == GapFill::DISTRIBUTED) {
            break;
          }
        }
      }
    }
  }
}

void NlxParser::Postprocess(ProcessingContext &context) {
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

  LOG(UPDATE) << name() << ". Streamed "
              << output_port_signal_->slot(0)->nitems_produced()
              << " multi-channel data items.";
}

void NlxParser::print_stats(bool condition) {
  LOG_IF(UPDATE, condition)
      << name() << ". Stats report: " << stats_.n_invalid << " invalid, "
      << stats_.n_duplicated << " duplicated, " << stats_.n_outoforder
      << " out of order, " << stats_.n_missed << " missed, " << stats_.n_gaps
      << " gaps. " << n_filling_packets_
      << " packets were filled. Synchronous lag: "
      << (stats_.n_missed - n_filling_packets_) /
             data_in_port_->slot(0)->streaminfo().stream_rate() * 1e3
      << " ms.";
}

REGISTERPROCESSOR(NlxParser)
