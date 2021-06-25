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

#include <string>
#include "idata.hpp"

namespace nsMUAType {

using Base = AnyType;

struct Parameters : Base::Parameters {
  Parameters(double bin = 0) : Base::Parameters(), bin_size(bin) {}
  double bin_size;
};

class Capabilities : public Base::Capabilities {
 public:
  virtual void Validate(const Parameters &parameters) const {
    if (parameters.bin_size <= 0) {
      throw std::runtime_error("Bin size cannot be smaller or equal to zero.");
    }
  }
};

class Data : public Base::Data {
 public:
  void Initialize(double bin_size);
  void Initialize(const Parameters &parameters);
  void ClearData() override;
  void set_n_spikes(unsigned int n_spikes);
  double mua() const;
  void set_bin_size(double bin_size);
  double bin_size();
  unsigned int n_spikes();

  void SerializeBinary(std::ostream &stream,
                       Serialization::Format format =
                       Serialization::Format::FULL) const final;

  void SerializeYAML(YAML::Node &node,
                     Serialization::Format format =
                     Serialization::Format::FULL) const final;

  void YAMLDescription(YAML::Node &node,
                       Serialization::Format format =
                       Serialization::Format::FULL) const final;

  void SerializeFlatBuffer(flexbuffers::Builder& flex_builder) final;

 protected:
  double bin_size_;  // in ms
  unsigned int n_spikes_;
};

}  // namespace nsMUAType

class MUAType {
 public:
  static const std::string datatype() { return "mua"; }
  static const std::string dataname() { return "mua"; }

  using Base = nsMUAType::Base;
  using Parameters = nsMUAType::Parameters;
  using Capabilities = nsMUAType::Capabilities;
  using Data = nsMUAType::Data;
};
