.. _nxlparser:

NlxParser
=========

Read raw Neuralynx Digilynx data packets from a NlxPureReader processor and turns it a single MultiChannelData output
stream of all available channels. Optionally, multiple data packets are combined into batches.

.. list-table:: **Input ports**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **upd**
     - :ref:`VectorData` <char>
     - 1
     -

.. list-table:: **Output ports**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **data**
     - :ref:`MultiChannelData` <double>
     - 1
     - create an output port for each channel in the channelmap
   * - **ttl**
     - :ref:`MultiChannelData` <uint32>
     - 1
     -


.. tabularcolumns:: |p{4cm}|p{3cm}|p{2.5cm}|p{5.5cm}|

.. list-table:: **Options**
   :header-rows: 1

   * - name
     - data type
     - default
     - description
   * - **batch size**
     - unsigned int
     - 2
     - The number of data packets to concatenate into single multi-channel data bucket.
   * - **update interval**
     - unsigned int
     - 20
     - time interval (in seconds) between log updates
   * - **trigger/enable**
     - bool
     - False
     - Whether or not to wait for hardware trigger to start streaming data packets.
   * - **trigger/channel**
     - uint32_t
     - 0
     - Digital input channel to use as hardware trigger
   * - gap fill
     - string
     - asap
     - Method of filling in missing data packets.
       If 'none', no filling of " missed packets is performed.
       If 'asap', all missed packets will be filled with last available batch of samples.
       If 'distributed', missed packets will be filled with the last available batch of samples at each iteration.