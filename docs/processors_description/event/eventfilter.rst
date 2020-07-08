.. _EventFilter:

EventFilter
-----------
Process neural data

.. list-table:: **Input port**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **events**
     - :ref:`EventData`
     - 1-256
     -
   * - **blocking events**
     - :ref:`EventData`
     - 1-256
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
     - target event to be filtered out
   * - **block duration**
     - double
     - 10.0 ms
     - time during which target events are filtered out
   * - **sync time**
     - double
     - 1.5 ms
     - time used to check if any blocking target event is present after a target event has been received
   * - **block wait time**
     - double
     - 3.5 ms
     - time after blocking event during all arriving event are blocked
   * - **discard warnings**
     - bool
     - false
     - if true, warnings about discarded events will not be generated
   * - **detection criterion**
     - string or unsigned int
     - "any"
     - string or number to determine the criterion for a triggering detection; acceptable string values: 'any', 'all'
       acceptable integer values: any value between 1 (equivalent to 'any') and the number of input slots