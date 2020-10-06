#include "gtest/gtest.h"
//#include "utilities.cpp"
#include "dummysink/dummysink.hpp"
#include "burstdetector/burstdetector.hpp"


class TestBurstDetector : BurstDetector
{
    public:double getThreshold() {return initial_threshold_dev_();}
    public:double getSmoothTime() {return initial_smooth_time_();}
    public:double getDetectionLockoutTime() {return initial_detection_lockout_time_();}
    public:bool getStreamEvents() {return default_stream_events_();}
    public:bool getStatsOut() {return initial_stats_out_();}
    public:double getStatsBufferSize() {return stats_buffer_size_();}

};



namespace {

TEST(BurstDetectorTest, DefaultOptions)
{
   TestBurstDetector p;
   EXPECT_EQ(p.getThreshold(), 6.);
   EXPECT_EQ(p.getSmoothTime(), 10.);
   EXPECT_EQ(p.getDetectionLockoutTime(), 30.);
   EXPECT_EQ(p.getStatsBufferSize(), 0.5);
   EXPECT_TRUE(p.getStreamEvents());
   EXPECT_TRUE(p.getStatsOut());
}
/*
TEST(BurstDetectorTest, ChangeWrongOptions)
{
   TestBurstDetector p;
   p.internal_Configure(add options here)
   EXPECT_EQ(p.getSmoothTime(), 10.);
   EXPECT_EQ(p.getDetectionLockoutTime(), 30.);
   EXPECT_EQ(p.getStatsBufferSize(), 0.5);
}*/


TEST(BurstDetectorTest, ChangeWrongOptions)
{
    TestBurstDetector p;
}
}


/* Test strategy
 * V fake default options --> class fake which expose options
 * - fake by changing value (how to pass options value)   ---> Create a friend graph engine in test_utilities lib [internal_Configure(processor_node, global_context_)]
 * - fake by putting wrong value (just one raise exception)
 *
 * - fake processing loop by sending a value in input port and look the output in output port  --> Setup a mock process for input/output port
 * - fake processing loop by sending a value and the output with change of states  --> Setup a mock process for state modification
 * - check pre-processing / post-processing independently
 *
 * Out of scope in extension lib : fake how the state, input/output port are modified. This 3 variables should be all mocked.
*/
