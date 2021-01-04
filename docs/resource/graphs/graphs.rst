Graphs
======

A set of graphs is stored in remote-side and can be accessed either for direct running or as template in a graph file by
using the uri :

.. code-block::

    graphs://graph_name.yaml

Available graphs :

- events/event_test.yaml : allow to test event processors (eventsource -> eventfilter -> eventlogger)
- neuralynx/nlx_test.yaml : classic neuralynx workflow  (reader -> parser -> distributor)
- neuralynx/nlx_test_save.yaml : reader -> file serializer
- neuralynx/ripple_detection.yaml : see `ripple detection use-case <examples/graph.html>`_