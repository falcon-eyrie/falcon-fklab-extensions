Ripple detection graph
======================


The graph is divided in two parts : 

- server-side template
- user-side graph

The user should called the server-side template and only modified the option is interested on. 
The graph below is an example : 

.. code-block:: yaml

    falcon:
      version: 1.0

    graph: graphs://neuralynx/ripple_detection.yaml

    options:
        source:
           options:
              address:      # Ip address of Cheetah system
              channelmap:
               hp: []       # Hippocampus channel to analyze
               cx: []       # Cortex channel to analyze

        HIPPOCAMPUS_detector:
            options:
                threshold dev: 3
                smooth time: 7 # in seconds
                analysis lockout time: 50 #ms  # Post-detection lock-out - remove all detections

        CORTEX_detector :
            options:
                threshold dev: 12
                smooth time: 8 # in seconds
                analysis lockout time: 40 #ms  # Stop multiple detections of ripples

        eventfilter:
            options:
                block duration: 40  # Filter out all detections after detecting a "ripple" aka artefact in the cortex signal
                sync time: 1.5      #  Time between two detections to consider it as only one.
                block wait time: 4  # Filter out all detections arrived before detecting a "ripple" aka artefact in the cortex signal
                                    # below 3.5, asynch can occur

        stimulation_trigger:
            options:
                delayed: true
                event trigger lockout time:   # Stop the trigger of events but still are counted in the statistics of the ripple detector
                    period:  250
                    detection: false    # Remove duplicate stimulations
                    stimulation: true   #  Avoid overstimulation lock-out -

                analysis lockout time:   # Stop artefacts lock-out - Stop the detection in the ripple detector after a stimulation
                    period: 50
                    starting time: [0]
                delay range:
                - 30
                - 50

        ttl_output:
            options:
                port address:  # port usb where the arduino is plugged


For more information, this is the template graph called in background : 

.. literalinclude:: ../../resources/graphs/neuralynx/ripple_detection.yaml