.. _columndata:

Column Data
===========

General description
-------------------
Data packet of the column data type contains a generic nsamples-by-ncolumns array of data when each column as a label
associated.


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
   * - signal
     - vector of any datatype
     -



API
---
.. doxygenclass:: nsColumn::Data
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

.. doxygenstruct:: nsColumn::Parameters
   :members:
   :undoc-members:

Capabilities
------------

.. list-table::
   :header-rows: 1

   * - name
     - type
     - default
     - description
   * - column range
     - ChannelRange or Range<unsigned int>
     - (none)
     - the number of channels that is supported
   * - sample range
     - SampleRange or Range<size_t>
     - [1, (maximum of the datatype can hold)]
     - the number of samples that is supported

OR

.. list-table::
   :header-rows: 1

   * - name
     - type
     - default
     - description
   * - labels
     - vector of string
     - (none)
     - name of all channels supported in input
   * - sample range
     - SampleRange or Range<size_t>
     - [1, (maximum of the datatype can hold)]
     - the number of samples that is supported


.. doxygenclass:: nsColumn::Capabilities
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

label: [label]
signal: [data]