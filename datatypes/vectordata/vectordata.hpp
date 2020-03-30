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

#ifndef VECTORDATA_H
#define VECTORDATA_H

#include "idata.hpp"
#include <vector>

template <class TYPE>
class VectorData : public IData {
public:
    struct Parameters : IData::Parameters {
        Parameters( unsigned int n )
          : IData::Parameters(), size(n) {}
        
        unsigned int size;
    };
    
    class Capabilities : public IData::Capabilities {
    public:
        void Validate(const Parameters & parameters ) {
            if (parameters.size == 0) {
                throw std::runtime_error("Vector size cannot be zero.");
            }
        }
    };
    
    static const std::string datatype() { return "vector"; }

public:
    Initialize( const Parameters & parameters ) {
        data_.reserve(parameters.size);
    }
    
    void setData( const std::vector<TYPE>& data ) {
        
        data_ = data; //copy
    }
    
    void setData( const TYPE* data, int len ) {
        
        // assert( len == _data.size() );
        std::copy( data, data + len, data_.begin() );
    }
    
    void setSample( int index, const TYPE& data ) {
        
        data_[index] = data;
    }
    
    const std::vector<TYPE> data() { return data_; }
    
protected:
    std::vector<TYPE> data_;
};



#endif
