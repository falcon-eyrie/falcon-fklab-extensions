.. _EventData:

EventData
=========

General description
-------------------
Data packets of the EventData type hold a string that
describes the event. Processors that operate on EventData
will usually specify what events they generate on their
output ports or what events they expect at their input ports.

Payload details
---------------

.. list-table::
   :header-rows: 1

   * - name
     - type
     - description
   * - event
     - string
     - event string

API
---

TODO (put here an overview of the public
methods and a short description of what they do.
Should we leave this to doxygen??)

Parameters
----------

.. list-table::
   :header-rows: 1

   * - name
     - type
     - description
     - validation
   * - event
     - string
     - default event string
     - cannot be empty

Capabilities
------------
(none)

Binary Serialization
--------------------
For serialization formats *FULL* and *COMPACT*,
the event string is serialized as a 128-byte long string.

YAML Serialization
------------------
For serialization formats *FULL* and *COMPACT*,
the following YAML is emitted:

event: [event string]
