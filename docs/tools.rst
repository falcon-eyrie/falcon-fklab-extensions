Tools
=====

This extension provide two additional tools built at the same time as falcon-core but independently executable.
The NlxTestBench is used to fake the Neuralynx acquisition system while the FilterTest is used to explore the different
filters available in the resource folder.


Digital filter test
-------------------

The filter test tool can be used to test the digital filtering library
that is included with falcon. Given the definition of a FIR or IIR filter,
the tool provides three functions:

1. to filter a signal
2. to time the execution of filtering
3. to compute the filter's impulse response

.. toctree::
   :maxdepth: 1
   :glob:

   tools/filter_test/*


Neuralynx testbench
-------------------

The neuralynx testbench is a tool to generate a network stream of raw
Neuralynx Digilynx data packets, which can be read by the NlxReader
processor. It enables testing of falcon processing graphs without the
need to be connected to a live Neuralynx acquisition system.

The testbench comes with a number of built-in signal sources, including
a wave generator (sine, square and white noise) and signals read from file.
Sources are defined in a configuration file and can be selected using
keyboard commands.


.. toctree::
   :maxdepth: 1
   :glob:

   tools/nlxtestbench/*
