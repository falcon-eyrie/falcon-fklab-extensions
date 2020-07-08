Data Types
==========

The data streams that flow from processor to processor consist of data packets
that carry the data of interest (the payload), a source timestamp
(when the data packet was generated) and (optionally) a hardware timestamp
(original timestamp of the external hardware that generated the data).
Each input and output port on processor nodes only handles dedicated types of data.
For example, some processors operate on arrays of analog data.

Data types in Falcon form a hierarchy from generic to specific. At the top of
the hierarchy is the most generic data type "IData" that is the base for all
other data types. As long as the data type of an input port is the same or
more generic than the data type of the upstream output port, a connection can
be made. Thus, a processor node with an input port that expects the most
generic IData type, can handle incoming data streams of any other type.

Input ports may specify additional requirements for the incoming data. For example,
an input port could indicate that it only supports multi-channel analog data with
exactly 4 channels. An upstream processor with an output port that serves multi-channel
data packets with fewer or more channels will thus not be compatible.

Below is a list of data types that are currently available in this extension.


.. toctree::
   :maxdepth: 1
   :glob:

   datatype_description/eventdata
   datatype_description/spikedata
   datatype_description/multichanneldata
   datatype_description/muadata
   datatype_description/vectordata
   datatype_description/scalardata
