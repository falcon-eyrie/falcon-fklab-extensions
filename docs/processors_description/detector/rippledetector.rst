.. _RippleDetector:

RippleDetector
==============

Detect ripples in a MultiChannelData stream and emits a ripple event in response

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
     - A stream of 'ripple' events.
   * - **statistics**
     - :ref:`MultiChannelData` <double>
     - 1
     -

.. tabularcolumns:: |p{5cm}|p{1.2cm}|p{1cm}|p{5.5cm}|

.. list-table:: **Options**
   :header-rows: 1

   * - port name
     - data type
     - default
     - description
   * - **threshold dev**
     - double
     - 6.0
     - threshold that needs to be crossed
   * - **smooth time**
     - double
     - 10.0 seconds
     - integration time for signal statistics. Must be a positive number.
   * - **detection lockout time**
     - double
     - 30 ms
     - refractory period after threshold crossing detection that is not considered for updating of statistics
       and for detecting events. Must greater than 0 ms.
   * - **statistics buffer size**
     - double
     - 0.5 sec
     - Buffer size (in seconds) for statistics output buffers. Should be equal larger than zero.
   * - **statistics downsample factor**
     - unsigned int
     - 1
     - downsample factor of streamed statistics signal. Should larger than zero..
   * - **stream events**
     - bool
     - True
     - enable/disable ripple event output
   * - **stream statistics**
     - bool
     - True
     - enable/disable streaming of ripple detection statistics
   * - **use power**
     - bool
     - True
     -

.. tabularcolumns:: |p{4cm}|p{1cm}|p{3cm}|p{1.5cm}|p{1.3cm}|p{3cm}|
.. list-table:: **Producer states**
   :header-rows: 1

   * - name
     - data type
     - initial value
     - external access
     - peers access
     - description
   * - **threshold**
     - double
     - 0.0
     - None
     - read-only
     - Current threshold that needs to be crossed
   * - **mean**
     - double
     - 0.0
     - None
     - read-only
     - Current signal mean. Units: same as input signal.
   * - **deviation**
     - double
     - 0.0
     - None
     - read-only
     - Current signal deviation. Units: same as input signal.

.. tabularcolumns:: |p{4cm}|p{1cm}|p{3cm}|p{1.5cm}|p{1.3cm}|p{3cm}|
.. list-table:: **Broadcaster states**
   :header-rows: 1

   * - name
     - data type
     - initial value
     - external access
     - peers access
     - description
   * - **ripple**
     - bool
     - False
     - read-only
     - read-only
     -

.. tabularcolumns:: |p{4cm}|p{1cm}|p{3cm}|p{1.5cm}|p{1.3cm}|p{3cm}|
.. list-table:: **Static states**
   :header-rows: 1

   * - name
     - data type
     - initial value
     - external access
     - peers access
     - description
   * - **threshold dev**
     - double
     - option:threshold dev
     - read-only
     - read/write
     -
   * - **smooth time**
     - double
     - option:smooth time
     - read-only
     - read/write
     - integration time for signal statistics. Must be a positive number.
   * - **detection lockout time**
     - double
     - option: detection lockout time
     - read-only
     - read/write
     - Current refractory period following threshold crossing that is not
       considered for  updating signal statistics and for event detection.
   * - **stream events**
     - bool
     - option:stream events
     - read-only
     - read/write
     -
   * - **stream statistics**
     - bool
     - option:stream statistics
     - read-only
     - read/write
     -