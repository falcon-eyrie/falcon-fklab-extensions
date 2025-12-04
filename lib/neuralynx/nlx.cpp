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

#include "nlx.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>

using namespace nlx;

bool valid_nlx_vt(VideoRec *vt_record, std::uint16_t vt_id,
                  ErrorNLXVT::Code &error_code,
                  decltype(NLX_VIDEO_RESOLUTION) resolution) {
    error_code = ErrorNLXVT::Code::NO_ERROR;
    if (vt_record->swstx != VTRecSWST) {
        error_code = ErrorNLXVT::Code::SWSTX;
        return false;
    }
    if (vt_record->swid != vt_id) {
        error_code = ErrorNLXVT::Code::SWID;
        return false;
    }
    if (sizeof(VideoRec) != vt_record->swdata_size) {
        error_code = ErrorNLXVT::Code::SWDATA_SIZE;
        return false;
    }
    if ((vt_record->dnextracted_x < 0) || (vt_record->dnextracted_y < 0)) {
        error_code = ErrorNLXVT::Code::NEGATIVE_COORDINATE;
        return false;
    }
    if (vt_record->dnextracted_x >= resolution[0] ||
        vt_record->dnextracted_y >= resolution[1]) {
        error_code = ErrorNLXVT::Code::OUT_OF_RESOLUTION;
        return false;
    }
    return true;
}

NlxSignalRecord::NlxSignalRecord(unsigned int nchannels,
                                 bool convert_byte_order)
    : convert_byte_order_(convert_byte_order) {
    set_nchannels(nchannels);
}

unsigned int NlxSignalRecord::nchannels() const { return nchannels_; }

void NlxSignalRecord::set_nchannels(unsigned int n) {
    nchannels_ = n;

    nlx_nfields_ = NLX_NFIELDS(nchannels_);
    nlx_packetbytesize_ = NLX_PACKETBYTESIZE(nchannels_);
    nlx_field_crc_ = nlx_field_crc(nchannels_);
    nlx_field_data_last_ = nlx_field_data_last(nchannels_);
    nlx_packetsize_ = nlx_packetsize(nchannels_);

    buffer_.resize(nlx_nfields_);
    Initialize();
}

bool NlxSignalRecord::convert_byte_order() const { return convert_byte_order_; }

void NlxSignalRecord::set_convert_byte_order(bool b) {
    convert_byte_order_ = b;
}

int NlxSignalRecord::FromNetworkBuffer(const char *buffer, size_t n) {
    // check size
    if (n != nlx_packetbytesize_) {
        return ERROR_TOO_SMALL_PACKET;
    }

    std::copy(buffer, buffer + n, (char *)buffer_.data());

    std::uint32_t tmp =
        ((NLX_STX << 8) & 0xFF00FF00) | ((NLX_STX >> 8) & 0xFF00FF);

    if (buffer_[NLX_FIELD_STX] == tmp) {
        char *p = (char *)buffer_.data();
        for (unsigned int k = 0; k < n; k += 2) {
            set_convert_byte_order(true);
            *((uint16_t *)(p + k)) = ntohs(*((uint16_t *)(buffer + k)));
        }
    }

    // test if valid record (record size os OK, first 3 fields are OK, CRC
    // checks out)
    return valid(buffer_);
}

size_t NlxSignalRecord::ToNetworkBuffer(char *buffer, size_t n) {
    // check size
    if (n < nlx_packetbytesize_) {
        return 0;
    }

    // finalize if necessary
    if (!finalized_) {
        Finalize();
    }

    char *p = (char *)buffer_.data();

    if (convert_byte_order_) {
        // perform hton conversion, copying into provided buffer in the process
        for (unsigned int k = 0; k < nlx_packetbytesize_; k += 2) {
            *((uint16_t *)(buffer + k)) = htons(*((uint16_t *)(p + k)));
        }
    } else {
        std::copy(p, p + n, buffer);
    }

    return nlx_packetbytesize_;
}

void NlxSignalRecord::Initialize() {
    std::fill(buffer_.begin(), buffer_.end(), 0);
    buffer_[NLX_FIELD_STX] = NLX_STX;
    buffer_[NLX_FIELD_RAWPACKETID] = NLX_RAWPACKETID;
    buffer_[NLX_FIELD_PACKETSIZE] = nlx_packetsize(nchannels_);
    initialized_ = true;
}

void NlxSignalRecord::Finalize() {
    buffer_[nlx_field_crc_] = crc();
    finalized_ = true;
}

int32_t NlxSignalRecord::crc() const {
    int32_t c = 0;
    for (unsigned int k = 0; k < nlx_nfields_ - 1; ++k) {
        c ^= buffer_[k];
    }
    return c;
}

bool NlxSignalRecord::initialized() const { return initialized_; }

bool NlxSignalRecord::finalized() const { return finalized_; }

int NlxSignalRecord::valid(std::vector<int32_t> buffer) {

    if (buffer[NLX_FIELD_STX] != NLX_STX) {
        initialized_ = false;
        return ERROR_NLX_FIELD_STX;
    } else if (buffer[NLX_FIELD_RAWPACKETID] != NLX_RAWPACKETID) {
        initialized_ = false;
        return ERROR_NLX_FIELD_RAWPACKETID;
    } else if (buffer[NLX_FIELD_PACKETSIZE] != nlx_packetsize_) {
        initialized_ = false;
        return ERROR_NLX_FIELD_PACKETSIZE;
    }

    if (buffer[nlx_field_crc_] != crc()) {
        finalized_ = false;
        return ERROR_BAD_CRC;
    }

    initialized_ = true;
    finalized_ = true;

    return SUCCESS_READING_BUFFER;
}

uint64_t NlxSignalRecord::timestamp() const {
    uint64_t t;
    t = (uint32_t)buffer_[NLX_FIELD_TIMESTAMP_HIGH];
    t = (t << 32) + (uint32_t)buffer_[NLX_FIELD_TIMESTAMP_LOW];
    return t;
}
void NlxSignalRecord::set_timestamp(uint64_t t) {
    buffer_[NLX_FIELD_TIMESTAMP_HIGH] = (int32_t)(t >> 32);
    buffer_[NLX_FIELD_TIMESTAMP_LOW] = (int32_t)t;
    finalized_ = false;
}
void NlxSignalRecord::inc_timestamp(uint64_t delta) {
    set_timestamp(timestamp() + delta);
}
void NlxSignalRecord::inc_timestamp(double delta) {
    set_timestamp(
        timestamp() +
        static_cast<uint64_t>(1000000 * delta / NLX_SIGNAL_SAMPLING_FREQUENCY));
}

uint32_t NlxSignalRecord::parallel_port() const {
    return static_cast<uint32_t>(buffer_[NLX_FIELD_DIO]);
}
void NlxSignalRecord::set_parallel_port(uint32_t dio) {
    buffer_[NLX_FIELD_DIO] = dio;
    finalized_ = false;
}

void NlxSignalRecord::data(std::vector<int32_t> &v) const {
    if (v.size() < nchannels_) {
        v.resize(nchannels_);
    }
    std::copy(data_begin(), data_end(), v.begin());
}

std::vector<int32_t>::iterator
NlxSignalRecord::data(std::vector<int32_t>::iterator it) const {
    return std::copy(data_begin(), data_end(), it);
}

int32_t NlxSignalRecord::sample(unsigned int index) const {
    return buffer_[NLX_FIELD_DATA_FIRST + index];
}

void NlxSignalRecord::set_data(int32_t value) {
    std::fill(data_begin(), data_end(), value);
    finalized_ = false;
}

void NlxSignalRecord::set_data(std::vector<int32_t> &v) {
    assert(v.size() >= nchannels_);
    std::copy(v.begin(), v.begin() + nchannels_, data_begin());
    finalized_ = false;
}

void NlxSignalRecord::set_data(std::vector<int32_t>::iterator it) {
    std::copy(it, it + nchannels_, data_begin());
    finalized_ = false;
}

void NlxSignalRecord::data(std::vector<double> &v) const {
    if (v.size() < nchannels_) {
        v.resize(nchannels_);
    }
    data(v.begin());
}
std::vector<double>::iterator
NlxSignalRecord::data(std::vector<double>::iterator it) const {
    auto first = data_begin();
    auto last = data_end();
    while (first != last) {
        *it = (double)*first * NLX_AD_BIT_MICROVOLTS;
        ++it;
        ++first;
    }
    return it;
}

double NlxSignalRecord::sample_microvolt(unsigned int index) const {
    return static_cast<double>(buffer_[NLX_FIELD_DATA_FIRST + index] *
                               NLX_AD_BIT_MICROVOLTS);
}

void NlxSignalRecord::set_data(double value) {
    int32_t v = static_cast<int32_t>(value / NLX_AD_BIT_MICROVOLTS);
    std::fill(data_begin(), data_end(), v);
    finalized_ = false;
}

void NlxSignalRecord::set_data(std::vector<double> &v) {
    assert(v.size() >= nchannels_);
    set_data(v.begin());
}

void NlxSignalRecord::set_data(std::vector<double>::iterator it) {
    auto first = data_begin();
    for (unsigned int c = 0; c < nchannels_; ++c) {
        *first = static_cast<int32_t>(*it / NLX_AD_BIT_MICROVOLTS);
        ++first;
        ++it;
    }
    finalized_ = false;
}

std::vector<int32_t>::iterator NlxSignalRecord::data_begin() {
    return buffer_.begin() + NLX_FIELD_DATA_FIRST;
}

std::vector<int32_t>::iterator NlxSignalRecord::data_end() {
    return buffer_.begin() + nlx_field_data_last_ + 1;
}

std::vector<int32_t>::const_iterator NlxSignalRecord::data_begin() const {
    return buffer_.begin() + NLX_FIELD_DATA_FIRST;
}

std::vector<int32_t>::const_iterator NlxSignalRecord::data_end() const {
    return buffer_.begin() + nlx_field_data_last_ + 1;
}

void NlxStatistics::clear() {
    n_invalid = 0;
    n_duplicated = 0;
    n_outoforder = 0;
    n_missed = 0;
    n_gaps = 0;
}

uint64_t nlx::CheckTimestamp(const NlxSignalRecord &rec,
                             uint64_t &last_timestamp, NlxStatistics &stats) {
    uint64_t timestamp = rec.timestamp();

    if (last_timestamp == nlx::INVALID_TIMESTAMP) {
        last_timestamp = timestamp;
    } else if (timestamp == last_timestamp) {
        ++stats.n_duplicated;
    } else if (timestamp < last_timestamp) {
        ++stats.n_outoforder;
    } else {
        uint64_t delta = timestamp - last_timestamp;
        if (delta > nlx::MAX_ALLOWABLE_TIMEGAP_MICROSECONDS) {
            int64_t n_missed = round(delta / nlx::SAMPLING_PERIOD_MICROSEC) - 1;
            stats.n_missed += n_missed;
            ++stats.n_gaps;
            // LOG(DEBUG) << n_missed << " timestamps were found to be missing.
            // ";
        }
        last_timestamp = timestamp;
    }

    return timestamp;
}
