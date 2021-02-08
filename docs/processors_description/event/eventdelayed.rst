.. _EventDelayed:

EventDelayed
------------
Transfer event ontime or with a delay randomly chosen in a range. There is also a lockout period to remove
too close events and an option file logging of these events.

There is three different states:

- Enabled state -> enabled= true = Detection event is sent but no ontime or delayed event is sent.
- Ontime state  -> delayed=false = Event are sent immediately with the ontime message
- Delayed state -> delayed= true = Event are sent immediately as it arrived with the detection message and another event when the delay time is reach with the delayed message.

This 3 messages can be personalized in the options.

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
   * - **detection only mode**
     - bool
     - false
     - Disabled the processing of incoming events.
   * - **analysis lockout period**
     - double
     - 50.0 ms
     - Disable time for ripple detection after stimulation
   * - **event trigger lockout period**
     - double
     - 50.0 ms
     - Disable time for sending trigger event after stimulation
   * - **delayed mode**
     - bool
     - false
     - Enable the delay of the event for a time randomly chosen between the delay range
   * - **delay range**
     - long int
     - [150, 250] ms
     -  if delayed event is true, the delayed time will be pseudo-randomly chosen in this range.
   * - **message/detection**
     - string
     - "r"
     - Delayed/Disable mode - message to send when an event is received.
   * - **message/delayed**
     - string
     - "d"
     - Delayed mode - message to send when the event has been delayed.
   * - **message/ontime**
     - string
     - "o"
     - Delayed mode - message to send when an event is received.



.. tabularcolumns:: |p{4cm}|p{1cm}|p{3cm}|p{1.5cm}|p{1.3cm}|p{3cm}|

.. list-table:: **Static State**
   :header-rows: 1

   * - name
     - data type
     - initial value
     - external access
     - peers access
     - description
   * - **analysis lockout time**
     - double
     - option: analysis lockout time
     - read-only
     - read/write
     -
    * - **event trigger lockout time**
     - double
     - option: event trigger lockout time
     - read-only
     - read/write
     -

.. tabularcolumns:: |p{4cm}|p{1cm}|p{3cm}|p{1.5cm}|p{1.3cm}|p{3cm}|

.. list-table:: **Follower States**
   :header-rows: 1

   * - name
     - data type
     - initial value
     - external access
     - peers access
     - description
   * - **delayed mode**
     - bool
     - option: delayed mode
     - read/write
     - read/write
     -
   * - **detection only mode**
     - bool
     - option: disabled
     - read/write
     - read/write
     -