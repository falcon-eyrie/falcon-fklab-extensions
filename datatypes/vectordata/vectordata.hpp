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

struct Parameters : Base::Parameters {
  Parameters(unsigned int n) : Base::Parameters(), size(n) {}

  unsigned int size;
};

class Capabilities : public Base::Capabilities {
 public:
  void Validate(const Parameters &parameters) {
    if (parameters.size == 0) {
      throw std::runtime_error("Vector size cannot be zero.");
    }
  }
};

template <typename TYPE> class Data : public Base::Data {
 public:
  void Initialize(const Parameters &parameters) {
    data_.resize(parameters.size);
  }

  void setData(const std::vector<TYPE> &data) {
    data_ = data;  // copy
  }

  void setData(const TYPE *data, int len) {
    // assert( len == _data.size() );
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
}  // namespace nsVectorType

template <class TYPE> class VectorType {
 public:
  static const std::string datatype() { return "vector"; }
  static const std::string dataname() { return "data"; }

  using Base = nsVectorType::Base;
  using Parameters = nsVectorType::Parameters;
  using Capabilities = nsVectorType::Capabilities;
  using Data = nsVectorType::Data<TYPE>;
};
