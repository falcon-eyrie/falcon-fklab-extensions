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

#include "dio.hpp"
#include "interfaces/dio.hpp"
#include "utilities/time.hpp"

DigitalState::reference::operator bool() const {
    return (*parent_) & mask_;
}
DigitalState::reference& DigitalState::reference::operator= (bool x) {
    if (x) { *parent_ |= mask_;}
    else { *parent_ &= ~mask_; }
    return *this;
}
DigitalState::reference& DigitalState::reference::operator= (const DigitalState::reference& x) {
    if (*(x.parent_) & x.mask_) { *parent_ |= mask_; }
    else { *parent_ &= ~mask_; }
    return *this;
}
DigitalState::reference& DigitalState::reference::flip() {
    *parent_ ^= mask_;
    return *this;
}
bool DigitalState::reference::operator~() const {
    return !bool(*this);
}
    
DigitalState::DigitalState(size_t n, uint64_t value) : n_(n), value_(value) {
    if (n == 0 || n > MAX_NUM_BITS) {
        throw DigitalStateError("DigitalState represents at least 1 and at most " + std::to_string(MAX_NUM_BITS) + " bits.");
    }
    
    default_mask_ = n == MAX_NUM_BITS ? std::numeric_limits<value_type>::max() : (1 << n) - 1;
    mask_= default_mask_;
}
   
size_t DigitalState::size() const { return n_; }

bool DigitalState::operator[](size_t n) const {
    check_index(n);
    return value_ & (1<<n);
}

DigitalState::reference DigitalState::operator[](size_t n) {
    check_index(n);
    return DigitalState::reference(&value_, n);
}

std::string DigitalState::to_string(std::string high, std::string low, std::string spacer, std::string omit) const {
    
    std::string s = "";
    
    for ( size_t k=size(); k>0; --k ) {
        if (k<size()) { s += spacer; }
        if (mask_ & (1<<(k-1))) { s += (value_ & (1<<(k-1))) ? high : low; }
        else { s += omit; }
    }
    
    return s;
}

DigitalState& DigitalState::set(bool b) {
    if (b) { value_ |= mask_; }  // value_ = mask_
    else { value_ &= ~mask_; }  //value_ = 0
    return *this;
}

DigitalState& DigitalState::set(size_t n, bool b) {
    check_index(n);  // should we check mask?
    if (b) { value_ |= (1<<n); }
    else { value_ &= ~(1<<n); }
    return *this;
}

DigitalState& DigitalState::masked_set(const DigitalState & mask, bool b) {
    // the masks are combined!!
    check_size(mask);
    if (b) { value_ |= (mask.value_ & mask.mask_ & mask_); }  // value_ |= mask.value_
    else { value_ &= ~(mask.value_ & mask.mask_ & mask_); }  // value_ &= ~mask.value_
    return *this;
}

DigitalState& DigitalState::set(const DigitalState & value) {
    check_size(value);
    uint64_t m = value.mask_ & mask_;
    value_ = ( value.value_ & m ) | (value_ & ~m);
    return *this;
}

DigitalState& DigitalState::reset() { return set(false); }

DigitalState& DigitalState::reset(size_t n) { return set(n, false); }

DigitalState& DigitalState::masked_reset(const DigitalState & mask) { return masked_set(mask, false); }

DigitalState& DigitalState::flip() {
    value_ ^= mask_;
    return *this;
}

DigitalState& DigitalState::flip(size_t n) {
    check_index(n);  // should we check mask?
    value_ ^= (1<<n);
    return *this;
}

bool DigitalState::any() const {
    return (value_ & mask_) > 0;
}

bool DigitalState::none() const {
    return (value_ & mask_) == 0;
}

bool DigitalState::all() const {
    return value_ == mask_;
}

bool DigitalState::mask_any() const {
    return mask_ > 0;
}

bool DigitalState::mask_none() const {
    return mask_ == 0;
}

bool DigitalState::mask_all() const {
    return mask_ == default_mask_;
}

size_t DigitalState::count() const {
    size_t count = 0;
    uint64_t n = value_ & mask_;
    while (n) {
        n &= (n-1) ;
        count++;
    }
    return count;
}

DigitalState& DigitalState::operator&= (const DigitalState& rhs) {
    check_size(rhs);
    //value_ &= rhs.value_;
    value_ &= (rhs.value_ | ~(rhs.mask_& mask_));
    return *this;
}

DigitalState& DigitalState::operator|= (const DigitalState& rhs) {
    check_size(rhs);
    //value_ |= rhs.value_;
    value_ |= (rhs.value_ & rhs.mask_ & mask_);
    return *this;
}

DigitalState& DigitalState::operator^= (const DigitalState& rhs) {
    check_size(rhs);
    //value_ ^= rhs.value_;
    value_ ^= (rhs.value_ & rhs.mask_ & mask_);
    return *this;
}

DigitalState& DigitalState::operator<<= (size_t pos) {
    // we shift value, but not mask
    check_index(pos);
    value_ <<= pos;
    value_ &= default_mask_;
    return *this;
}
    
DigitalState& DigitalState::operator>>= (size_t pos) {
    // we shift value, but not mask
    check_index(pos);
    value_ >>= pos;
    return *this;
}

DigitalState DigitalState::operator~() const {
    DigitalState d(*this);
    return d.flip();
}

DigitalState DigitalState::operator<<(size_t pos) const {
    DigitalState d(*this);
    d <<= pos;
    return d;
}

DigitalState DigitalState::operator>>(size_t pos) const {
    DigitalState d(*this);
    d >>= pos;
    return d;
}

bool DigitalState::operator== (const DigitalState& rhs) const {
    check_size(rhs);
    return (value_ & mask_) == (rhs.value_ & rhs.mask_);
}

bool DigitalState::operator!= (const DigitalState& rhs) const {
    check_size(rhs);
    return (value_ & mask_) != (rhs.value_ & rhs.mask_);
}

void DigitalState::check_size(const DigitalState & other) const {
    if (size() != other.size()) {
        throw DigitalStateError("Number of bits is not equal.");
    }
}

void DigitalState::check_index(size_t n, bool mask) const {
    if (n>=n_ || (mask && (mask_ & (1<<n)) == 0) ) {
        throw std::out_of_range("Bit index out of range or bit is masked.");
    }
}

DigitalState operator& (const DigitalState& lhs, const DigitalState& rhs) {
    DigitalState d(lhs);
    d &= rhs;
    return d;
}

DigitalState operator| (const DigitalState& lhs, const DigitalState& rhs) {
    DigitalState d(lhs);
    d |= rhs;
    return d;
}

DigitalState operator^ (const DigitalState& lhs, const DigitalState& rhs) {
    DigitalState d(lhs);
    d ^= rhs;
    return d;
}


