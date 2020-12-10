.. _EventConverter:

EventConverter
--------------
Convert incoming target event into another event.

.. list-table:: **Input port**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **events**
     - :ref:`EventData`
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
     -

.. tabularcolumns:: |p{3cm}|p{3cm}|p{1cm}|p{7.5cm}|

.. list-table:: **Options**
   :header-rows: 1

   * - port name
     - data type
     - default
     - description
   * - **disabled**
     - bool
     - false
     - Name for generating the event
   * - **event name**
     - string
     - "o"
     - Lock out time after the processing of an event.
   * - **replace**
     - bool
     - true
     - Either replace incoming events with new event name, or generate output events by appending incoming event and new event name.
