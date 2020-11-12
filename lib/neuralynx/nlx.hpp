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

#include <netinet/in.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
#include <limits>

namespace nlx {

// a digilynx raw packet has the following layout
//  int32            | start of transmission identifier == 2048
//  int32            | packet type identifier == 1
//  int32            | packet size == #channels + #extra
// uint32            | timestamp (high order bits)
// uint32            | timestamp (low order bits)
//  int32            | status (reserved)
// uint32            | parallel input port value
//  int32[10]        | extra values (reserved)
//  int32[nchannels] | data values (nchannels == number of channels in
//  acquisition system) int32            | CRC value for record

// some values are needed at compile time
constexpr uint16_t NLX_NFIELDS_EXTRA = 10;
constexpr uint16_t NLX_FIELDBYTESIZE = 4;
constexpr uint16_t NLX_NFIELDS(uint16_t c) {
  return (8 + NLX_NFIELDS_EXTRA + (c));
}
constexpr uint16_t NLX_PACKETBYTESIZE(uint16_t c) {
  return (NLX_FIELDBYTESIZE * NLX_NFIELDS(c));
}
constexpr uint16_t NLX_NCHANNELS_FROM_PACKETBYTESIZE(uint16_t sz) {
  return ((sz / NLX_FIELDBYTESIZE) - 8 - NLX_NFIELDS_EXTRA);
}
constexpr uint16_t NLX_NCHANNELS_FROM_NFIELDS(uint16_t n) {
  return n - 8 - NLX_NFIELDS_EXTRA;
}

constexpr uint16_t SUCCESS_READING_BUFFER = 0;
constexpr uint16_t ERROR_NLX_FIELD_STX = 2;
constexpr uint16_t ERROR_NLX_FIELD_RAWPACKETID = 3;
constexpr uint16_t ERROR_NLX_FIELD_PACKETSIZE = 5;
constexpr uint16_t ERROR_TOO_SMALL_PACKET = 4;
constexpr uint16_t ERROR_BAD_CRC = 1;


constexpr uint16_t NLX_FIELD_STX = 0;
constexpr uint16_t NLX_FIELD_RAWPACKETID = 1;
constexpr uint16_t NLX_FIELD_PACKETSIZE = 2;
constexpr uint16_t NLX_FIELD_TIMESTAMP_HIGH = 3;
constexpr uint16_t NLX_FIELD_TIMESTAMP_LOW = 4;
constexpr uint16_t NLX_FIELD_DIO = 6;
constexpr uint16_t NLX_FIELD_EXTRA_FIRST = 7;
constexpr uint16_t NLX_FIELD_DATA_FIRST =
    NLX_FIELD_EXTRA_FIRST + NLX_NFIELDS_EXTRA;

constexpr double NLX_AD_BIT_MICROVOLTS = 0.015624999960550667;
constexpr int32_t NLX_STX = 2048;
constexpr int32_t NLX_RAWPACKETID = 1;
constexpr uint16_t NLX_DEFAULT_NCHANNELS = 128;
constexpr uint16_t NLX_MAX_NCHANNELS = 1024;
constexpr double NLX_SIGNAL_SAMPLING_FREQUENCY = 32000;
constexpr unsigned int NLX_DEFAULT_BUFFERSIZE =
    NLX_PACKETBYTESIZE(NLX_DEFAULT_NCHANNELS);

const double NLX_VIDEO_SAMPLING_FREQUENCY = 25;
const int VTRecNumTransitionBitfields =
    400;  ///< Number of VT bitfield transitions stored in the VideoRec::dwPoints
          ///< array
const int VTRecNumTargets = 50;  ///< Number of VT bitfield transitions stored in
                                 ///< the VideoRec::dntargets array
const std::uint16_t VTRecSWST =
    0x800;  ///< Value always used for VideoRec::swstx

typedef struct {       // from Neuralynx header Nlx_DataTypes.h
  std::uint16_t swstx;  ///< Value is always NlxDataTypes::NLX_VTREC_SWSTX
  std::uint16_t
      swid;  ///< The ID assigned to the video tracker that created this record
  std::uint16_t swdata_size;  ///< The size of the VT record in bytes
  std::uint64_t qwTimeStamp;  ///< Timestamp of this record in microseconds
  std::uint32_t
      dwPoints[VTRecNumTransitionBitfields];  ///< An array of bitfields encoding
                                              ///< all threshold crossings in the
                                              ///< video frame.
  std::int16_t sncrc;  ///< Ignored, relic from Cheetah160VT
  std::int32_t
      dnextracted_x;  ///< Calculated x coordinate from our extraction algorithm
  std::int32_t
      dnextracted_y;  ///< Calculated y coordinate from our extraction algorithm
  std::int32_t dnextracted_angle;  ///< Calculated head direction in degrees from
                                  ///< the Y axis
  std::int32_t dntargets[VTRecNumTargets];  ///< An array of aggregated
                                            ///< transitions in the same bitfield
                                            ///< format as VideoRec::dwPoints
} __attribute__((__packed__))
VideoRec;  // packing does not hurt perfomance on x64 CPUs
// used here in order to guarantee sizeof(VideoRec) == swdata_size so that is
// read correctly (otherwise it is NOT!) read more on:
// https://attractivechaos.wordpress.com/2013/05/02/does-packed-struct-hurt-performance-on-x86_64/

struct ErrorNLXVT {   // wrapped to avoid namespace conflict
  enum Code {
    UNKNOWN = -1,
    NO_ERROR = 0,
    SWSTX,
    SWID,
    SWDATA_SIZE,
    NEGATIVE_COORDINATE,
    OUT_OF_RESOLUTION
  };
};

const std::array<std::int32_t, 2> NLX_VIDEO_RESOLUTION = {{720, 576}};

bool valid_nlx_vt(VideoRec *vt_record, std::uint16_t vt_id,
                  ErrorNLXVT::Code &error_code,
                  decltype(NLX_VIDEO_RESOLUTION) resolution);

inline const unsigned int
nlx_field_data_last(unsigned int nchannels = NLX_DEFAULT_NCHANNELS) {
  return NLX_FIELD_DATA_FIRST + nchannels - 1;
}
inline const unsigned int
nlx_field_crc(unsigned int nchannels = NLX_DEFAULT_NCHANNELS) {
  return NLX_FIELD_DATA_FIRST + nchannels;
}
inline const int32_t
nlx_packetsize(unsigned int nchannels = NLX_DEFAULT_NCHANNELS) {
  return nchannels + NLX_NFIELDS_EXTRA;
}

class NlxSignalRecord {
 public:
  NlxSignalRecord(unsigned int nchannels = NLX_DEFAULT_NCHANNELS,
                  bool convert_byte_order = true);

  unsigned int nchannels() const;
  void set_nchannels(unsigned int n);

  bool convert_byte_order() const;
  void set_convert_byte_order(bool b);

  int FromNetworkBuffer(const char *buffer, size_t n);

  template <typename T> int FromNetworkBuffer(const std::vector<T> &buffer) {
    return FromNetworkBuffer((char *)buffer.data(), buffer.size() * sizeof(T));
  }

  size_t ToNetworkBuffer(char *buffer, size_t n);

  template <typename T> size_t ToNetworkBuffer(std::vector<T> &buffer) {
    // check size
    if (buffer.size() < (nlx_packetbytesize_ / sizeof(T))) {
      buffer.resize(nlx_packetbytesize_ / sizeof(T));
    }

    return ToNetworkBuffer((char *)buffer.data(), buffer.size() * sizeof(T));
  }

  void Initialize();  // set required fields 1-3
  void Finalize();   // compute CRC

  int32_t crc() const;

  bool initialized() const;
  bool finalized() const;

  int valid(std::vector<int32_t> buffer);

  // timestamp access functions
  uint64_t timestamp() const;
  void set_timestamp(uint64_t t);
  void inc_timestamp(uint64_t delta);
  void inc_timestamp(double delta);

  // digital input/output access methods
  uint32_t parallel_port() const;
  void set_parallel_port(uint32_t dio = 0);

  // data (int32) getter methods
  void data(std::vector<int32_t> &v) const;  // will copy
  std::vector<int32_t>::iterator
  data(std::vector<int32_t>::iterator it) const;  // will copy
  int32_t sample(unsigned int index) const;

  // data (int32) setter methods
  void set_data(int32_t value = 0);
  void set_data(std::vector<int32_t> &v);
  void set_data(std::vector<int32_t>::iterator it);

  // data (microVolt) getter methods
  void data(std::vector<double> &v) const;  // will convert to uV and copy
  std::vector<double>::iterator data(std::vector<double>::iterator it) const;
  double sample_microvolt(unsigned int index) const;

  // data (microVolt) setter methods
  void set_data(double value = 0);
  void set_data(std::vector<double> &v);
  void set_data(std::vector<double>::iterator it);

  std::vector<int32_t> buffer_;
  int32_t nlx_packetsize_;
protected:
  std::vector<int32_t>::iterator data_begin();
  std::vector<int32_t>::iterator data_end();

  std::vector<int32_t>::const_iterator data_begin() const;
  std::vector<int32_t>::const_iterator data_end() const;

  bool convert_byte_order_;
  unsigned int nchannels_;
  bool initialized_ = false;
  bool finalized_ = false;

  unsigned int nlx_nfields_;
  unsigned int nlx_packetbytesize_;
  uint16_t nlx_field_crc_;
  uint16_t nlx_field_data_last_;
};

// timestamp related constants
// sampling period (microseconds)
constexpr decltype(NLX_SIGNAL_SAMPLING_FREQUENCY) SAMPLING_PERIOD_MICROSEC =
    1e6 / NLX_SIGNAL_SAMPLING_FREQUENCY;
// maximum tolerated difference between two timestamps that
// is not considered a gap with missing data packets
const uint64_t MAX_ALLOWABLE_TIMEGAP_MICROSECONDS =
    trunc(SAMPLING_PERIOD_MICROSEC) + 1;
// value for an invalid timestamp
constexpr uint64_t INVALID_TIMESTAMP = std::numeric_limits<uint64_t>::max();

class NlxStatistics {
 public:
  NlxStatistics()
      : n_invalid(0), n_duplicated(0), n_outoforder(0), n_missed(0), n_gaps(0) {
  }

 public:
  uint64_t n_invalid;
  uint64_t n_duplicated;
  uint64_t n_outoforder;
  uint64_t n_missed;
  uint64_t n_gaps;

  void clear();
};

uint64_t CheckTimestamp(const NlxSignalRecord &rec, uint64_t &last_timestamp,
                        NlxStatistics &stats);
}  // namespace nlx
