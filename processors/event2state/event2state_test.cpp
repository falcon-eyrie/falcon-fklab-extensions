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

#include "event2state/event2state.hpp"
#include "options/units.hpp"
#include "gtest/gtest.h"

class TestEvent2State : public Event2State {

  public:
    using Event2State::event_counter_;

    std::string get_target() { return target_event_().event(); }
    PortIn<EventType> *getDataInPort() { return data_in_port_; };
    PortOut<EventType> *getDataOutPort() { return data_out_port_; };
};

namespace {

TEST(Event2StateTest, DefaultOptions) {
    TestEvent2State p;
    EXPECT_EQ(p.get_target(), "None");
}

TEST(Event2StateTest, ChangeWrongOptions) {
    TestEvent2State p;
    ASSERT_THROW(
        p.load_fake_options(YAML::Load("{options: {target event: null}}")),
        std::runtime_error);
}

TEST(Event2StateTest, Process) {

    TestEvent2State p;
    YAML::Node node = YAML::Load("{options: {target event: target}}");
    p.load_fake_options(node);

    p.CreatePorts();

    GlobalContext globalContext;
    RunContext runContext(globalContext);
    ProcessingContext context(runContext, "tests", false);
    std::deque<EventType::Data *> fake_data;
    std::deque<long int> fake_delay;

    for (int i = 0; i < 4; i++) {
        auto data = new EventType::Data();
        data->Initialize();
        data->set_hardware_timestamp(i);
        if (i % 2 == 0) {
            data->set_event("target");
        }
        data->set_event("no target");
        fake_data.push_back(data);
    }

    p.getDataInPort()->slot(0)->SetFakeData(fake_data);
    p.Process(context);
    p.Postprocess(context);
    auto event_data = p.getDataOutPort()->slot(0)->getData();
    sleep(2); // Give time to the log to be displayed

    EXPECT_EQ(p.event_counter_.all_received, 4);
    EXPECT_EQ(p.event_counter_.target, 2);
    EXPECT_EQ(p.event_counter_.non_target, 2);
}
} // namespace