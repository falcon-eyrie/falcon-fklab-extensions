Rebuffer
========

Rebuffer and downsample multiple MultiChannelData streams. No anti-aliasing filter is applied before downsampling.


.. list-table:: **Input port**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **data**
     - :ref:`MultiChannelData` <double>
     - 1-256
     -

.. list-table:: **Output port**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **data**
     - :ref:`MultiChannelData` <double>
     - 1-256
     -

.. list-table:: **Options**
   :header-rows: 1

   * - port name
     - data type
     - default
     - description
   * - **downsample factor**
     - unsigned int
     - 1
     -
   * - **buffer size**
     - unsigned int
     - 10 samples or equivalent in second based on the downsample factor depending of the buffer unit.
     - Output buffer size in samples or seconds.
