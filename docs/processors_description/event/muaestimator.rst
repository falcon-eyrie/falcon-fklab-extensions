MUAEstimator
============
Compute the Multi-Unit Activity from the spike counts provided by the spike detectors and outputs MUAData.


.. list-table:: **Input port**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **spikes**
     - :ref:`SpikeData`
     - 1-64
     -

.. list-table:: **Output port**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **mua**
     - :ref:`MUAData`
     - 1
     -

.. list-table:: **Options**
   :header-rows: 1

   * - port name
     - data type
     - default
     - description
   * - **bin size**
     - unsigned int
     - 20 ms
     -


.. list-table:: **Static states**
   :header-rows: 1

   * - name
     - data type
     - initial value
     - external access
     - peers access
     - description
   * - **bin size**
     - unsigned int
     - options:bin size
     - read-only
     - read/write
     -

.. list-table:: **Broadcaster states**
   :header-rows: 1

   * - name
     - data type
     - initial value
     - external access
     - description
   * - **MUA**
     - double
     - 0.0
     - read-only
     -