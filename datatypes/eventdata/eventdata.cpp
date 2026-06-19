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

#include "eventdata.hpp"

#include <cstdio>
#include <iostream>
#include <string>

using namespace nsEventType;

Data::Data(const std::string& event) : default_event_(event) {
    set_event(event);
}

void Data::ClearData() {
    set_event(default_event_);
}

std::string Data::event() const {
    return event_;
}

size_t Data::hash() const {
    return hash_;
}

size_t Data::size() const {
    return event_.size();
}

void Data::set_event(const std::string& event) {
    if (event.size() == 0) {
        throw std::runtime_error("Event string cannot be empty.");
    }

    event_ = event;
    hash_ = std::hash<std::string>()(event_);
}

void Data::set_event(const Data& source) {
    event_ = source.event();
    hash_ = source.hash();
}

namespace nsEventType {

bool operator==(const Data& e1, const Data& e2) {
    return e1.hash_ == e2.hash_;
}

bool operator!=(const Data& e1, const Data& e2) {
    return e1.hash_ != e2.hash_;
}
}  // namespace nsEventType

void Data::SerializeBinary(std::ostream& stream, Serialization::Format format) const {
    BaseClass::SerializeBinary(stream, format);
    if (format == Serialization::Format::FULL || format == Serialization::Format::COMPACT) {
        uint8_t timestamp_len = sizeof(hardware_timestamp_);
        stream.write(reinterpret_cast<const char*>(&timestamp_len), sizeof(uint8_t));
        stream.write(reinterpret_cast<const char*>(&hardware_timestamp_), timestamp_len);
        uint16_t event_len = static_cast<uint16_t>(event_.size());
        stream.write(reinterpret_cast<const char*>(&event_len), sizeof(uint16_t));
        stream.write(event_.data(), event_len);
    }
}

void Data::SerializeYAML(YAML::Node& node, Serialization::Format format) const {
    BaseClass::SerializeYAML(node, format);
    if (format == Serialization::Format::FULL || format == Serialization::Format::COMPACT) {
        node["event"] = event_;
    }
}

void Data::SerializeFlatBuffer(flexbuffers::Builder& flex_builder) {
    BaseClass::SerializeFlatBuffer(flex_builder);
    flex_builder.String("event", event_);
}

void Data::YAMLDescription(YAML::Node& node, Serialization::Format format) const {
    BaseClass::YAMLDescription(node, format);
    if (format == Serialization::Format::FULL || format == Serialization::Format::COMPACT) {
        node.push_back("event_string str (" + std::to_string(EVENT_STRING_LENGTH) + ")");
    }
}
