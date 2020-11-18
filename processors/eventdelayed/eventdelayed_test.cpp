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

#include "eventdelayed/eventdelayed.hpp"
#include "options/units.hpp"
#include "gtest/gtest.h"

class TestEventDelayed : EventDelayed {

public:
  using EventDelayed::CreatePorts;
  using EventDelayed::Preprocess;
  using EventDelayed::Configure;
  using EventDelayed::Process;
  using EventDelayed::delayed_range_;
  using IProcessor::load_fake_options;

  bool get_default_enabled() { return default_enabled_(); };
  int get_lockout_period() { return initial_lockout_period_(); }
  bool get_delayed_event() { return initial_delayed_event_(); }
  std::vector<long int> get_delayed_range() { return initial_delayed_range_(); }
  bool get_save_event() { return save_events_(); }
  std::string get_prefix() { return prefix_(); }
  PortIn<EventType> *getDataInPort() { return data_in_port_; };
  PortOut<EventType> *getDataOutPort() { return data_out_port_; };
};

namespace {

TEST(EventDelayedTest, DefaultOptions) {
  TestEventDelayed p;

  EXPECT_TRUE(p.get_default_enabled());
  EXPECT_TRUE(p.get_save_event());
  EXPECT_FALSE(p.get_delayed_event());
  EXPECT_EQ(p.get_lockout_period(), 50);
  EXPECT_EQ(p.get_prefix(), "stim_");

  std::vector <long int> expect_result = {150, 200};
  EXPECT_EQ(p.get_delayed_range(), expect_result);
}

TEST(EventDelayedTest, ChangeWrongOptions) {
  TestEventDelayed p;
  ASSERT_THROW(
      p.load_fake_options(YAML::Load("{options: {lockout period: -1}}")),
      std::runtime_error);
}

TEST(EventDelayedTest, Configure) {
  TestEventDelayed p;
  YAML::Node node = YAML::Load("{options: {delay range: [2, 20]}}");
  p.load_fake_options(node);
  std::vector <long int> expect_result = {2, 20};
  EXPECT_EQ(p.get_delayed_range(), expect_result);

  GlobalContext globalContext;
  p.Configure(globalContext);
  EXPECT_EQ(p.delayed_range_.lower(), 2);
  EXPECT_EQ(p.delayed_range_.upper(), 20);

}

TEST(EventDelayedTest, OnTimeProcess) {

  TestEventDelayed p;
  YAML::Node node = YAML::Load("{options: {enable saving: false}}");
  p.load_fake_options(node);

  p.CreatePorts();

  GlobalContext globalContext;
  RunContext runContext(globalContext);
  ProcessingContext context(runContext, "tests", false);
  p.Configure(globalContext);
  p.Preprocess(context);
  std::deque<EventType::Data *> fake_data;
  std::deque<long int> fake_delay;

  for (int i = 0; i < 10; i++) {
    auto data = new EventType::Data();

    data->Initialize();
    data->set_hardware_timestamp(i);
    std::string name = "target" + std::to_string(i);
    data->set_event(name);
    fake_delay.push_back(i * 10);
    fake_data.push_back(data);
  }

  p.getDataInPort()->slot(0)->SetFakeData(fake_data);
  p.getDataInPort()->slot(0)->SetFakeDelay(fake_delay);
  p.Process(context);
  auto event_data = p.getDataOutPort()->slot(0)->getData();
  sleep(2); // Give time to the log to be displayed
}

TEST(EventDelayedTest, DelayedProcess) {

  TestEventDelayed p;
  YAML::Node node = YAML::Load("{options: { "
                               "enable saving: false, "
                               "delayed: true, "
                               "delay range: [2, 20]}}");
  p.load_fake_options(node);

  p.CreatePorts();

  GlobalContext globalContext;
  RunContext runContext(globalContext);
  ProcessingContext context(runContext, "tests", false);
  p.Configure(globalContext);
  p.Preprocess(context);

  std::deque<EventType::Data *> fake_data;
  std::deque<long int> fake_delay;

  for (int i = 0; i < 10; i++) {
    auto data = new EventType::Data();

    data->Initialize();
    data->set_hardware_timestamp(i);
    std::string name = "target" + std::to_string(i);
    data->set_event(name);
    if ((i + 1) % 5 == 0) {
      fake_delay.push_back(10);
    } else {
      fake_delay.push_back(0);
    }

    fake_data.push_back(data);
  }

  p.getDataInPort()->slot(0)->SetFakeData(fake_data);
  p.getDataInPort()->slot(0)->SetFakeDelay(fake_delay);
  p.Process(context);
  auto event_data = p.getDataOutPort()->slot(0)->getData();

  sleep(2); // Give time to the log to be displayed
}

} // namespace