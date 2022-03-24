![Falcon version](https://img.shields.io/badge/Falcon-v1.3.0-blue)

# Falcon extensions

The extension does not work as standalone and needs to be integrated with [Falcon](https://falcon-core.readthedocs.io)

The full documentation can be found [here](https://falcon-fklab-extensions.readthedocs.io) but below, you can found a quick overview :

**Processor** : 

- burst detector
- digital output
- dummy sink
- event filter
- event logger
- event sync
- file serializer
- level crossing detector
- mua estimator
- multi channel filter
- neuralynx reader
- rebuffer
- ripple detector
- running stats
- spike detector
- OpenEphysZMQ (input node to work with neuropixels from Open-Ephys)
- zmq serializer

**Datatype** :

- event data
- mua data
- multichannel data
- scalar data
- spike data
- vector data

**Additional libraries**:

- digital output (dio)
- dsp
- neuralynx
- vector operation


## Contribution 

If your issue concerned processors, datatypes or libs included in this repository, don't hesitate to add an issue 
describing the problem / or the feature to develop. Add the graph (+ eventually the config file) used to run Falcon
is highly recommended. 
 
To develop a new extension, an issue can be open in here for guidance but most probably the maintainer will advise you to 
create your own repository and then open an PR in Falcon to link your extension doc in the Falcon doc. 
