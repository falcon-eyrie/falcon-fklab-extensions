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
   * - **enabled**
     - bool
     - enabled state
     - enable/disable digital output
   * - **lockout period**
     - unsigned int
     - 300 ms
     - set the maximal stimulation frequency. If equal to zero, it disabled the feature.
   * - **pulse width**
     - unsigned int
     - 300 ms
     - duration of digital output pulse in microseconds
   * - **enable saving**
     - bool
     - enabled state
     - enable/disable saving stimulation events
   * - **device**
     - unsigned int
     - no default type value - Always specify in the graph config file.
     - map specifying the digital output device. A required "type" key indicates which device should be used.
       Valid values are "dummy". The dummy device requires an additional "nchannels" key.
   * - **print protocol execution updates**
     -
     -
     - maps events to digital output protocols.


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

.. tabularcolumns:: |p{3cm}|p{1cm}|p{4cm}|p{1.5cm}|p{1.8cm}|p{2cm}|
.. list-table:: **Static states**
   :header-rows: 1

   * - name
     - data type
     - initial value
     - external access
     - peers access
     - description
   * - **enabled**
     - bool
     - option: enabled
     - write/read
     - read-only
     -
   * - **lockout period**
     - bool
     - option: lockout period
     - write/read
     - read-only
     -

