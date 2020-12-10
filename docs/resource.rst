Resources
=========

Resources are automatically added in falcon at installation time. It is available with their corresponding URIs.
It can be found either in the _build folder inside the falcon-core repository (post-build time) or the share folder in the installation path.

Graph examples
--------------
A set of graphs is stored in remote-side and can be accessed either for direct running or as template in a graph file by
using the uri : graphs://graph_name.yaml

Available graphs :

- events/event_test.yaml : allow to test event processors (eventsource -> eventfilter -> eventlogger)
- neuralynx/nlx_test.yaml : classic neuralynx workflow  (reader -> parser -> distributor)
- neuralynx/nlx_test_save.yaml : reader -> file serializer
- neuralynx/ripple_detection.yaml : see `ripple detection use-case <examples/graph.html>`_


Filters
-------

A set of filters is also stored in remote side and can be accessed via the uri:

filters://path_filter_definition.filter

Available filters :

- cheetah/spike_6k_32taps.filter
- iir_antialiasing
    - cheby2_antialiasing_32kHz.filter
    - cheby2_antialiasing_32kHz_ba.filter

    .. image:: resource/filters/cheby2_antialiasing.png

- iir_ripple
    - ellip_ripple_4kHz.filter

    .. image:: resource/filters/ellip_ripple_4kHz.png

- iir_ripple_low_design
    - matlab_design/irr_ripple_low_delay.filter
    - python_design/cheby2_ripple_2