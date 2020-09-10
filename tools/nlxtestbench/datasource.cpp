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

#include "datasource.hpp"
#include <iostream>

#include "filesource.hpp"
#include "ripplesource.hpp"
#include "sinesource.hpp"
#include "squaresource.hpp"
#include "whitenoisesource.hpp"

std::vector<std::unique_ptr<DataSource>>
datasources_from_yaml(const YAML::Node &node) {
  std::vector<std::unique_ptr<DataSource>> sources;
  std::string source_name;
  std::string source_class;

  // node["sources"]
  if (!node || node.IsNull() ||(node["class"] and node["class"].as<std::string>() == "default")) {
    // create default data sources
    sources.push_back(
        std::unique_ptr<DataSource>(new WhiteNoiseSource(0.0, 1.0, 32000.0)));
  } else if (!node.IsSequence()) {
    throw std::runtime_error("Could not read sources.");
  } else {
    for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
      // source_name = it->first.as<std::string>();
      if (!it->IsMap() || !(*it)["class"]) {
        throw std::runtime_error("Please specify class of source");
      } else {
        source_class = (*it)["class"].as<std::string>();
        if (source_class == "nlx") {
          sources.push_back(std::unique_ptr<DataSource>(
              FileSource::from_yaml((*it)["options"])));
        } else if (source_class == "noise") {
          sources.push_back(std::unique_ptr<DataSource>(
              WhiteNoiseSource::from_yaml((*it)["options"])));
        } else if (source_class == "sine") {
          sources.push_back(std::unique_ptr<DataSource>(
              SineSource::from_yaml((*it)["options"])));
        } else if (source_class == "square") {
          sources.push_back(std::unique_ptr<DataSource>(
              SquareSource::from_yaml((*it)["options"])));
        } else if (source_class == "ripple") {
          sources.push_back(std::unique_ptr<DataSource>(
              RippleSource::from_yaml((*it)["options"])));
        }
      }
    }
  }
  return sources;
}
