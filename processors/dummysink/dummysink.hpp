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

/* DummySink: takes an any data stream and eats it. Mainly used to show 
 * and test basic graph processing functionality.
 * 
 * input ports:
 * data <IData> (1 slot)
 *
 * output ports:
 * none
 *
 * exposed states:
 * tickle <bool> - logs message
 *
 * exposed methods:
 * kick - logs message
 *
 * options:
 * none
 * 
 */

#ifndef DUMMYSINK_H
#define DUMMYSINK_H

#include "isink.hpp"
#include "idata.hpp"

class DummySink : public ISink<IData>
{
public:
    virtual void SetPortName() override {port_name = "data";};
    virtual void CreateStates() override;
    virtual bool Process_start( ProcessingContext& context ) override;
    virtual bool Process_loop( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context )  override;

    YAML::Node Kick( const YAML::Node & node );

protected:

    ReadableState<bool>* tickle_state_;
    std::vector<IData*> data_in;
    uint64_t packet_counter = 0;
    uint64_t retrieve_counter = 0;

    bool eos = false;
    bool tickling = false;
};

#endif
