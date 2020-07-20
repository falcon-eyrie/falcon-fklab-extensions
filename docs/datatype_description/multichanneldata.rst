.. _multichanneldata:

MultiChannelData
================

General description
-------------------
Data packet of the MultiChannelData type contains a generic nsamples-by-nchannels array of data.


Payload details
---------------

.. list-table::
   :header-rows: 1

   * - name
     - type
     - description
   * - timestamp
     - uint64
     -
   * - signal
     - vector of any datatype
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
   * - nsample
     - unsigned int
     - 0
     - Number of samples cannot be zero and needs to be in range
   * - nchannels
     - unsigned int
     - 0
     - Number of channels cannot be zero and needs to be in range
   * - sample_rate
     - double
     - 1.0
     - Sample rate needs to be larger than 0

Capabilities
------------

.. list-table::
   :header-rows: 1

   * - name
     - type
     - default
     - description
   * - channel range
     - ChannelRange or Range<unsigned int>
     - (none)
     - the number of channels that is supported
   * - sample range
     - SampleRange or Range<size_t>
     - [1, (maximum of the datatype can hold)]
     - the number of samples that is supported


Binary Serialization
--------------------
For serialization formats *FULL* and *COMPACT*,
The timestamps and the data are serialized.

YAML Serialization
------------------
For serialization formats *FULL* and *COMPACT*,

the following YAML is emitted:

timestamps: [timestamps (unsigned int)]
signal: [data]