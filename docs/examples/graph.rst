Ripple detection graph
======================

.. code-block:: yaml

    processors:
        source:
            class: NlxReader
            advanced:
                threadpriority: 100
                threadcore: 0
            options:
                address: 192.168.3.101
                port: 26090
                batch size: 3
                update interval: 10
                channelmap:
                    hp1: [13,20]
                    cx1: [9]
    ​
        ripple filter(1-2):
            class: MultiChannelFilter
            options:
                filter:
                    file: filter://iir_ripple_low_delay.filter
    ​
        HIPPOCAMPUS detector:
            class: RippleDetector
            options:
                threshold dev: 14
                smooth time: 7 # in seconds
                detection lockout time: 50 #ms
                stream events: true
                stream statistics: true
                statistics buffer size: 1.0 # sec
                statistics downsample factor: 4
                use power: true
    ​
        CORTEX_detector:
            class: RippleDetector
            options:
                threshold dev: 12
                smooth time: 8 # in seconds
                detection lockout time: 40 #ms
                stream events: true
                stream statistics: false
                statistics buffer size: 1.0 # sec
                statistics downsample factor: 4
                use power: true
    ​
        event filter:
            class: EventFilter
            options:
                target event: ripple
                blockout duration: 40
                sync time: 1.5
                block wait time: 4 # below 3.5, asynch can occur
                detection criterion: "any" # 'any', 'all' or number
                discard warnings: false
    ​
        network sink:
            class: ZMQSerializer
            options:
                encoding: binary
                format: compact
    ​
        event sink:
            class: EventSink
            options:
                target event: ripple
    ​
        datasink ev:
            class: FileSerializer
            options:
                encoding: yaml
                format: full
    ​
        datasink ripplestats:
            class: FileSerializer
            options:
                encoding: binary
                format: compact

        ttl output:
            class: SerialOutput
            options:
                enabled: true
                target event: ripple
                message: "d" # d for delay stim, o for ontime
                lockout period: 250
    ​
    connections:
        - source.hp1=ripple filter1.data
        - source.cx1=ripple filter2.data
        - ripple filter1.data=HIPPOCAMPUS detector.data
        - ripple filter2.data=CORTEX detector.data
        - HIPPOCAMPUS detector.events=event filter.events
        - CORTEX detector.events=event filter.blocking events
        - HIPPOCAMPUS detector.statistics.0=network sink.data
        - event filter.events.0=ttl output.events
        - event filter.events.0=event sink.events
        - event filter.events.0=datasink ev.data
        - HIPPOCAMPUS detector.statistics.0=datasink ripplestats.data