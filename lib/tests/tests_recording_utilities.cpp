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

#include "../recording_utilities/channelist.hpp"
#include "gtest/gtest.h"

namespace {

TEST(AddChannels, onebyone) {

 auto channelist = ChannelList<int>();
 channelist.add_channels(1);
 auto result = channelist.get_channels();

 EXPECT_EQ(result.size(), 1);
 EXPECT_EQ(result[0], 1);

 channelist.add_channels(3);
 result = channelist.get_channels();

 EXPECT_EQ(result.size(), 2);
 EXPECT_EQ(result[0], 1);
 EXPECT_EQ(result[1], 3);

}

TEST(AddChannels, fromvector) {
 std::vector<int> input = {1, 3, 4};
 auto channelist = ChannelList<int>();
 channelist.add_channels(input);

 auto result = channelist.get_channels();

 EXPECT_EQ(result.size(), 3);
 EXPECT_EQ(result, input);

 channelist.add_channels(input);

 result = channelist.get_channels();
 std::vector<int> expected_output = {1, 3, 4, 1, 3, 4};
 EXPECT_EQ(result.size(), 6);
 EXPECT_EQ(result, expected_output);

}

TEST(AddChannels, fromrange) {
 auto channelist = ChannelList<int>();
 channelist.add_channels(1, 4);

 auto result = channelist.get_channels();
 std::vector<int> expected_output = {1, 2, 3, 4};
 EXPECT_EQ(result.size(), 4);
 EXPECT_EQ(result, expected_output);

 channelist.add_channels(5, 7);

 result = channelist.get_channels();
 expected_output = {1, 2, 3, 4, 5, 6, 7};
 EXPECT_EQ(result.size(), 7);
 EXPECT_EQ(result, expected_output);

}

TEST(RemoveChannels, fromvector) {
 std::vector<int> input = {1, 3, 4};
 auto channelist = ChannelList<int>();
 channelist.add_channels(1, 9);
 channelist.remove_channels(input);
 auto result = channelist.get_channels();
 std::vector<int> expected_output = {2, 5, 6, 7, 8, 9};
 EXPECT_EQ(result.size(), 6);
 EXPECT_EQ(result, expected_output);
}

TEST(RemoveChannels, fromrange) {
    std::vector<int> input = {1, 3, 4};
    auto channelist = ChannelList<int>();
    channelist.add_channels(1, 9);
    channelist.remove_channels(1, 4);
    auto result = channelist.get_channels();
    std::vector<int> expected_output = {5, 6, 7, 8, 9};
    EXPECT_EQ(result.size(), 5);
    EXPECT_EQ(result, expected_output);
}

TEST(InspectChannels, inrange) {
    auto channelist = ChannelList<int>();
    channelist.add_channels(1, 9);

    EXPECT_FALSE(channelist.in_range(0, 2));
    EXPECT_TRUE(channelist.in_range(0, 10));
}

TEST(InspectChannels, isunique) {
    auto channelist = ChannelList<int>();
    channelist.add_channels(1, 9);
    EXPECT_TRUE(channelist.is_unique());

    channelist.add_channels(1, 9);
    EXPECT_FALSE(channelist.is_unique());
}
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
