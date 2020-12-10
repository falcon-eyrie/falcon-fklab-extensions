.. _distributor:

Distributor
===========

Read multi-channel data stream and splits the data across multiple output streams based on a channel mapping.

.. list-table:: **Input ports**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **data**
     - :ref:`MultiChannelData` <double>
     - 1
     - Received data from the NlxParser

.. list-table:: **Output ports**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **(configurable) ports name from channelmap options**
     - :ref:`MultiChannelData` <double>
     - 1
     - create an output port for each channel in the channelmap

The channelmap defines the output port names and for each port lists the AD channels that will be copied to the
MultiChannelData buckets on that port. The channelmap option should be specified as follows:

.. code-block:: yaml

    channelmap:
      portnameA: [0,1,2,3,4]
      portnameB: [5,6]
      portnameC: [0,5]

.. tabularcolumns:: |p{4cm}|p{3cm}|p{2.5cm}|p{5.5cm}|

.. list-table:: **Options**
   :header-rows: 1

   * - name
     - data type
     - default
     - description
   * - **channelmap**
     - map of vector<int>
     - No default value
     - mapping between AD channels and output ports