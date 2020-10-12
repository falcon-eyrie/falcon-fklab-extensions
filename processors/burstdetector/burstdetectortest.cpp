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


#include "burstdetector/burstdetector.hpp"
#include "options/units.hpp"
#include "gtest/gtest.h"

class TestBurstDetector : BurstDetector {
public:
  using BurstDetector::CompleteStreamInfo;
  using BurstDetector::CreatePorts;
  using BurstDetector::Postprocess;
  using BurstDetector::Preprocess;
  using BurstDetector::Process;
  using BurstDetector::running_statistics_;
  using BurstDetector::sample_rate_;
  using BurstDetector::stats_nsamples_;
  using BurstDetector::threshold_detector_;
  using IProcessor::fake_connection_input_port;
  using IProcessor::load_fake_options;

public:
  double getThreshold() { return initial_threshold_dev_(); }
  double getSmoothTime() { return initial_smooth_time_(); }
  double getDetectionLockoutTime() { return initial_detection_lockout_time_(); }
  bool getStreamEvents() { return default_stream_events_(); }
  bool getStatsOut() { return initial_stats_out_(); }
  double getStatsBufferSize() { return stats_buffer_size_(); }

  PortIn<MUAType> *getDataInPort() { return data_in_port_; };
  PortOut<EventType> *getDataOutPort() { return data_out_port_; };
  PortOut<MultiChannelType<double>> *getStatsPort() { return stats_out_port_; };

  void TestConfigure(float bin_size) {
    CreatePorts();
    fake_connection_input_port<MUAType>("mua", MUAType::Capabilities(),
                                        MUAType::Parameters(bin_size),
                                        PortOutPolicy(1));
  }
};

namespace {

TEST(BurstDetectorTest, DefaultOptions) {
  TestBurstDetector p;
  EXPECT_EQ(p.getThreshold(), 6.);
  EXPECT_EQ(p.getSmoothTime(), 10.);
  EXPECT_EQ(p.getDetectionLockoutTime(), 30.);
  EXPECT_EQ(p.getStatsBufferSize(), 0.5);
  EXPECT_TRUE(p.getStreamEvents());
  EXPECT_TRUE(p.getStatsOut());
}

TEST(BurstDetectorTest, ChangeOptions) {
  TestBurstDetector p;

  YAML::Node node = YAML::Load("{options: "
                               "{threshold dev: 7, "
                               "smooth time : 8}}");
  p.load_fake_options(node);

  EXPECT_EQ(p.getThreshold(), 7.);
  EXPECT_EQ(p.getSmoothTime(), 8.);
}

TEST(BurstDetectorTest, ChangeWrongOptions) {
  TestBurstDetector p;

  ASSERT_THROW(p.load_fake_options(YAML::Load("{options: {smooth time: -1}")),
               std::runtime_error);
  ASSERT_THROW(
      p.load_fake_options(YAML::Load("{options: {statistics buffer size: -3}")),
      std::runtime_error);
  ASSERT_THROW(p.load_fake_options(
                   YAML::Load("{options: {detection lockout time : -2}")),
               std::runtime_error);
}

TEST(BurstDetectorTest, CompleteStreamInfo) {
  TestBurstDetector p;
  p.TestConfigure(10);
  p.CompleteStreamInfo();
  EXPECT_EQ(p.stats_nsamples_, 50);
  EXPECT_EQ(p.getDataInPort()->streaminfo(0).stream_rate(),
            p.getStatsPort()->streaminfo(0).stream_rate());
  EXPECT_EQ(p.getDataOutPort()->streaminfo(0).stream_rate(), IRREGULARSTREAM);
}

TEST(BurstDetectorTest, WrongStreamConnexion) {
  // Statistics buffer size option * 1e3 < data bin size coming as input
  TestBurstDetector p;
  p.load_fake_options(YAML::Load("{options: "
                                 "{statistics buffer size = 0.5}}"));
  p.TestConfigure(1000);
  ASSERT_THROW(p.CompleteStreamInfo(), ProcessingCreatePortsError);
}

TEST(BurstDetectorTest, PreProcess) {
  TestBurstDetector p;
  float bin_size = 500;
  p.TestConfigure(bin_size);
  p.CompleteStreamInfo();
  GlobalContext globalContext;
  RunContext runContext(globalContext);
  ProcessingContext context(runContext, "tests", false);
  p.Preprocess(context);

  EXPECT_EQ(p.sample_rate_, 2);
  EXPECT_EQ(p.threshold_detector_->threshold(), 0);
  EXPECT_EQ(p.running_statistics_->alpha(), 0.05);
  EXPECT_EQ(p.running_statistics_->burn_in(), 20);
}

TEST(BurstDetectorTest, Process) {
  TestBurstDetector p;
  float bin_size = 500;
  p.TestConfigure(bin_size);
  p.CompleteStreamInfo();
  GlobalContext globalContext;
  RunContext runContext(globalContext);
  ProcessingContext context(runContext, "tests", false);
  p.Preprocess(context);

  std::deque<MUAType::Data *> fake_data;
  for (int i = 0; i < 300; i++) {
    auto data = new MUAType::Data();
    data->Initialize(500);
    if (i < 20) {
      data->set_n_spikes(5);
    } else if (i == 150) {
      data->set_n_spikes(100);
    } else {
      data->set_n_spikes(30);
    }

    data->set_hardware_timestamp(i);
    fake_data.push_back(data);
  }

  p.getDataInPort()->slot(0)->SetFakeData(fake_data);
  p.Process(context);
  auto event_data = p.getDataOutPort()->slot(0)->getData();
  auto stats_data = p.getStatsPort()->slot(0)->getData();

  // Expect first stats out hardware timestamp to be the 21 (because before it
  // was the burn-in section
  EXPECT_EQ(stats_data[0]->hardware_timestamp(), 20);

  // Expect the number of stats output packet being : (300 - burn_in )/ output
  // buffer size
  EXPECT_EQ(stats_data.size(),
            (300 - p.running_statistics_->burn_in()) / p.stats_nsamples_);

  // Expect two channels in the multichannel packet :
  EXPECT_EQ(stats_data[0]->data().size(), 2);
  // channel 0 = input data->mua - running_stats.center())
  EXPECT_EQ(stats_data[0]->data()[0], 50);

  // Expect 1 burst event from the input packet 150
  EXPECT_EQ(event_data.size(), 1);
  EXPECT_EQ(event_data[0]->event(), "burst");
  EXPECT_EQ(event_data[0]->hardware_timestamp(), 150);

  p.Postprocess(context);

  EXPECT_EQ(p.getDataOutPort()->slot(0)->nitems_produced(), 1);
}

} // namespace