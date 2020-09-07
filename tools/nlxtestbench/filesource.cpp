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

#include "filesource.hpp"

#include <iostream>

#include "utilities/string.hpp"
FileSource::FileSource(std::string file, bool cycle)
    : file_(file), cycle_(cycle) {
  try {
    raw_data_file.open(file_, std::ios::in | std::ios::binary);
    raw_data_file.seekg(std::ios::beg);
  } catch (...) {
    throw std::runtime_error("Unable to open file " + file_ + ".");
  }

  if (raw_data_file.bad() | raw_data_file.fail()) {
    throw std::runtime_error("Unable to open file " + file_ +
                             ". Check if filepath is correct.\n");
  }

  // read first three int32 to determine of this is a proper raw nlx file
  // and to determine the number of channels
  std::vector<int32_t> local_buffer{3};
  raw_data_file.read((char *)local_buffer.data(), 3 * sizeof(int32_t));

  convert_byte_order_ = false;

  if (local_buffer[0] == 2048) {
    // OK, no conversion needed
  } else if (local_buffer[0] == 8) {
    // OK, conversion needed (in-place)
    convert_byte_order_ = true;
    auto *p = (uint16_t *)local_buffer.data();
    for (unsigned int k = 0; k < 3 * sizeof(uint32_t) / sizeof(uint16_t); k++) {
      *(p + k) = ntohs(*(p + k));
    }
  } else {
    throw std::runtime_error("Cannot recognize file.");
  }

  if (local_buffer[1] != 1) {
    throw std::runtime_error("Cannot recognize file.");
  }

  nchannels_ = local_buffer[2] - nlx::NLX_NFIELDS_EXTRA;

  buffer_size_ = nlx::NLX_PACKETBYTESIZE(nchannels_);

  // check that file size is multiple of buffer size
  raw_data_file.seekg(0, std::ios::end);
  uint64_t length = raw_data_file.tellg();
  raw_data_file.seekg(0, std::ios::beg);

  if (length % buffer_size_ != 0) {
    throw std::runtime_error("File size is not a multiple of the record size.");
  }

  buffer_.resize(buffer_size_);
}

FileSource::~FileSource() { raw_data_file.close(); }

std::string FileSource::string() {
  return "file \"" + file() +
         "\" (fs = " + to_string_n(nlx::NLX_SIGNAL_SAMPLING_FREQUENCY) +
         ", "
         "nchannels = " +
         std::to_string(nchannels_) + ", " +
         "convert byte order = " + std::to_string(convert_byte_order_) + ")";
}

std::string FileSource::file() const { return file_; }

int64_t FileSource::Produce(char **data) {
  raw_data_file.read((char *)buffer_.data(), buffer_size_);

  if (raw_data_file.eof()) {
    if (cycle_) {
      raw_data_file.clear();
      raw_data_file.seekg(std::ios::beg);
      raw_data_file.read((char *)buffer_.data(), buffer_size_);
    } else {
      return 0;
    }
  }

  if (!raw_data_file) {
    std::cout << "Error reading from data file." << raw_data_file.bad() << " "
              << raw_data_file.fail() << " " << raw_data_file.eof()
              << std::endl;
    return 0;
  }

  *data = (char *)buffer_.data();
  return buffer_size_;
}

YAML::Node FileSource::to_yaml() const {
  YAML::Node node;
  node["file"] = file_;
  node["cycle"] = cycle_;
  return node;
}

FileSource *FileSource::from_yaml(const YAML::Node node) {
  return new FileSource(node["file"].as<std::string>(),
                        node["cycle"].as<bool>(false));
}
