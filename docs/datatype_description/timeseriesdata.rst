.. _timeseriesdata:

Time Series data
================

General description
-------------------
Data packet of the time series data type contains a generic nsamples-by-ncolumn array of data. It inherits from
the column data type and it replaced the multichannel data from previous falcon-fklab-extension version.

Each column has a label which by default will be the channel number. It has also a timestamps associated to the samples.


Payload details
---------------

.. list-table::
   :header-rows: 1

   * - name
     - type
     - description
   * - labels
     - vector of ncolumn strings
     - a string name for each column
   * - timestamp
     - uint64
     -
   * - signal
     - vector of any datatype
     -

API
---
.. doxygenclass:: nsTimeSeries::Data
   :members:
   :undoc-members:

Parameters
----------

.. list-table::
   :header-rows: 1

   * - name
     - type
     - description
     - validation
   * - nsamples
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

.. doxygenstruct:: nsTimeSeries::Parameters
   :members:
   :undoc-members:

Capabilities
------------

see :ref:columndata type capabilities

.. doxygentypedef:: nsTimeSeries::Capabilities
   :members:
   :undoc-members:

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
label: [label]
