.. _MultiChannelFilter:

MultiChannelFilter
==================

.. list-table:: **Input ports**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **data**
     - :ref:`MultiChannelData` <double>
     - 1-256
     -

.. list-table:: **Output ports**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **data**
     - :ref:`MultiChannelData` <double>
     - 1-256
     -

.. tabularcolumns:: |p{2cm}|p{3cm}|p{3cm}|p{5.5cm}|
.. list-table:: **Options**
   :header-rows: 1

   * - name
     - data type
     - default
     - description
   * - **filter**
     - string OR definition structure
     - No default value - the definition of this parameter in the graph file is mandatory.
     - YAML filter definition or name of file that contains filter


Here are some example filter configurations:

.. code-block:: yaml

    filter:
        file: filters://butter_lpf_0.1fs.filter

    filter:
       type: biquad
       gain: 3.405376527201278e-04
       coefficients:
          - [1.0, 2.0, 1.0, 1.0, -1.032069405319709, 0.275707942472944]
          - [1.0, 2.0, 1.0, 1.0, -1.142980502539903, 0.412801598096190]
          - [1.0, 2.0, 1.0, 1.0, -1.404384890471583, 0.735915191196473]
       description: 6th order butterworth low pass filter with cutoff at 0.1 times the sampling frequency

    filter:
       type: fir
       description: 101 taps low pass filter with cutoff at 0.1 times the sampling frequency
       coefficients: [-6.24626469088e-19, -0.000309386982441, -0.000528204854007, ...]

