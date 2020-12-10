.. _nxlpurereader:

NlxPureReader
=============

Read raw data from a Neuralynx Digilynx data acquisition system and turns it into multiple MultiChannelData output streams
based on a channel mapping

.. list-table:: **Output ports**
   :header-rows: 1

   * - port name
     - data type
     - slots
     - description
   * - **udp**
     - :ref:`VectorData` <char>
     - 1
     -


.. tabularcolumns:: |p{4cm}|p{3cm}|p{2.5cm}|p{5.5cm}|

.. list-table:: **Options**
   :header-rows: 1

   * - name
     - data type
     - default
     - description
   * - **address**
     - string
     - "127.0.0.1"
     - IP address of Digilynx system
   * - **port**
     - unsigned int
     - 5000
     - port of Digilynx system
   * - **npackets**
     - uint64_t
     - 0
     - number of raw data packets to read before exiting (0 = continuous streaming)
   * - **nchannels**
     - unsigned int
     - 128
     - number of channels in Digilynx system


.. tabularcolumns:: |p{4cm}|p{1cm}|p{3cm}|p{1.5cm}|p{1.3cm}|p{3cm}|

.. list-table:: **Broadcaster States**
   :header-rows: 1

   * - name
     - data type
     - initial value
     - external access
     - peers access
     - description
   * - **n_invalid**
     - uint64_t
     - 0
     - read
     - read
     - The number of invalid packets that were received.
