Usage
=====

::

    nlxtestbench -c /path/to/config/file -a 1


Command-line options
********************

::

    usage: ./nlxtestbench [options] ...
    options:
      -c, --config       configuration file (string [=$HOME/.nlxtestbench/config.yaml])
      -a, --autostart    source to auto start streaming (int [=-1])
      -r, --rate         data stream rate (Hz) (double [=-1])
      -n, --npackets     maximum number of packets to stream (0 means all packets) (long [=-1])
      -?, --help         print this message


Keyboard commands
-----------------

After starting the testbench, the following keys are available:

======= ===============================
key     action
======= ===============================
<space> list all defined sources
a-z     select signal to stream
<ESC>   quit
======= ===============================