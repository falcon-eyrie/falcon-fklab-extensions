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


#include "eventfilter/eventfilter.hpp"
#include "gtest/gtest.h"

class TestEventFilter : EventFilter{
public:
  using EventFilter::CreatePorts;
  using EventFilter::Prepare;
  using EventFilter::Process;
  using EventFilter::event_counter_;
  using EventFilter::blocking_events_counter_;
  using IProcessor::fake_connection_input_port;
  using IProcessor::load_fake_options;

public:
  double getBlockOutTime() { return blockout_time_(); }
  double getBlockWaitTime() { return block_wait_time_(); }
  double getSyncTime() { return sync_time_(); }

  bool getDiscardWarning() { return discard_warnings_(); }
  SlotType getDetectionCriterion() {return detections_to_criterion_();};
  EventType::Data getTargetEvent() {return target_event_();};

  PortIn<EventType> *getDataInPort() { return data_in_port_; };
  PortIn<EventType> *getBlockInPort() { return block_in_port_; };
  PortOut<EventType> *getDataOutPort() { return data_out_port_; };

  void TestConfigure(int nbr_slot) {
    CreatePorts();
    fake_connection_input_port<EventType>("events",
                                          EventType::Capabilities(),
                                          EventType::Parameters(),
                                          PortOutPolicy(SlotRange(nbr_slot)));
  }
};

namespace {
TEST(EventFilterTest, DefaultOptions) {

  TestEventFilter p;
  EXPECT_EQ(p.getBlockOutTime(), 10);
  EXPECT_EQ(p.getBlockWaitTime(), 1.5);
  EXPECT_EQ(p.getSyncTime(), 3.5);
  EXPECT_EQ(p.getTargetEvent().event(), DEFAULT_EVENT);
  EXPECT_EQ(p.getDetectionCriterion(), 1);
  EXPECT_FALSE(p.getDiscardWarning());
}

TEST(EventFilterTest, WrongOptions) {

  TestEventFilter p;
  ASSERT_THROW(p.load_fake_options(YAML::Load("{options:{block duration: -1}}")),
               std::runtime_error);
  ASSERT_THROW(p.load_fake_options(YAML::Load("{options:{block wait time: -1}}")),
               std::runtime_error);
  ASSERT_THROW(p.load_fake_options(YAML::Load("{options:{sync time: -1}}")),
               std::runtime_error);
  ASSERT_THROW(p.load_fake_options(YAML::Load("{options:{target event: null}}")),
               std::runtime_error);
}

TEST(EventFilterTest, PrepareCriterion_0Slot) {
  TestEventFilter p;

  p.load_fake_options(YAML::Load("{options: {detection criterion: 0}}"));
  p.TestConfigure(3);

  GlobalContext context;
  p.Prepare(context);

  EXPECT_EQ(p.getDetectionCriterion(), 3);
}

TEST(EventFilterTest, PrepareCriterion_TooMuchSlot) {
  TestEventFilter p;

  p.load_fake_options(YAML::Load("{options: {detection criterion: 100}}"));
  p.TestConfigure(1);

  GlobalContext context;
  ASSERT_THROW(p.Prepare(context), ProcessingPrepareError);
}

TEST(EventFilterTest, Process) {
  TestEventFilter p;

  p.TestConfigure(1);

  GlobalContext globalContext;
  p.Prepare(globalContext);

  RunContext runContext(globalContext);
  ProcessingContext context(runContext, "tests", false);
  std::deque<EventType::Data*> fake_data;

  for (int i=0; i < 300; i++){
    auto target_data= new EventType::Data();
    auto block_data= new EventType::Data();
    if(i%2) {
      target_data->set_event("Not target event");
    }
    target_data->set_hardware_timestamp(i);
    if(i != 150){
      block_data->set_event("Not target event");
    }
    target_data->set_hardware_timestamp(i);

  }
  std::deque<EventType::Data*> block_fake_data;

  p.getDataInPort()->slot(0)->SetFakeData(fake_data);
  p.getBlockInPort()->slot(0)->SetFakeData(block_fake_data);
  p.Process(context);

  std::deque<EventType::Data*> output_data = p.getDataOutPort()->slot(0)->getData();
  std::cout << p.event_counter_.non_target << std::endl;
  std::cout << p.event_counter_.target << std::endl;
  std::cout << p.event_counter_.all_received << std::endl;
  std::cout << p.blocking_events_counter_.non_target << std::endl;
  std::cout << p.blocking_events_counter_.target << std::endl;
  std::cout << p.blocking_events_counter_.all_received << std::endl;
}

}



