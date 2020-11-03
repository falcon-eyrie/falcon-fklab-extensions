.. _Event2State:

Event2State
-----------
Arrival of target events set state to true, arrival of non-target events set state to false.
Target events are forwarded to output stream.

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
   * - **target event**
     - string
     - None
     - The target event that sets the state to true

.. tabularcolumns:: |p{4cm}|p{1cm}|p{3cm}|p{1.5cm}|p{1.3cm}|p{3cm}|

.. list-table:: **Broadcaster State**
   :header-rows: 1

   * - name
     - data type
     - initial value
     - external access
     - description
   * - **enabled**
     - bool
     - false
     - write
     - If target event is received, the state become true, if it is another event, it set back to false.
