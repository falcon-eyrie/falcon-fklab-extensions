Data Types
==========

The data streams that flow from processor to processor consist of data packets
that carry the data of interest (the payload), a source timestamp
(when the data packet was generated) and (optionally) a hardware timestamp
(original timestamp of the external hardware that generated the data).
Each input and output port on processor nodes only handles dedicated types of data.
For example, some processors operate on arrays of analog data.

See how to `extends a datatype <https://falcon-core.readthedocs.io/en/latest/extensions/extend_datatype.html>`_ on Falcon for more information.
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
