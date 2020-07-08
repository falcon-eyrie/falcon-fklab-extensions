.. _MUAData:

MUAData
=======

General description
-------------------
Data packet of the MUAData type describes the multi-unit activity by the frequency of spikes in a bin.


Payload details
---------------

.. list-table::
   :header-rows: 1

   * - name
     - type
     - description
   * - mua
     - double
     - number of spikes / size of the bin * 1e3

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
   * - bin_size
     - double (ms)
     - Input parameter with default value = 0
     - needs to be a positive value (>0)
   * - n_spikes
     - double
     - Set separately after initialization of the datatype
     -

Capabilities
------------
(none)

Binary Serialization
--------------------
For serialization formats *FULL* and *COMPACT*,
if compact format, only the mua data as being the number of spike / size of the bin * 1e3 is serialized.
if full format, the number of spikes and the bin size are also sent.

YAML Serialization
------------------
For serialization formats *FULL* and *COMPACT*,

if compact format:
the following YAML is emitted:

MUA: [number of spikes / size of the bin * 1e3]

if full format:
the following YAML is emitted:

MUA: [number of spikes / size of the bin * 1e3]
n_spikes: [number of spikes (double)]
bin_size: [bin size (double)]