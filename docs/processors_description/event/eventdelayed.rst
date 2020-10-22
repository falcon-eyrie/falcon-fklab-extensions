.. _EventDelayed:

EventDelayed
------------
Transfer event with ontime or with a delay randomly chosen in a range. There is also a lockout period to remove
too close events and an option file logging of these events.

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
   * - **enabled**
     - bool
     - true
     - Enable the processing of incoming events.
   * - **lockout period**
     - double
     - 50.0 ms
     -  Lock out time after the processing of an event.
   * - **delayed**
     - bool
     - false
     -  Enable the delay of the event for a time randomly chosen between the delay range
   * - **delay range**
     - long int
     - [150, 250] ms
     -  if delayed event is true, the delayed time will be pseudo-randomly chosen in this range.
   * - **enabled saving**
     - bool
     - true
     - Enable saving of target events to disk.
   * - **prefix**
     - string
     - "stim_"
     - add a prefix in the filename (prefix+eventname) where target event are saved
