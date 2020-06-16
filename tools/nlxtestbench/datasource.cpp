#include "datasource.hpp"

#include "filesource.hpp"
#include "whitenoisesource.hpp"
#include "squaresource.hpp"
#include "sinesource.hpp"

#include "yaml-cpp/yaml.h"

std::vector<std::unique_ptr<DataSource>> datasources_from_yaml(const YAML::Node & node) {

    std::vector<std::unique_ptr<DataSource>> sources;
    std::string source_name;
    std::string source_class;
    
    //node["sources"]
    if (!node || node.IsNull()) {
        // create default data sources
        sources.push_back( std::unique_ptr<DataSource>( new WhiteNoiseSource( 0.0, 1.0, 32000.0 ) ) );
    } else if (!node.IsSequence()) {
        throw std::runtime_error("Could not read sources.");
    } else {
        for(YAML::const_iterator it=node.begin();it!=node.end();++it) {
            //source_name = it->first.as<std::string>();
            if (!it->IsMap() || !(*it)["class"] ) {
                throw std::runtime_error("Please specify class of source");
            } else {
                source_class = (*it)["class"].as<std::string>();
                if (source_class=="nlx") {
                    sources.push_back( std::unique_ptr<DataSource>( FileSource::from_yaml((*it)["options"]) ) );
                } else if (source_class=="noise") {
                    sources.push_back( std::unique_ptr<DataSource>( WhiteNoiseSource::from_yaml((*it)["options"]) ) );
                } else if (source_class=="sine") {
                    sources.push_back( std::unique_ptr<DataSource>( SineSource::from_yaml((*it)["options"]) ) );
                } else if (source_class=="square") {
                    sources.push_back( std::unique_ptr<DataSource>( SquareSource::from_yaml((*it)["options"]) ) );
                }
                
            }
        }
    }

    return sources;
}
