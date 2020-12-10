SerialOutput
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
     - 1-10
     -

.. tabularcolumns:: |p{3cm}|p{2cm}|p{3cm}|p{5.5cm}|

.. list-table:: **Options**
   :header-rows: 1

   * - name
     - data type
     - default
     - description
   * - **port address**
     - string
     - "/dev/ttyACM0"
     - address of serial port
   * - **baud rate**
     - double
     - 9600
     - serial rate exchange
   * - **event logging**
     - bool
     - true
     -

