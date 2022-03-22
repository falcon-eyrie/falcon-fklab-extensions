.. _scalardata:

ScalarData
==========

General description
-------------------
Data packet of the ScalarData type contains a generic unique value. The value can be modified by a value of the same type.

Payload details
---------------

.. list-table::
   :header-rows: 1

   * - name
     - type
     - description
   * - data
     - any type
     - scalar value

API
---

.. doxygenclass:: nsScalarType::Data
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
   * - default_value
     - any type
     -
     -


.. doxygenstruct:: nsScalarType::Parameters
   :members:
   :undoc-members:


Capabilities
------------

.. doxygenclass:: nsScalarType::Capabilities
   :members:
   :undoc-members:


Binary Serialization
--------------------
For serialization formats *FULL* and *COMPACT*, the data variable is sent as a string.

YAML Serialization
------------------
For serialization formats *FULL* and *COMPACT*,

the following YAML is emitted:

scalar_data [data]