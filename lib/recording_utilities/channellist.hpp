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


    std::vector<std::pair<T, std::string>> get_channels() const{
        return channels_;
    }

    std::vector<T> get_channel_numbers() const{

        std::vector<T> channel_numbers;

        std::transform(channels_.begin(), channels_.end(),
                       std::back_inserter(channel_numbers),
                       [](auto const& pair){ return pair.first; });

        return channel_numbers;
    }

    std::vector<std::string> get_labels() const{

        std::vector<std::string> labels;

        std::transform(channels_.begin(), channels_.end(),
                       std::back_inserter(labels),
                       [](auto const& pair){ return pair.second; });

        return labels;
    }


    void add_channels(T channel, std::string label=""){
        if(label == ""){
            label = std::to_string(channel);
        }
        add_element(channel, label);
    };

    void add_channels(std::vector<T> channels, std::vector<std::string> labels={}){
        channels_.insert(channels_.end(), channels.begin(), channels.end());

        if (labels.size() == 0) {
            for(auto c : channels){
                add_element(c, std::to_string(c));
            }
        }
        else if(labels.size() != channels.size()){
            throw std::length_error(". channels and labels should have the same size.");
        }
        else{
            for(uint i=0; i<channels.size(); i++){
                add_element(channels[i], labels[i]);
            }

        }

    };

    void add_channels(T range_min, T range_max){
        for(T channel = range_min; channel <= range_max; channel++){
            add_channels(channel);
        }
    }

    void remove_channels(double range_min, double range_max){


       auto to_remove = std::remove_if(
                            channels_.begin(), channels_.end(),
                            [range_min, range_max](T x) {
                                return x.first >= range_min and x.first <= range_max;
                            }
                            );

       channels_.erase(to_remove, channels_.end());
    }

    void remove_channels(std::vector<T> channels){

        auto to_remove = std::remove_if(
                    channels_.begin(), channels_.end(),
                    [channels](T x) {
                        return std::find(channels.begin(), channels.end(), x.first)!= channels.end();
                    }
                    );

        channels_.erase(to_remove ,channels_.end());
    }

    void remove_channels(std::vector<std::string> labels){
        auto to_remove = std::remove_if(
                    channels_.begin(), channels_.end(),
                    [labels](T x) {
                        return std::find(labels.begin(), labels.end(), x.second)!= labels.end();
                    }
                    );

        channels_.erase(to_remove ,channels_.end());

    }

    auto size() const{
        return channels_.size();
    };

    T &operator[](size_t index) { return channels_[index]; };
    const T &operator[](size_t index) const { return channels_[index]; };

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

    /**
     * @brief to_string print the channel list in a yaml format understood by the options lib.
     * @note labels are not added in this format.
     *
     * @return
     */
    std::string to_string() const
    {
        std::string str="[";
        if(!channels_.empty()){

            unsigned int index=0;
            bool inrange = false;

            while( index < channels_.size()-1 ){
                if(channels_[index].first+1 == channels_[index+1].first){
                    if(!inrange){
                        str += std::to_string(channels_[index].first)+"-";
                    }
                    inrange = true;
                }else{
                    str += std::to_string(channels_[index].first) + ", ";
                    inrange = false;
                }
                index++;

            }
            str += std::to_string(channels_[index].first);

        }
        str += "]";
        return str;

    }

    /**
     * @brief all_in_range check if all the column numbers in this collection are contained in the range given in input.
     * @param range_min
     * @param range_max
     * @return
     */
    bool all_in_range(T range_min, T range_max) const {
        for (auto const &ch : channels_) {
            if (ch.first >= range_max or ch.first < range_min) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief is_subset check if the column numbers in this collection is a subset of the column numbers list given in input.
     * @param channels_vec
     * @return
     */
    bool is_subset(std::vector<T> channels_vec) const {
        const auto subset_vec = get_channel_numbers();
        const std::set<T> subset(subset_vec.begin(), subset_vec.end());
        const std::set<T> channels(channels_vec.begin(), channels_vec.end());

        return std::includes(channels.begin(), channels.end(), subset.begin(), subset.end());
    }

    /**
     * @brief is_subset check if the labels in this collection is a subset of the labels list given in input.
     * @param labels_vec
     * @return
     */
    bool is_subset(std::vector<std::string> labels_vec) const {
        const auto subset_vec = get_labels();
        const std::set<std::string> subset(subset_vec.begin(), subset_vec.end());
        const std::set<std::string> labels(labels_vec.begin(), labels_vec.end());
        return std::includes(labels.begin(), labels.end(), subset.begin(), subset.end());
    }

    /**
     * @brief is_sorted check without modifying the collection if it is sorted by column numbers or by labels
     *
     * @param by_label  optional, switch the sorting test to sort by labels instead of column numbers
     * @return
     */
    bool is_sorted(bool by_label=false) const {
        if(by_label){
            return std::is_sorted(channels_.begin(), channels_.end(),
                                  [](const auto &lhs, const auto &rhs) {
                                          return lhs.second < rhs.second; } );
        }

        return std::is_sorted(channels_.begin(), channels_.end());
    }

    /**
     * @brief is_unique check without modifying the collection if there is an unique set of column numbers.
     * @return
     */
    bool is_unique() const {
        auto channels = get_channels();
        std::sort(channels.begin(), channels.end());
        return std::adjacent_find(channels.begin(), channels.end()) == channels.end();
    }

    /**
     * @brief sort the collection of pair (column index, label)
     *
     * std::pair has a sort implementation by default - it sorts automatically by the first element (channel number)
     * and then in case of equality by the second element (label).
     *
     * @param by_label optional, optional, switch the sorting to sort by labels instead of column numbers
     *
     */
    void sort(bool by_label=false) {
        if(by_label){
            std::sort(channels_.begin(), channels_.end(),
                      [](const auto &lhs, const auto &rhs) {
                              return lhs.second < rhs.second; } );
        }else{
            std::sort(channels_.begin(), channels_.end());
        }
    }

    /**
     * @brief unique - Remove duplicates to keep only an unique set of channel numbers.
     * Because of the way sort works, the first label in alphabetical order is the one keep.
     *
     * No need to have an unique implementation for the labels as we cannot from the start have duplicate labels.
     */
    void unique() {
        sort();
        auto it = std::unique(channels_.begin(), channels_.end());
        channels_.resize( std::distance(channels_.begin(),it) );
    }

private :
    void add_element(T channel, std::string label){

        if(std::any_of(channels_.begin(), channels_.end(),
                       [&label](const std::pair<T, std::string>& ch)
                       { return ch.second == label; })){
            throw std::invalid_argument(". This label " + label + " already exist in this channel list and should be unique");
        }

        channels_.push_back(std::make_pair(channel, label));
    }

private:
    std::vector<std::pair<T, std::string>> channels_;
};


namespace YAML {

template <typename T> struct convert<ChannelList<T>> {
    static Node encode(const ChannelList<T> &rhs) {
        Node node;
        node = rhs.get_channel_numbers();
        return node;
    }

    static bool decode(const Node &node, ChannelList<T> &rhs) {

        if(node.IsSequence()){
            auto channels = node.as<std::vector<std::string>>();
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

typedef std::map<std::string, ChannelList<unsigned int>> ChannelMap;
