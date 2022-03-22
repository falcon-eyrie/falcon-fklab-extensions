
.. _vectordata:

VectorData
==========

General description
-------------------
Data packet of the VectorData type contains a generic vector.
The data are set separately, once the vector data has been created by allocating at initialization time in memory
the right size of data needed.

Payload details
---------------

.. list-table::
   :header-rows: 1

   * - name
     - type
     - description
   * - data
     - vector of any type
     -

API
---


.. doxygenclass:: nsVectorType::Data
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
   * - size
     - unsigned int
     - size of the vector
     - cannot be zero


.. doxygenstruct:: nsVectorType::Parameters
   :members:
   :undoc-members:


Capabilities
------------


.. doxygenclass:: nsVectorType::Capabilities
   :members:
   :undoc-members:


Binary Serialization
--------------------
None

YAML Serialization
------------------
None