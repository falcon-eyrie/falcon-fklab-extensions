Neuralynx library
=================

The Neuralynx library is used to:

- handle incoming records (e.g. nlxreader)
- construct a NlxRecord object with required number of channels
- read packet into char buffer and call NlxRecord::FromNetworkBuffer

If the buffer is valid (i.e. record size is OK, first 3 fields are OK, CRC checks out), it grabs data, timestamp, parallel port
value with member methods to create new record and constructs a NlxRecord object with required number of channels (optionally
Initialize() - will be done during construction), then it sets the new record and Finalize() (will compute CRC) and finally
copy packet into external buffer to be sent over network using ToNetworkBuffer.

.. doxygennamespace:: nlx
   :members:
   :undoc-members:
