Ripple detection graph
======================

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
                detection lockout time: 50 #ms  # Post-detection lock-out - remove all detections

        CORTEX_detector :
            options:
                threshold dev: 12
                smooth time: 8 # in seconds
                detection lockout time: 40 #ms

        eventfilter:
            options:
                block duration: 40  # Filter out all detections after detecting a "ripple" aka artefact in the cortex signal
                sync time: 1.5      #  Time between two detections to consider it as only one.
                block wait time: 4  # Filter out all detections arrived before detecting a "ripple" aka artefact in the cortex signal
                                    # below 3.5, asynch can occur

        stimulation_trigger:
            options:
                delayed: true
                lockout period: 150  # Post-stimulation lock-out - remove all stimulation events / keep detections
                detection lockout period: 60  # Post-stimulation detection lock-out - remove all detections
                delay range:
                - 30
                - 50

        ttl_output:
            options:
                port address:  # port usb where the arduino is plugged
