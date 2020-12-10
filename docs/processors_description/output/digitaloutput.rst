DigitalOutput
=============
Take an EventData stream and sets digital outputs according to an event-specific protocol

.. list-table:: **Input ports**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **events**
     - :ref:`EventData`
     - 1
     -

.. tabularcolumns:: |p{3cm}|p{2cm}|p{3cm}|p{5.5cm}|

.. list-table:: **Options**
   :header-rows: 1

   * - name
     - data type
     - default
     - description
   * - **event logging**
     - bool
     - true
     -
   * - **pulse width**
     - unsigned int
     - 300 ms
     - duration of digital output pulse in microseconds
   * - **device/type**
     - string
     - "dummy"
     - dummy is the only type implemented yet.
   * - **device/nchannels**
     - unsigned int
     - 16
     - Number of digital channel on the device.
   * - **protocols**
     - yaml node
     - no default type value - Always specify in the graph config file.
     - protocol map (see below)



The protocols option specifies a map with  for each target event a map of actions for selected digital output channels.
Note that each channel can only be associated with a single action (even if it is listed more than once).
There are 4 possible actions: high, low, toggle and pulse. Events that are not in the protocols map are ignored.
Example configuration for protocols option:

.. code-block::

      protocols:
        event_a:
          high: [0,1]
        event_b:
          low: [0]
          toggle: [1]
        event_c:
          pulse: [2]
