Usage
=====


Command-line options
--------------------

.. code-block:: bash

    usage: ./filtertest [options] ... filter_definition_file
    options:
      -s, --signal               signal to filter (string [=])
      -o, --signal_output        output file for filtered signal (string [=filtered_signal.dat])
      -t, --timing               perform timing of filtering operation
      -n, --n_timing_points      number of points for timing (unsigned long [=1000000])
      -c, --n_timing_channels    number of channels for timing (unsigned int [=1])
      -i, --impulse              compute impulse response of filter
      -f, --impulse_output       output file for impulse response (string [=impulse_response.dat])
      -p, --n_impulse_points     number of points for impulse response (0 means number of samples is chosen automatically) (unsigned int [=0])
      -?, --help                 print this message

Examples
--------

Filter a signal and save the result in *output.dat*:

::

    filtertest -s signal.dat -o output.dat /path/to/filter/file

Perform timing of the filtering operation using the specified number
of time points and channels:

::

    filtertest -t -n 1000000 -c 10 /path/to/filter/file

Compute impulse response and save result:

::

    filtertest -i -f output.dat /path/to/filter/file
