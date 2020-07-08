.. _processors:

Processors
==========

Built-in processors can be found in the `falcon-fklab-extensions <https://bitbucket.org/kloostermannerflab/falcon-fklab-extensions/src/master/>`_

For each processor, a description is given of:

- what data streams it expects on the input ports
- what data streams are generated on the output ports
- the configuration options (used to initialize the processor)
- the shared states that are exposed


.. toctree::
   :maxdepth: 2
   :glob:

   processors_description/detectors
   processors_description/signal_generators_filters_converters
   processors_description/events
   processors_description/digitaloutput/*
   processors_description/serializers
   processors_description/examples
