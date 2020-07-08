EventSource
===========
Generate an EventData stream by randomly emitting events from a list of candidates at a predefined rate


.. list-table:: **Output ports**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **events**
     - :ref:`EventData`
     - 1
     -

.. list-table:: **Options**
   :header-rows: 1

   * - port name
     - data type
     - default
     - description
   * - **events**
     - list of string
     - "default_eventsource_event"
     - list of events to emit
   * - **rate**
     - double
     - 1Hz
     - (approximate) event rate
