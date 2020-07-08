.. _spikedata:

SpikeData
=========

General description
-------------------
Data packets of the SpikeData type hold a list of the timestamp and the amplitude of each spike.
The list has a maximum size, specified at initialization time, which cannot be overreached.

Payload details
---------------

.. list-table::
   :header-rows: 1

   * - name
     - type
     - description
   * - n_detected_spikes
     - unsigned int
     - the number of spikes added in the datatype
   * - hw_ts_detected_spikes
     - vector of unsigned int64
     - hardware timestamp of each spike
   * - zero_timestamps
     - vector of 100 (maximum of spikes in the buffer) unsigned int64
     -
   * - amplitudes
     - vector of double
     -
   * - zero_amplitudes
     - vector of 1600 (maximum of spikes in the buffer x maximum number of channels) double
     -

API
---

TODO

Parameters
----------

.. list-table::
   :header-rows: 1

   * - name
     - type
     - description
     - validation
   * - nchannels
     - unsigned int
     - number of channels
     - needs to be larger than 0
   * - sample_rate
     - double
     -
     - needs to be larger than 0
   * - max_nspikes
     - size_t
     - buffer size
     - needs to be larger than 0


Capabilities
------------
(none)

Binary Serialization
--------------------
For serialization format *FULL*, the number of detected spikes, the hardware, zero timestamps, amplitudes of each spikes
are serialized in a string format.
For serialization format *COMPACT*, only the number of detected spikes and the amplitudes of each spikes
are serialized in a string format.

YAML Serialization
------------------
For serialization formats *FULL* and *COMPACT*,
the following YAML is emitted:

n_channels [channel number unsigned int]
n_detected_spikes [ number of spikes unsigned int]

if the previous one is superior to zero :
ts_detected_spikes [hardware timestamps]
spike_amplitudes [amplitudes]