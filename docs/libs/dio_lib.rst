Digital IO library
==================

The Digital IO Library is used to read or write a digital signal from a n number of channels based on different modes and a pulse width.

The available modes are :
- NONE
- HIGH
- LOW
- TOGGLE
- PULSE

The library defined an interface which allow to easily extends to different types of IO device.

.. doxygenclass:: DigitalDevice
   :members:
   :undoc-members:

.. doxygenclass:: DigitalState
   :members:
   :undoc-members:


Available devices
-----------------

Dummy IO
********

Only classical read/write state.

.. doxygenclass:: DummyDIO
   :members:
   :undoc-members:

Advantech IO
************

Implementation present in past version of Falcon but removed because it needs a separate/private set of libraries.
It can be re-implemented in a separate extension if needed in the future.


