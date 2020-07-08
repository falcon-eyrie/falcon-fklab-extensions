.. _ZMQSerializer:

ZMQSerializer
=============

Serializes data streams to cloud

.. list-table:: **Input ports**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **data**
     - IData
     - 1-256
     -

.. tabularcolumns:: |p{2cm}|p{2cm}|p{2cm}|p{7.5cm}|

.. list-table:: **Options**
   :header-rows: 1

   * - port name
     - data type
     - default
     - description
   * - **port**
     - unsigned int
     - 7777
     -
   * - **encoding**
     - string
     - "binary"
     - Only two acceptable keyword: 'binary' or 'yaml'
   * - **format**
     - string
     - "full"
     - Only tree acceptable keyword: 'full', 'nodata', 'compact' (see serializer.hpp for more informations on this mode)
   * - **interleave**
     - bool
     - false
     - whether data streams from different input slots are interleaved