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

#include <fstream>
#include <string>
#include <vector>

#include "datasource.hpp"

class FileSource : public DataSource {
public:
  FileSource(std::string file, bool cycle);
  ~FileSource() override;

  std::string string() override;
  int64_t Produce(char **data) override;
  YAML::Node to_yaml() const override;

  static FileSource *from_yaml(YAML::Node node);
  std::string file() const;

protected:
  std::string file_;
  bool cycle_;
  std::ifstream raw_data_file;
  std::vector<char> buffer_;
  unsigned int nchannels_;
  uint16_t buffer_size_;
  bool convert_byte_order_;
};
