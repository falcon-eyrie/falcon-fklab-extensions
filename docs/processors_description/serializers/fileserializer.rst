.. _FileSerializer:

FileSerializer
==============

Serializes data streams to file

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

.. tabularcolumns:: |p{3cm}|p{1.5cm}|p{2cm}|p{7cm}|

.. list-table:: **Options**
   :header-rows: 1

   * - port name
     - data type
     - default
     - description
   * - **path**
     - string
     - "run://"
     - server-side path
   * - **encoding**
     - string
     - "binary"
     - Only two acceptable keyword: 'binary' or 'yaml'
   * - **format**
     - string
     - "full"
     - Only tree acceptable keyword: 'full', 'nodata', 'compact' (see serializer.hpp for more informations on this mode)
   * - **overwrite**
     - bool
     - False
     - overwrite existing file
   * - **throttle/enabled**
     - bool
     - False
     - throttle saving if we can't keep up
   * - **throttle/threshold**
     - double
     - 0.3
     - upstream ringbuffer fill fraction (between 0-1) at which throttling takes effect
   * - **throttle/smooth**
     - double
     - 0.5
     - smooth level of throttle level (between 0-1)
