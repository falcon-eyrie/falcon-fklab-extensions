DummySink
=========
Take any data stream and eats it. Mainly used to show and test basic graph processing functionality.

.. list-table:: **Input port**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **data**
     - IData
     - 1
     -

.. list-table:: **States**
   :header-rows: 1

   * - name
     - data type
     - initial value
     - external access
     - peers access
     - description
   * - **tickle**
     - bool
     - False
     - read-only
     - read/write
     - trigger a log message when changed :

       True = "Hi hi, that tickles!"

       False = "Why stop tickling?




