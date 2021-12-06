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

#include <vector>
#include <string>
#include <algorithm>

#include "idata.hpp"

namespace nsVectorType {
using Base = AnyType;

struct Parameters {
  Parameters(unsigned int n) : size(n) {}
  unsigned int size;
};


template <typename TYPE>
class Data : public IData<Data<TYPE>,Base> {
 public:
  Data(unsigned int n) {
    if (n==0) {
      throw std::runtime_error("Vector size cannot be zero.");
    }
    data_.resize(n);
  }
  Data(const Parameters &parameters) : Data(parameters.size) {}
  
  static const std::string static_datatype() { return "vector [" + get_type_string<TYPE>() + "]"; }
  static const std::string static_dataname() { return "data"; }

  Parameters parameters() const {
    return Parameters(data_.size());
  }

  void setData(const std::vector<TYPE> &data) {
    if (data.size() != data_.size()) {
      throw std::runtime_error("Setting vector data from wrong size source vector");
    }
    data_ = data;  // copy
  }

  void setData(const TYPE *data, int len) {
    if (len != data_.size()) {
      throw std::runtime_error("Setting vector data from wrong size source data");
    }
    std::copy(data, data + len, data_.begin());
  }

  void setSample(int index, const TYPE &data) { data_[index] = data; }

  std::vector<TYPE> &data() { return data_; }

  void SerializeBinary(std::ostream &stream,
                               Serialization::Format format =
                                   Serialization::Format::FULL) const override {
    Base::Data::SerializeBinary(stream, format);
    if (format == Serialization::Format::FULL ||
        format == Serialization::Format::COMPACT) {
      stream.write(reinterpret_cast<const char *>(data_.data()),
                   data_.size() * sizeof(TYPE));
    }
  }

  void SerializeYAML(YAML::Node &node,
                             Serialization::Format format =
                                 Serialization::Format::FULL) const override {
    Base::Data::SerializeYAML(node, format);
    if (format == Serialization::Format::FULL ||
        format == Serialization::Format::COMPACT) {
      node["data"] = data_;
    }
  }

  void YAMLDescription(YAML::Node &node,
                               Serialization::Format format =
                                   Serialization::Format::FULL) const override {
    Base::Data::YAMLDescription(node, format);
    if (format == Serialization::Format::FULL ||
        format == Serialization::Format::COMPACT) {
      node.push_back("data " + get_type_string<TYPE>() + " (" +
                     std::to_string(data_.size()) + ")");
    }
  }

 protected:
  std::vector<TYPE> data_;
};

using Capabilities = Base::Capabilities;

}  // namespace nsVectorType

template <typename T>
using VectorType = DefineType<
  nsVectorType::Data<T>, AnyType, true,
  nsVectorType::Capabilities, nsVectorType::Parameters
  >;
