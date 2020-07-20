RunningStats
============

Compute running statistics


.. list-table:: **Input port**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **data**
     - :ref:`MultiChannelData` <double>
     - 1-256
     -

.. list-table:: **Output port**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **data**
     - :ref:`MultiChannelData` <double>
     - 1-256
     -

.. tabularcolumns:: |p{3cm}|p{1cm}|p{1cm}|p{8.5cm}|

.. list-table:: **Options**
   :header-rows: 1

   * - port name
     - data type
     - default
     - description
   * - **integration time**
     - double
     - 1.0
     - time window for exponential smoothing
   * - **outlier/protection**
     - bool
     - False
     - enable outlier protection. Outliers are values larger than a predefined z-score.
       The contribution of an outlier is reduced by an amount that depends on the magnitude of the outlier
   * - **outlier/zscore**
     - double
     - 6.0
     - z-score that defines an outlier
   * - **outlier/half life**
     - double
     - 2.0
     - the number of standard deviations above the outlier z-score at which the influence of the outlier is halved.


