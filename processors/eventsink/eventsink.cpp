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

#include "eventsink.hpp"

#include <string>
#include <utility>

#include "idata.hpp"
#include "utilities/zmqutil.hpp"
#include "utilities/string.hpp"

EventSink::EventSink() : IProcessor() {
  add_option("address", address_, "Cheetah ip address");
  add_option("port", port_,"Cheetah network port.");
  add_option("ttl", ttl_,"TTL");
  add_option("message", event_message_,"event message");
  add_option("event id", eventid_,"Event id.");
  add_option("system", system_, "could be oe, oe_ttl or nlx");
  add_option("interleave", interleave_, "always activate the same ttl or activate a ttl by input slots.");

}

void EventSink::Configure(const GlobalContext &context){
    if(system_() != "oe" and system_() != "nlx"){
        throw ProcessingConfigureError("System option can be only oe or nlx.", name());
    }
}
void EventSink::CreatePorts() {
  data_port_ = create_input_port<AnyType>("data", EventType::Capabilities(),
                                 PortInPolicy(SlotRange(1, 256), false));
}

void EventSink::Preprocess(ProcessingContext &context) {

  socket_= std::make_unique<zmq::socket_t>(context.run().global().zmq(), ZMQ_REQ);
  std::string address = "tcp://"+ address_() +":" + std::to_string(port_());
  socket_->connect(address.c_str());
  int t  = 3000;
  zmq_setsockopt(*(socket_), ZMQ_RCVTIMEO, &t, sizeof(t));
  serializer_.reset(Serialization::serializer(Serialization::Encoding::YAML, Serialization::Format::FULL));
}

void EventSink::Process(ProcessingContext &context) {
  std::vector<typename AnyType::Data *> data;
  zmq_frames buffer;
  zmq_frames reply;
  std::stringstream buffer_serialization;
  int ttl;
  while (!context.terminated()) {
    for (int k = 0; k < data_port_->number_of_slots(); ++k) {
      if (!data_port_->slot(k)->RetrieveDataAll(data, 0)) {
        break;
      }

      for (auto &it : data) {
        reply.clear();
        buffer.clear();
        if(interleave_()){
            ttl = ttl_() + k;
        }else{
            ttl = ttl_();
        }

        if(system_() == "nlx"){
            buffer.push_back("event");

            buffer_serialization.str("");
            buffer_serialization.clear();

            if (serializer_->Serialize(buffer_serialization, it, k, 0,
                                       data_port_->slot(k)->upstream_address().processor(),
                                       data_port_->slot(k)->upstream_address().port(),
                                       data_port_->slot(k)->upstream_address().slot())) {
                buffer.push_back(buffer_serialization.str());
                buffer.push_back(std::to_string(ttl));
                buffer.push_back(std::to_string(eventid_()));
                if (!s_send_multi(*(socket_), buffer)) {
                    LOG(DEBUG) << "failed to send zmq message.";
                }
                reply = s_blocking_recv_multi(*(socket_));

                LOG(DEBUG) << "nlx reply: " << reply[0];
            } else {
            LOG(WARNING) << name() << ": Unable to serialize data stream " << k;
            }


        }else if(system_()== "oe"){

            if (!s_send(*(socket_), "TTL "+ std::to_string(ttl)+" on="+std::to_string(it->hardware_timestamp()))) {
                LOG(DEBUG) << "failed to send zmq message.";
            }
            reply = s_blocking_recv_multi(*(socket_));
            LOG(DEBUG) << "oe reply: " << reply[0];

        }else if(system_()== "oe_ttl"){
            if (!s_send(*(socket_), "TTL "+ std::to_string(ttl)+" on=1")) {
                LOG(DEBUG) << "failed to send zmq message.";
            }
            reply = s_blocking_recv_multi(*(socket_));
            LOG(DEBUG) << "oe reply: " << reply[0];
            if (!s_send(*(socket_), "TTL "+ std::to_string(ttl)+" on=0")) {
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
  void EventSink::Postprocess(ProcessingContext &context) {
      socket_->close();

  }

REGISTERPROCESSOR(EventSink)
