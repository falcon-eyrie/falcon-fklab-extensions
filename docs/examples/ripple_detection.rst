Hippocampal ripple detection
============================

Overview
--------


Implementation
--------------

- **Recording**: the hippocampal activity is recorded at 4 or 32 kHz depending on the experiment.
  Experiments that only focus on SWR are done at 4 kHz, whereas those that also use single cell
  signal (between 300 and 6000 Hz) have to be recorded at 32 kHz.

- **Filtering**: the filter is a Chebyshev type II with a passband from 130 to 283 Hz and 
  transition edges of 10%. This specific design comes from the initial wish
  to reject as many gamma bursts as possible.

- **Envelope estimation**: the simple squared value of the filtered signal is the estimation
  of the envelope in this algorithm. While neglecting to smooth the signal may lead to
  spurious detections due to outliers, the advantage of speed of detection obtained with
  this choice outweighs the disadvantage of false detections caused by a few rare outliers.

- **Thresholding**: Thresholding: the threshold is permanently modified as the experiment runs. 
  Indeed, it is based on statistical properties of the envelope which are the mean and the Mean Absolute Deviation (MAD).
  Every time a new value is computed (on the last time bin of 10 ms), it is added to the
  previous statistics with a predefined weight (α).

The corresponding equations are:

.. math::

    mean = (1  -α )* mean + α   * (mean of last time bin)
    
    MAD  = (1  - α)  * MAD + α  * |sample value  - mean|
    
    threshold = mean + arbitrary factor *MAD

The arbitrary factor is what the user has to tune for every experiment. 
The higher it is, the less the number of detections will be.

- **Cortical channel comparison**: as the rat is moving in its environment, some actions
  like chewing and bumping lead to artifacts in the neural signal. Fortunately, they are
  largely spread in the brain so that we can detect them outside hippocampus. This
  property is harnessed to reject them when they trigger a detection. An electrode records
  the signal in the cortex (where no SWRs occur) and the ripple detection algorithm is
  also running on it. If the same event is detected in the hippocampus and in the cortex,
  it is considered as an artifact and it is directly rejected.

To avoid over-stimulation, a post-processing technique consists of blocking any detection that
occurs less than 150 ms after a stimulation. This is called the lock-out period.

processors
----------

- source : :ref:`NlxReader`
- ripple filter 1 : :ref:`MultiChannelFilter`
- ripple filter 2 : :ref:`MultiChannelFilter`
- HIPPOCAMPUS detector: :ref:`RippleDetector`
- CORTEX detector: :ref:`RippleDetector`
- event filter : :ref:`EventFilter`
    * blocking event : Event from the cortex
    * input event : Event from the hippocampus
- network sink : :ref:`ZMQSerializer`
- event sink : :ref:`EventLogger`
- datasink ev : :ref:`FileSerializer`
- datasink ripple_stats : :ref:`FileSerializer`
- ttl output: SerialOutput (not yet ported on the new falcon)


States writable by the user
---------------------------

Processor :ref:`RippleDetector`:

   - threshold dev (double)
   - smooth time (double): integration time for signal statistics. Must be a positive number.
   - detection lockout time (double): Current refractory period following threshold crossing that is not
       considered for  updating signal statistics and for event detection.
   - stream events (bool)
   - stream statistics (bool)


.. note:: As their is two RippleDetectors, these states are available for each processor.

Output
------

- ripple events : :ref:`EventData`
- ripple statistics


.. include:: graph.rst
