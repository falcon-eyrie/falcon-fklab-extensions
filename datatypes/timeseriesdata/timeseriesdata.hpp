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

#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include "idata.hpp"
#include "columnsdata/columnsdata.hpp"

namespace nsTimeSeries {

struct Parameters :  nsColumn::Parameters {
  Parameters(size_t nchan = 0, size_t nsamp = 0, double rate = 1.0)
      :  nsColumn::Parameters(generate_labels(nchan), nsamp), sample_rate(rate) {}

  Parameters(std::vector<std::string> column_label, size_t nsamp = 0, double rate = 1.0)
      :  nsColumn::Parameters(column_label, nsamp), sample_rate(rate) {}

  double sample_rate;
};


template <typename T>
using Base = ColumnsType<T>;

template <typename T> class Data : public Base<T>::Data {

 public:
  typedef stride_iter<T *> column_iterator;
  typedef T *sample_iterator;

    /**
   * @brief Data constructor with the label of the columns, the number of samples and the sample rate.
   *
   * @param columns_label give a label for each column in the data / set also the number of columns in the dataset
   * @param nsamples  give the number of samples by column
   * @param sample_rate
   */
  Data(std::vector<std::string> columns_label, size_t nsamples, double sample_rate)
      : Base<T>::Data(columns_label, nsamples) {

      if (sample_rate <= 0) {
        throw std::runtime_error("Time Series Data::Initialize - sample rate "
                                 "needs to be larger than 0.");
      }

      sample_rate_ = sample_rate;
      timestamps_.resize(nsamples);
  }

  /**
   * @brief Data constructor with only the number of columns, the number of samples and the sample rate
   *
   * The label of column is generated from the column number ("0", "1"...,)
   *
   * @param ncolumns
   * @param nsamples
   * @param sample_rate
   */
  Data(size_t ncolumns, size_t nsamples, double sample_rate)
      : Data(generate_labels(ncolumns), ncolumns, nsamples, sample_rate) {}

  /**
   * @brief Data constructor based on the parameters object
   *
   * @param parameters - contains already the column labels either automatically generated
   * based on the number of columns or set directly.
   */
  Data(const Parameters &parameters)
      : Data(parameters.labels, parameters.nsamples, parameters.sample_rate){}

  static const std::string static_datatype() { return "time series"; }
  static const std::string static_dataname() { return "data"; }

  /**
   * @brief ClearData - clear the timestamps and the data (done in the columndatatype::Data)
   */
  void ClearData() override {
    Base<T>::Data::ClearData();
    std::fill(timestamps_.begin(), timestamps_.end(), 0);
  }

  Parameters parameters() const {
    return Parameters(this->labels_, this->nsamples_, sample_rate_);
  }

  double sample_rate() const { return sample_rate_; }

  /**
   * @brief sample_timestamp - give the timestamp corresponding to a sample number
   * @param sample
   * @return one timestamp
   */
  uint64_t sample_timestamp(size_t sample = 0) const {
    return timestamps_[sample];
  }

  /**
   * @brief sample_timestamps - give all the timestamps in the packet
   * @return  all timestamps
   */
  const std::vector<uint64_t> &sample_timestamps() const { return timestamps_; }
  std::vector<uint64_t> &sample_timestamps() { return timestamps_; }

  /**
   * @brief set_sample_timestamp - set one timestamp for a given sample number
   * @param sample
   * @param t
   */
  void set_sample_timestamp(size_t sample, uint64_t t) {
    if (sample >= this->nsamples_) {
      throw std::out_of_range(". Sample index " + std::to_string(sample) +
                              " out of range. Max index is " +
                              std::to_string(this->nsamples_ - 1));
    } else {
      timestamps_[sample] = t;
    }
  }

  /**
   * @brief set_sample_timestamps - set all timestamps for all sample
   * @param t
   */
  void set_sample_timestamps(std::vector<uint64_t> &t) {

    if(t.size() !=  this->nsamples_){
        throw  std::length_error(". the timestamps vector to set should have the same size as the number of samples ("
                                 + std::to_string(this->nsamples_)+" samples)");
    }
    timestamps_ = t;
  }

  /**
   * @brief FlatbufferDescription - This datatype add in the flexbuffer stream the timestamps and the type name.
   *
   * @note the columndata is already taking care of adding the data itself.
   *
   * @param flex_builder
   */
  void SerializeFlatBuffer(flexbuffers::Builder& flex_builder) override{
      flex_builder.TypedVector("timestamps", [&]{
             for(auto samples: timestamps_)
                 flex_builder.Add(samples);
      });

      flex_builder.String("type", static_datatype());
    }

  /**
   * @brief SerializeBinary - This datatype add in the binary stream the timestamps.
   *
   * @note the columndata is taking care of adding the data in the stream.
   *
   * @param stream
   * @param format
   */
  void SerializeBinary(std::ostream &stream,
                       Serialization::Format format =
                                   Serialization::Format::FULL) const override {

    Base<T>::Data::SerializeBinary(stream, format);

    if (format == Serialization::Format::FULL) {
            stream.write(reinterpret_cast<const char *>(timestamps_.data()),
                         timestamps_.size() * sizeof(uint64_t));
    }

    if (format == Serialization::Format::COMPACT) {
        for (size_t k = 0; k < this->nsamples_; ++k) {
              stream.write(reinterpret_cast<const char *>(&timestamps_[k]),
                           sizeof(uint64_t));
        }
    }
   }

  /**
   * @brief SerializeYAML - This datatype add as a yaml node the timestamps.
   *
   * @note the columndata is taking care of adding the data in the upper node.
   *
   * @param node
   * @param format
   */
  void SerializeYAML(YAML::Node &node,
                     Serialization::Format format =
                                 Serialization::Format::FULL) const override {

    Base<T>::Data::SerializeYAML(node, format);

    if (format == Serialization::Format::FULL ||
        format == Serialization::Format::COMPACT) {
      node["timestamps"] = timestamps_;

    }
  }

  /**
   * @brief YAMLDescription - this datatype add to the metadata describing the data format the
   * timestamp format.
   *
   * @param node
   * @param format
   */
  void YAMLDescription(YAML::Node &node,
                       Serialization::Format format =
                                   Serialization::Format::FULL) const override {

     Base<T>::Data::YAMLDescription(node, format);

     if (format == Serialization::Format::FULL) {
       node.push_back("timestamps uint64 (" + std::to_string(this->nsamples()) + ")");
     }

     if (format == Serialization::Format::COMPACT) {
       node.push_back("timestamps uint64 (1)");
     }

  }


protected:
  double sample_rate_;
  std::vector<uint64_t> timestamps_;
};

  using Capabilities = nsColumn::Capabilities;
}  // namespace nsMulticolumn

template <typename T>
using TimeSeriesType = DefineType<
  nsTimeSeries::Data<T>, ColumnsType<T>, false,
  nsTimeSeries::Capabilities, nsTimeSeries::Parameters
  >;
