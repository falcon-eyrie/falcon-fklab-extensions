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

#include "nlxsink.hpp"

#include <string>
#include <utility>

#include "idata.hpp"
#include "utilities/zmqutil.hpp"
#include "utilities/string.hpp"

NlxSink::NlxSink() : IProcessor() {
  add_option("address", address_, "Cheetah ip address");
  add_option("port", port_,"Cheetah network port.");
  add_option("ttl", ttl_,"TTL");
  add_option("event id", eventid_,"Event id.");
  add_option("system", system_, "could be oe or nlx");
}

void NlxSink::CreatePorts() {
  data_port_ = create_input_port<EventType>("data", EventType::Capabilities(),
                                 PortInPolicy(SlotRange(1, 256), false));
}

void NlxSink::Preprocess(ProcessingContext &context) {
  std::string address;

  socket_= std::make_unique<zmq::socket_t>(context.run().global().zmq(), ZMQ_REQ);
  address = "tcp://"+ address_() +":" + std::to_string(port_());
  socket_->connect(address.c_str());
}

void NlxSink::Process(ProcessingContext &context) {
  std::vector<typename EventType::Data *> data;
  zmq_frames buffer;
  zmq_frames reply;
  while (!context.terminated()) {
    for (int k = 0; k < data_port_->number_of_slots(); ++k) {
      if (!data_port_->slot(k)->RetrieveDataAll(data, 0)) {
        break;
      }

      for (auto &it : data) {

        reply.clear();
        buffer.clear();

        if(system_().compare("nlx")){
            buffer.push_back("event");
            buffer.push_back(it->event());
            buffer.push_back(std::to_string(ttl_()));
            buffer.push_back(std::to_string(eventid_()));

            if (!s_send_multi(*(socket_), buffer)) {
                LOG(DEBUG) << "failed to send zmq message.";
            }
            reply = s_blocking_recv_multi(*(socket_));

            LOG(DEBUG) << "nlx reply: " << reply[0];

        }else if(system_().compare("oe")){
            if (!s_send(*(socket_), "TTL "+ std::to_string(ttl_())+" 1")) {
                LOG(DEBUG) << "failed to send zmq message.";
            }
            reply = s_blocking_recv_multi(*(socket_));
            if (!s_send(*(socket_), "TTL "+ std::to_string(ttl_())+" 0")) {
                LOG(DEBUG) << "failed to send zmq message.";
            }
            reply = s_blocking_recv_multi(*(socket_));
            LOG(DEBUG) << "oe reply: " << reply[0];
        }


      }
      data_port_->slot(k)->ReleaseData();
    }
  }
}

REGISTERPROCESSOR(NlxSink)
