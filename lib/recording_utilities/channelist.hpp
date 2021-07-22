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

template <typename T> class ChannelList{
public:
    ChannelList(){};
    void fromVector(std::vector<T> channels){
        channels_ = channels;
    };
    void fromRange(double range_min, double range_max){
        std::vector<T> tmp(range_max-range_min);
        std::iota(tmp.begin(), tmp.end(), range_min);
        channels_ = tmp;
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
    if(node.IsMap() and node["range"] and node["range"].size()==2){
        rhs.fromRange(node["range"].as<std::vector<T>>()[0], node["range"].as<std::vector<T>>()[1]);
    }else if(node.IsSequence()){
        rhs.fromVector(node.as<std::vector<T>>());
    }else{
        return false;
    }

    return true;
  }
};
}
