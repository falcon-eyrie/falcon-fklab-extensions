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

#include <algorithm>
#include <string>
#include <vector>

#include "idata.hpp"

namespace nsVectorType {
using ParentType = AnyType;

struct Parameters {
    Parameters(unsigned int n) : size(n) {}
    unsigned int size;
};

template <typename T>
class Data : public IData<Data<T>, ParentType> {
   public:
    using BaseClass = IData<Data<T>, ParentType>;

    Data(unsigned int n) {
        if (n == 0) {
            throw std::runtime_error("Vector size cannot be zero.");
        }
        data_.resize(n);
    }
    Data(const Parameters& parameters) : Data(parameters.size) {}

    static const std::string static_datatype() { return "vector [" + get_type_string<T>() + "]"; }
    static const std::string static_dataname() { return "data"; }

    Parameters parameters() const { return Parameters(data_.size()); }

    void setData(const std::vector<T>& data) {
        if (data.size() != data_.size()) {
            throw std::runtime_error("Setting vector data from wrong size source vector");
        }
        data_ = data; // copy
    }

    void setData(const T* data, int len) {
        if (len != data_.size()) {
            throw std::runtime_error("Setting vector data from wrong size source data");
        }
        std::copy(data, data + len, data_.begin());
    }

    void setSample(int index, const T& data) { data_[index] = data; }

    std::vector<T>& data() { return data_; }

    void SerializeBinary(std::ostream& stream, Serialization::Format format =
                                                   Serialization::Format::FULL) const override {
        BaseClass::SerializeBinary(stream, format);
        if (format == Serialization::Format::FULL || format == Serialization::Format::COMPACT) {
            stream.write(reinterpret_cast<const char*>(data_.data()), data_.size() * sizeof(T));
        }
    }

    void SerializeYAML(YAML::Node&           node,
                       Serialization::Format format = Serialization::Format::FULL) const override {
        BaseClass::SerializeYAML(node, format);
        if (format == Serialization::Format::FULL || format == Serialization::Format::COMPACT) {
            node["data"] = data_;
        }
    }

    void YAMLDescription(YAML::Node& node, Serialization::Format format =
                                               Serialization::Format::FULL) const override {
        BaseClass::YAMLDescription(node, format);
        if (format == Serialization::Format::FULL || format == Serialization::Format::COMPACT) {
            node.push_back("data " + get_type_string<T>() + " (" + std::to_string(data_.size()) +
                           ")");
        }
    }

   protected:
    std::vector<T> data_;
};

using Capabilities = ParentType::Capabilities;

} // namespace nsVectorType

template <typename T>
using VectorType = DefineType<nsVectorType::Data<T>, AnyType, true, nsVectorType::Capabilities,
                              nsVectorType::Parameters>;
