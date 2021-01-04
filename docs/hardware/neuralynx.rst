Neuralynx
=========

The acquisition system is packaging data to be sent to the digilinx system.
Only computers with the right address IP and the right mac address can read the data.
After packaging, packets are duplicated to also be sent to Falcon.

Because of this packaging, the falcon computer, to be able to read the data as well, needs to have a
Ethernet interface specifically for this communication (where to plug the Ethernet cable) with the exact same
configuration as the Cheetah computer (originally receiving the data):

- ip address
- mac address


This same address ip is re-used in the config file of the graph used in falcon.