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

#ifndef DIO_H
#define DIO_H

#include <string>
#include <cstdint>
#include <stdexcept>
#include <limits>

enum class DigitalOutputMode {NONE, HIGH, LOW, TOGGLE, PULSE};

class DigitalStateError : public std::runtime_error {

public:
    DigitalStateError( std::string msg ) : runtime_error( msg ) {}
};


class DigitalState {
public:
    
    typedef uint64_t value_type;
    static const size_t MAX_NUM_BITS = std::numeric_limits<value_type>::digits;
    
    class reference {
        friend class DigitalState;
        reference(uint64_t * parent, size_t n) noexcept
          : parent_(parent), mask_(1<<n) {}                              // no public constructor
        
        uint64_t *parent_;
        uint64_t mask_;
    public:
        ~reference() {}
        operator bool() const;                     // convert to bool
        reference& operator= (bool x);             // assign bool
        reference& operator= (const reference& x); // assign bit
        reference& flip();                         // flip bit value
        bool operator~() const;                    // return inverse value
    };
    
    DigitalState(size_t n = 1, uint64_t value = 0);
    
    size_t size() const;
    
    bool operator[](size_t n) const;
    
    reference operator[](size_t n);
    
    std::string to_string(std::string high = "1",
                          std::string low = "0", std::string spacer= "",
                          std::string omit = "-") const;
    
    DigitalState mask() const {
        return DigitalState(n_, mask_);
    }
    
    void set_mask(const DigitalState & mask) {
        check_size(mask);
        mask_ = mask.value_;
    }
    
    void set_mask(size_t n, bool mask = true) {
        check_index(n, false);
        if (mask) { mask_ |= (1<<n); }
        else { mask_ &= ~(1<<n); }
    }
    
    void set_exclusive_mask(size_t n, bool mask = true){
        check_index(n, false);
        if (mask) { mask_ = 1<<n; }
        else { mask_ = default_mask_ & ~(1<<n); }
    }
    
    void reset_mask() {
        mask_ = default_mask_;
    }
    
    void set_mask_unset() {
        // enable those bits that are not set
        mask_ = ~value_ & default_mask_;
    }
    
    void set_mask_set() {
        // enable those bits that are set
        mask_ = value_;
    }
    
    void merge_mask() {
        value_ &= mask_;
        mask_ = default_mask_;
    }
    
    template <class T>
    operator T() const { return static_cast<T>(value_ & mask_); }
    
    template <class T>
    T extract(size_t n=0) {
        size_t max = MAX_NUM_BITS/std::numeric_limits<T>::digits;
        if (n >= max) {
            throw std::out_of_range("Cannot extract block (" + std::to_string(n) + " > " + std::to_string(max) + ").");
        }
        return static_cast<T>( (value_ & mask_) >> (n*std::numeric_limits<T>::digits) );
    }
    
    DigitalState& set(bool b = true);
    
    DigitalState& set(size_t n, bool b = true);
    
    DigitalState& masked_set(const DigitalState & mask, bool b = true);
    
    DigitalState& set(const DigitalState & value);
    
    DigitalState& reset();
    
    DigitalState& reset(size_t n);
    
    DigitalState& masked_reset(const DigitalState & mask);
    
    DigitalState& flip();
    
    DigitalState& flip(size_t n);
    
    bool any() const;
    bool none() const;
    bool all() const;
    
    bool mask_any() const;
    bool mask_none() const;
    bool mask_all() const;
    
    size_t count() const;
    
    DigitalState& operator&= (const DigitalState& rhs);
    
    DigitalState& operator|= (const DigitalState& rhs);
    
    DigitalState& operator^= (const DigitalState& rhs);
    
    DigitalState& operator<<= (size_t pos);
        
    DigitalState& operator>>= (size_t pos);
    
    DigitalState operator~() const;
    
    DigitalState operator<<(size_t pos) const;
    
    DigitalState operator>>(size_t pos) const;
    
    bool operator== (const DigitalState& rhs) const;
    
    bool operator!= (const DigitalState& rhs) const;

protected:
    void check_size(const DigitalState & other) const;
    
    void check_index(size_t n, bool mask = true) const;

protected:
    size_t n_;
    value_type value_;
    value_type default_mask_;
    value_type mask_;
};

DigitalState operator& (const DigitalState& lhs, const DigitalState& rhs);

DigitalState operator| (const DigitalState& lhs, const DigitalState& rhs);

DigitalState operator^ (const DigitalState& lhs, const DigitalState& rhs);


#endif // DIO_H
