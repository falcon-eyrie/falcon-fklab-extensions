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

#include "idata.hpp"
#include "utilities/general.hpp"
#include "utilities/iterators.hpp"
#include "utilities/string.hpp"

typedef Range<size_t> SampleRange;

inline std::vector<std::string> generate_labels(size_t nchannels){
    std::vector<std::string> labels;
    for(size_t i=0; i<nchannels; i++){
        labels.push_back(std::to_string(i));
    }

    return labels;
}

namespace nsColumn {

using ParentType = AnyType;

struct Parameters{

  Parameters(const std::vector<std::string> &labels, size_t nsamp = 0)
     : ncolumns(labels.size()), nsamples(nsamp), labels(labels) {}

  size_t ncolumns;
  size_t nsamples;
  std::vector<std::string> labels;
};

template <typename T> class Data : public IData<Data<T>,ParentType> {
public:
    using BaseClass = IData<Data<T>,ParentType>;
    /**
     * @brief Data constructor with the labels for each column and the number of samples
     * @param labels give a label for each column
     * @param nsamples give the number of samples
     * @param streamname label specific to the datastream
     */
    Data(const std::vector<std::string>& labels, size_t nsamples){
        if (labels.size() == 0 || nsamples == 0) {
          throw std::runtime_error("Column Data::Initialize - number of "
                                   "columns/samples needs to be larger than 0.");
        }
        labels_ = labels;
        ncolumns_ = labels.size();
        nsamples_ = nsamples;
        data_.resize(ncolumns_ * nsamples_);
    }

    /**
     * @brief Data constructor from the parameters object
     * @param parameters
     */
    Data(const Parameters &parameters)
        : Data(parameters.labels, parameters.nsamples){}

    static const std::string static_datatype() { return "columnar [" + get_type_string<T>() + "]"; }
    static const std::string static_dataname() { return "data"; }

    /**
    * @brief ClearData - clear the data
    */
   void ClearData() override {
     std::fill(data_.begin(), data_.end(), 0);
   }

   Parameters parameters() const {
     return Parameters(labels_, nsamples_);
   }

   size_t ncolumns() const { return ncolumns_; }
   size_t nsamples() const { return nsamples_; }
   std::vector<std::string> labels() const{ return labels_; }

   /**
    * @brief set_data_column set all samples for one column based on the name of the column
    * @param column - selected column name
    * @param data - vector containing data for all samples
    */
   void set_data_column(std::string column, const std::vector<T> &data){

     if(data.size() !=  this->nsamples_){
        throw  std::length_error(". the data vector to set should have the same size as the number of columns ("
                                    + std::to_string(ncolumns_)+" columns)");
     }
     std::copy(data.begin(), data.end(), begin_column(column));
   }

   /**
    * @brief set_data_column set all samples for one column based on the column index
    * @param column - selected column index
    * @param data - vector containing data for all samples
    */
   void set_data_column(size_t column, const std::vector<T> &data){

     if(data.size() !=  this->nsamples_){
         throw  std::length_error(". the data vector to set should have the same size as the number of samples ("
                                  + std::to_string(nsamples_)+" samples)");
     }
     std::copy(data.begin(), data.end(), begin_column(column));
   }

   /**
    * @brief set_data_sample set column data for one sample
    * @param sample - selected sample index
    * @param data - vector containing data for all columns
    */
   void set_data_sample(size_t sample, const std::vector<T> &data) {
     if(data.size() !=  ncolumns_){
         throw  std::length_error(". the data vector to set should have the same size as the number of column in the dataset ("
                                  + std::to_string(ncolumns_)+" columns)");
     }
     std::copy(data.begin(), data.end(), begin_sample(sample));
   }

   /**
    * @brief set_data_sample set one data for one column index and one sample index
    * @param sample - selected sample index
    * @param column - selected column index
    * @param data
    */
   void set_data_sample(size_t sample, size_t column, T data) {
     if (sample >= nsamples_) {
       throw std::out_of_range(". Sample index " + std::to_string(sample) +
                               " out of range. Max index is " +
                               std::to_string(nsamples_ - 1));
     }
     if (column >= ncolumns_) {
       throw std::out_of_range(". Channel index " + std::to_string(sample) +
                               " out of range. Max index is " +
                               std::to_string(ncolumns_ - 1));
     }
     data_[flat_index(sample, column)] = data;
   }

   /**
    * @brief set_data_sample set one data for one column label and one sample index
    * @param sample - selected sample index
    * @param column - selected column label
    * @param data
    */
   void set_data_sample(size_t sample, std::string column, T data) {
       auto index =  extract_index_from_column(column);
       set_data_sample(sample, index, data);
   }

   /**
    * @brief data - return full data with this format [c1s1, c1s2.., c1sn, c2s1, c2s2 ..., cns1,...cnsn]
    *
    * @return
    */
   std::vector<T> &data() { return data_; }
   const std::vector<T> &data() const { return data_; }

   /**
    * @brief data_sample - return the data for one sample index and one column index
    * @param sample - selected sample index
    * @param column - selected column index
    * @return
    */
   const T &data_sample(size_t sample, size_t column = 0) const {
     return data_[flat_index(sample, column)];
   }

   /**
    * @brief data_sample - return the data for one sample index and one column label
    * @param sample - selected sample index
    * @param column - selected column label
    * @return
    */
   const T &data_sample(size_t sample, std::string column) const{
     return data_[flat_index(sample, extract_index_from_column(column))];
   }

   // Operator based on sample index / column index

   T &operator()(size_t sample, size_t column = 0) {
     return data_[flat_index(sample, column)];
   }
   const T &operator()(size_t sample, size_t column = 0) const {
     return data_[flat_index(sample, column)];
   }


   // Operator based on sample index / column label

   T &operator()(size_t sample, std::string column) {
     auto index =  extract_index_from_column(column);
     return data_[flat_index(sample, index)];
   }
   const T &operator()(size_t sample,  std::string column) const {
     auto index =  extract_index_from_column(column);
     return data_[flat_index(sample, index)];
   }

   // operator based on index from the full dataset
   // [column_index = index/ncolumns, sample_index = index%ncolumns]
   T &operator()(size_t index) { return data_[index]; }
   const T &operator()(size_t index) const { return data_[index]; }


   // iterators
   /**
    * @brief begin_sample iterator at the start of a sample index
    * @param sample
    * @return
    */
   T *begin_sample(size_t sample) { return &data_[flat_index(sample)]; }
   /**
    * @brief end_sample iterator at the end of a sample index
    * @param sample
    * @return
    */
   T *end_sample(size_t sample) { return begin_sample(sample) + ncolumns_; }

   const T *begin_sample(size_t sample) const {
     return &data_[flat_index(sample)];
   }
   const T *end_sample(size_t sample) const {
     return begin_sample(sample) + ncolumns_;
   }

   /**
    * @brief begin_column iterator at the start of the column based on the location in the dataset
    * @param column - column index
    * @return
    */
   stride_iter<T *> begin_column(size_t column) {
     return stride_iter<T *>(&data_[column], ncolumns_);
   }
   /**
    * @brief end_column - iterator at the end of the column based on the location in the dataset
    * @param column - column index
    * @return
    */
   stride_iter<T *> end_column(size_t column) {
     return begin_column(column) + nsamples_;
   }

   /**
    * @brief begin_column - iterator at the start of the column based on name of the column
    * @param column - column label
    * @return
    */
   stride_iter<T *> begin_column(std::string column) {
     auto index =  extract_index_from_column(column);
     return begin_column(index);
   }
   /**
    * @brief end_column - iterator at the end of the column based on name of the column
    * @param column - column label
    * @return
    */
   stride_iter<T *> end_column(std::string column) {
     return begin_column(column) + nsamples_;
   }

   /**
    * @brief SerializeBinary - add to the binary stream the data [columnxsample]
    * @param stream
    * @param format
    */
   void SerializeBinary(std::ostream &stream,
                        Serialization::Format format =
                                    Serialization::Format::FULL) const override {

     BaseClass::SerializeBinary(stream, format);
     if (format == Serialization::Format::FULL) {
        stream.write(reinterpret_cast<const char *>(data_.data()),
                            data_.size() * sizeof(T));
     }

     if (format == Serialization::Format::COMPACT) {
       for (size_t k = 0; k < nsamples_; ++k) {
           stream.write(reinterpret_cast<const char *>(&data_[flat_index(k)]),
                        sizeof(T) * ncolumns_);
       }
     }
   }

   /**
    * @brief SerializeFlatBuffer  - add to the flexbuffer the data labeled [label0:nsamples, label1:nsamples...],
    * the number of columns and the number of samples.
    * @param flex_builder
    */
   void SerializeFlatBuffer(flexbuffers::Builder& flex_builder) override{

       BaseClass::SerializeFlatBuffer(flex_builder);

       flex_builder.TypedVector("labels", [&]{
              for(auto label: labels_)
                  flex_builder.Add(label);
       });

       flex_builder.TypedVector("data", [&]{
              for(auto samples: data_)
                  flex_builder.Add(samples);
       });

       flex_builder.UInt("ncolumns", ncolumns());
       flex_builder.UInt("nsamples", nsamples());
   }

   /**
    * @brief SerializeYAML - add to the yaml node the data [columnxsample]
    * @param node
    * @param format
    */
   void SerializeYAML(YAML::Node &node,
                      Serialization::Format format =
                                  Serialization::Format::FULL) const override {
     BaseClass::SerializeYAML(node, format);
     if (format == Serialization::Format::FULL ||
         format == Serialization::Format::COMPACT) {
       node["signal"] = data_;
     }
   }

   /**
    * @brief YAMLDescription - this datatype add to the metadata describing the data format the
   * signal, number of columns and samples format.
   *
    * @param format
    */
   void YAMLDescription(YAML::Node &node,
                        Serialization::Format format =
                                    Serialization::Format::FULL) const override {
      BaseClass::YAMLDescription(node, format);
      if (format == Serialization::Format::FULL) {
        node.push_back("signal " + get_type_string<T>() + " (" +
                       std::to_string(nsColumn::Data<T>::nsamples()) + "," +
                       std::to_string(nsColumn::Data<T>::ncolumns()) + ")");
      }

      if (format == Serialization::Format::COMPACT) {
          node.push_back("signal " + get_type_string<T>() + " (" +
                       std::to_string(nsColumn::Data<T>::ncolumns()) + ")");
      }
   }

   /**
    * @brief sum_column - compute the (absolute) sum of all samples in the same column
    *
    * @param column - column label
    * @param absolute - absolute sum or not
    * @return (absolute) sum
    */
   T sum_column(std::string column, bool absolute=false) const {
     auto index =  extract_index_from_column(column);
     return sum_column(index);

   }

   /**
    * @brief sum_column - compute the (absolute) sum of all samples in the same column
    *
    * @param column - column index
    * @param absolute - absolute sum or not
    * @return (absolute) sum
    */
   T sum_column(size_t column, bool absolute=false) const {
     if(absolute){
        return std::accumulate(begin_column(column), end_column(column), 0,
                            [](T a, T b) { return a + std::abs(b); });
     }

     return std::accumulate(begin_column(column), end_column(column), 0.0);

   }

   /**
    * @brief mean_column - compute the mean of the (absolute) sum
    * of all samples in the same column
    *
    * @param column - column label
    * @param absolute - absolute sum or not
    * @return mean of the (absolute) sum
    */
   T mean_column(std::string column, bool absolute=false) const {
     return sum_column(column, absolute) / ncolumns_;
   }

   /**
    * @brief mean_column - compute the mean of the (absolute) sum
    * of all samples in the same column
    *
    * @param column - column index
    * @param absolute - absolute sum or not
    * @return mean of the (absolute) sum
    */
   T mean_column(size_t column, bool absolute=false) const {
     return sum_column(column, absolute) / ncolumns_;
   }

   /**
    * @brief sum_sample - compute the (absolute) sum of all column data for the same sample index
    *
    * @param sample - sample index
    * @param absolute - absolute sum or not
    * @return (absolute) sum
    */
   T sum_sample(size_t sample,  bool absolute=false) const {
     if(absolute){

        return std::accumulate(begin_sample(sample), end_sample(sample), 0,
                                [](T a, T b) { return a + std::abs(b); });
     }
     return std::accumulate(begin_sample(sample), end_sample(sample), 0.0);
   }

   /**
    * @brief mean_sample - compute the mean of the (absolute) sum of all column data for the same sample index
    *
    * @param sample - sample index
    * @param absolute - absolute sum or not
    * @return  the mean of the (absolute) sum
    */
   T mean_sample(size_t sample, bool absolute=false) const { return sum_sample(sample, absolute) / ncolumns_; }

 private:
   size_t extract_index_from_column(std::string column) const{
       auto it = std::find(labels_.begin(), labels_.end(), column);

       // If element was found
       if (it == labels_.end())
       {
            throw std::runtime_error(". The column " + column +
                                     " does not exist in this dataset.");
       }

       return it - labels_.begin();
   }

 protected:
   inline size_t flat_index(size_t sample, size_t channel) const {
     return channel + sample * ncolumns_;
   }
   inline size_t flat_index(size_t sample) const { return sample * ncolumns_; }

 protected:
   size_t ncolumns_;
   size_t nsamples_;
   std::string streamname_;

   std::vector<std::string> labels_;
   std::vector<T> data_;

};

class Capabilities{
 public:
  Capabilities(ChannelRange column_range,
               SampleRange sample_range =
               SampleRange(1, std::numeric_limits<uint32_t>::max()))
      : column_range_(column_range),
        sample_range_(sample_range),
        labels_({}) {}

  Capabilities(std::vector<std::string> labels,
               SampleRange sample_range =
                   SampleRange(1, std::numeric_limits<uint32_t>::max()))
      : column_range_(ChannelRange(labels.size())),
        sample_range_(sample_range), labels_(labels) {}

  ChannelRange column_range() const { return column_range_; }
  SampleRange sample_range() const { return sample_range_; }

  template <class T>
    void Validate(const Data<T> & prototype) {

    if (!sample_range_.inrange(prototype.nsamples())) {
      throw std::runtime_error(
          "Number of samples cannot be zero and needs to be in range " +
          sample_range_.to_string());
    }

    if (!column_range_.inrange(prototype.ncolumns())) {
      throw std::runtime_error(
          "Number of columns cannot be zero and needs to be in range " +
          column_range_.to_string());
    }

    auto cl = prototype.labels();
    for(auto label: labels_){
        if (std::find(cl.begin(), cl.end(), label)== cl.end())
        {
            throw std::runtime_error("Expected column: "+ label
                                     + " is not part of the column dataset given in input.");
        }
    }
  }

 protected:
  ChannelRange column_range_;
  SampleRange sample_range_;
  std::vector<std::string> labels_;
};

} // namespace nsColumn

template <typename T>
using ColumnsType = DefineType<
  nsColumn::Data<T>, AnyType, true,
  nsColumn::Capabilities, nsColumn::Parameters
  >;

