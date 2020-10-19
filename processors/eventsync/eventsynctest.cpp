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


#include "eventsync/eventsync.hpp"
#include "gtest/gtest.h"

class TestEventSync : EventSync{
public:
  using EventSync::CreatePorts;
  using EventSync::Process;
  using IProcessor::load_fake_options;

public:


  PortIn<EventType> *getDataInPort() { return data_in_port_; };
  PortOut<EventType> *getDataOutPort() { return data_out_port_; };

};

namespace {
TEST(EventConverterTest, DefaultOptions) {

  TestEventSync p;
  EXPECT_EQ(p.getEventName(), "stimulation");
  EXPECT_TRUE(p.getReplace());
}

TEST(EventConverterTest, WrongOptions) {

  TestEventSync p;
  ASSERT_THROW(p.load_fake_options(YAML::Load("{options:{event name: null}}")),
               std::runtime_error);
}
}