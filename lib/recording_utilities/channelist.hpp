// ---------------------------------------------------------------------
// This file is part of falcon-core.
//
// Copyright (C) 2021-present Neuro-Electronics Research Flanders
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

#include <regex>
#include <vector>
#include <iostream>
#include "yaml-cpp/yaml.h"
#include "utilities/string.hpp"

template <typename T> class ChannelList{
public:
    ChannelList(){};

    std::vector<T> get_channels() const{
        return channels_;
    }

    void add_channels(T channel){
        channels_.push_back(channel);
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
        channels_.erase(std::remove_if(
                            channels_.begin(), channels_.end(),
                            [range_min, range_max](T x) {
                                return x >= range_min and x <= range_max;
                            }
                            ),channels_.end());
    }

    void remove_channels(std::vector<T> channels){
        channels_.erase(std::remove_if(
                            channels_.begin(), channels_.end(),
                            [channels](T x) {
                                return std::find(channels.begin(), channels.end(), x)!= channels.end();
                            }
                            ),channels_.end());
    }

    auto size() const{
        return channels_.size();
    };

    T &operator[](size_t index) { return channels_[index]; };
    const T &operator[](size_t index) const { return channels_[index]; };

    std::string to_string() const
    {
        std::string str="[";
        unsigned int index=0;
        bool inrange = false;
        while( index < channels_.size()-1 ){
            if(channels_[index]+1 == channels_[index+1]){
                if(!inrange){
                    str += std::to_string(channels_[index])+"-";
                }
                inrange = true;
            }else{
                str += std::to_string(channels_[index]) + ", ";
                inrange = false;
            }
            index++;

        }
        str += std::to_string(channels_[index])+"]";
        return str;

    }

    auto begin() const {
        return channels_.begin();
    };
    auto end() const {
        return channels_.end();
    };

    auto begin() {
        return channels_.begin();
    };
    auto end() {
        return channels_.end();
    };

    bool all_in_range(T range_min, T range_max) const {
        for (auto const &ch : channels_) {
            if (ch >= range_max or ch < range_min) {
                return false;
            }
        }
        return true;
    }

    bool is_sorted() const {
        return std::is_sorted(channels_.begin(), channels_.end());
    }

    bool is_unique() const {
        std::vector<T> sorted_vector = channels_;
        std::sort(sorted_vector.begin(), sorted_vector.end());
        return std::adjacent_find(sorted_vector.begin(), sorted_vector.end()) == sorted_vector.end();
    }

    void sort() {
        std::sort(channels_.begin(), channels_.end());
    }

    void unique() {
        std::sort(channels_.begin(), channels_.end());
        auto it = std::unique(channels_.begin(), channels_.end());
        channels_.resize( std::distance(channels_.begin(),it) );
    }
private:
    std::vector<T> channels_;
};


namespace YAML {

template <typename T> struct convert<ChannelList<T>> {
    static Node encode(const ChannelList<T> &rhs) {
        Node node;
        node = rhs.get_channels();
        return node;
    }

    static bool decode(const Node &node, ChannelList<T> &rhs) {

        if(node.IsSequence()){
            auto channels = node.as<std::vector<std::string>>();
            std::smatch submatch;
            for(auto part: channels){
                part = std::regex_replace(part, std::regex(" "), "");
                auto range = split(part, '-');
                if(range.size()==1){
                    rhs.add_channels(atoi(range[0].c_str()));
                }
                else if (range.size()==2){
                    rhs.add_channels(atoi(range[0].c_str()), atoi(range[1].c_str()));
                } else {
                    throw std::runtime_error(part + " is not a valid list specification.");
                }
            }
        }else{
            return false;
        }

        return true;
    }
};
}
