LevelCrossingDetector
=====================

Detect a threshold crossing on any of the channels in the incoming MultiChannelData stream and emits an event in response

.. image:: ../../images/LevelCrossingDetector.png
   :align: center

.. list-table:: **Input port**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **data**
     - :ref:`MultiChannelData` <double>
     - 1
     -

.. list-table:: **Output port**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **events**
     - :ref:`EventData`
     - 1
     - A stream of events.

.. tabularcolumns:: |p{3cm}|p{1.2cm}|p{3cm}|p{5.5cm}|

.. list-table:: **Options**
   :header-rows: 1

   * - port name
     - data type
     - default
     - description
   * - **threshold**
     - double
     - 0.0
     - threshold that needs to be crossed
   * - **event**
     - string
     - "threshold_crossing"
     - event to emit upon detection of threshold crossing
   * - **post detect block**
     - unsigned int
     - 2
     - refractory period after threshold crossing detection (in number of samples )
   * - **upslope**
     - bool
     - True
     - whether to look for upward (true) or downward (false) threshold crossings

.. tabularcolumns:: |p{4cm}|p{1cm}|p{3cm}|p{1.5cm}|p{1.3cm}|p{3cm}|

.. list-table:: **Static states**
   :header-rows: 1

   * - name
     - data type
     - initial value
     - external access
     - peers access
     - description
   * - **threshold**
     - double
     - option: threshold
     - read-only
     - write/read
     - Current threshold that needs to be crossed
   * - **post detect block**
     - unsigned int
     - option: post detect block
     - read-only
     - write/read
     -
   * - **upslope**
     - bool
     - option: upslope
     - read-only
     - write/read
     -