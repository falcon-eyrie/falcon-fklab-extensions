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

#pragma once

#include "utilities/string.hpp"
#include <regex>

template <typename T> class ChannelList{
public:
    ChannelList(){};

    void add_channels(std::vector<std::string> channels){
        for(auto part: channels){
            part = std::regex_replace(part, std::regex(" "), "");
            if (std::regex_match(part, std::regex("^\\d+(\\-\\d+)?$"))) {
                auto range = split(part, '-');
                if(range.size()==1){
                    channels_.push_back(atoi(range[0].c_str()));
                }
                else{
                    add_channels(atoi(range[0].c_str()), atoi(range[1].c_str()));
                }
            } else {
              throw std::runtime_error(part + " is not a valid list specification.");
            }
        }
    };

    void add_channels(std::vector<T> channels){
        channels_.insert(channels_.end(), channels.begin(), channels.end());
    };

    void add_channels(T range_min, T range_max){
        for(T channel = range_min; channel <= range_max; channel++){
            channels_.push_back(channel);
        }
    }

    void remove_channels(double range_min, double range_max){
        for(T channel = range_min; channel <= range_max; channel++){
            channels_.erase(channels_.at(channel));
        }
    }

    void remove_channels(std::vector<T> channels){
        for(auto channel : channels){
            channels_.erase(channels_.at(channel));
        }

    }

    auto size() const{
        return channels_.size();
    };

    T &operator()(size_t index) { return channels_[index]; };
    const T &operator()(size_t index) const { return channels_[index]; };

    auto begin() const {
        return channels_.begin();
    };
    auto end() const {
        return channels_.end();
    };

    std::vector<T> channels_;
};

namespace YAML {

template <typename T> struct convert<ChannelList<T>> {
  static Node encode(const ChannelList<T> &rhs) {
    Node node;
    node = rhs.channels_;
    return node;
  }

  static bool decode(const Node &node, ChannelList<T> &rhs) {

    if(node.IsSequence()){
        rhs.add_channels(node.as<std::vector<std::string>>());
    }

    return true;
  }
};
}
