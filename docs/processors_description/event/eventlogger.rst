.. _EventLogger:

EventLogger
===========

Synchronize on the occurrence of a target event on all its input slots, before emitting the same target event


.. list-table:: **Input ports**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **events**
     - :ref:`EventData`
     - 1-256
     -

Options
.......

.. list-table:: **Options**
   :header-rows: 1

   * - port name
     - data type
     - default
     - description
   * - **target event**
     - string
     - none (default  value set in :ref:`EventData`)
     -
