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

#ifndef INTERFACES_MESSAGE_HPP
#define INTERFACES_MESSAGE_HPP


#include <string>

class MessageOut { 
public:
    // required interface methods
    virtual bool message_out_send(const std::string & message) = 0;
};

class MessageIn{ 
public:
    // required interface methods
    virtual bool message_in_retrieve(std::string & message) = 0;
};


#endif  // INTERFACES_MESSAGE_HPP
