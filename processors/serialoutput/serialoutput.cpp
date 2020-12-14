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

#include "serialoutput.hpp"

#include <iostream>

SerialOutput::SerialOutput() {
  add_option("port address", port_address_, "Address of serial port");
  add_option("baud rate", baudrate_, "Serial rate exchange");
  add_option("event logging", event_log_, "Log message (UPDATE level) if true");
}

void SerialOutput::CreatePorts() {

  data_in_port_ = create_input_port<EventType>(EventType::Capabilities(),
                                               PortInPolicy(SlotRange(1, 10), false, 1));
}

void SerialOutput::Preprocess(ProcessingContext &context) {
  if (fd_.openDevice(port_address_().c_str(), baudrate_()) != 1) {
    throw ProcessingPreprocessingError(
        "Impossible to open the serial port specified: " + port_address_(), name());
  }

  LOG(INFO) << name() <<"Serial port " << port_address_() << " opened.";
}

void SerialOutput::Process(ProcessingContext &context) {

  EventType::Data *data_in = nullptr;
  auto nslots = data_in_port_->number_of_slots();
  while (!context.terminated()) {


      while (!context.terminated()) {

          for (int k = 0; k < nslots; ++k) {
              // retrieve new data
              if (!data_in_port_->slot(k)->RetrieveData(data_in))
                {
                  break;
                }

                auto nread = data_in_port_->slot(k)->status_read();

                if (nread == 0)
                {
                  data_in_port_->slot(k)->ReleaseData();
                  continue;
                }
              std::string message = data_in->event()+ "\0";

              if ((fd_.writeString(message.c_str())) != 1) {
                  LOG(INFO) << name() << ". Serial message " << message
                            << " not delivered.";
              } else {
                  LOG(INFO) << name() << ". Message " << data_in->event()
                            << " transmitted serially.";
              }

              data_in_port_->slot(k)->ReleaseData();
          }
      }
  }

}


void SerialOutput::Postprocess(ProcessingContext &context) {
  fd_.closeDevice();
}

REGISTERPROCESSOR(SerialOutput)