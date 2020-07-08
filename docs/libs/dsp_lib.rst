Digital Signal Processing (DSP) library
=======================================

The DSP library included with Falcon contains a number of modular and
reusable signal processing algorithms that can be used inside processor
nodes.

Detect threshold crossing
-------------------------

Given the value of a single data sample, an instance of the *ThresholdCrosser*
class detects if a threshold has been crossed. It does so by keeping track
of the previous sample value and testing if the current and previous data
samples are on opposite sides of the threshold. For example, for an upward
threshold crossing, a crossing is detected if the previous sample is smaller
than or equal to the threshold and the current sample is larger than the
threshold.

Operation of the ThresholdCrosser is determined by two parameters: the value
of the threshold and the direction of the crossing. These parameters are set
in the constructor or using member functions (*set_threshold* and
*set_slope*). Threshold crossing are detected by repeatedly calling the
*has_crossed* member function with the next data sample.

Here is an example of how to use the ThresholdCrosser class:

.. code-block:: c++

    // create instance with threshold = 10. and slope = UP
    auto t = dsp::algorithms::ThresholdCrosser( 10., Slope::UP );

    for (double k=0.5; k<20; ++k) {
        if (t.has_crossed(k)) {
            std::cout << "Threshold crossed!";
        }
    }

Compute running statistics of signal
------------------------------------

The *RunningStatistics* class supports the computation of running center and
dispersion measures of a signal. It is a virtual base class with currently
a single concrete subclass *RunningMeanMAD* that overrides the virtual method
*update_statistics* to compute the running mean and mean absolute deviation.

The weight of the most recent data sample is set by the *alpha* parameter,
which take a value between 0 and 1. The *update_statistics* method takes
a data sample and alpha value and updates the internal estimates of the center
and dispersion measures. For example, the *RunningMeanMad* class computes the
running mean as :math:`(1-alpha) \times mean + alpha \times sample`.

Optionally, a burn-in period can be set during which the alpha value gradually
changes from 1. to the preset alpha value. This has the effect that only weight
is given to the data samples and not the initial values of the center and
dispersion measures.

Also optionally, data samples that are more than a certain distance (in
multiples of the z-score) from the center (i.e. outliers) have reduced
influence on the computation of the running statistics.

Here is an example of how to use the *RunningMeanMAD* class:

.. code-block:: c++

    std::vector<double> samples{0.1, 2.0, 1.5, 3.2, 1.3, 2.4};
    
    // set alpha parameter to 0.1, no burn-in or outlier detection
    r = dsp::algorithms::RunningMeanMAD(0.1);
    r.add_samples(samples);

    std::cout << "running mean = " << r.mean() << " and running MAD = " << r.mad();

Detect local peaks
------------------

The *PeakDetector* class detects local peaks in a signal, gives access to
the timestamp and amplitude of the last detected peak and keeps track of
the total number of detected peaks.

Exponentially smooth signal
---------------------------

The *ExponentialSmoother* class smooths a signal sample by sample. The
integration window for smoothing is determined by the *alpha* parameter
that sets the weight of the new data sample, i.e.
:math:`value = value * (1-alpha) + sample * alpha`. Here is an example:

.. code-block:: c++

    double smooth_sample;
    
    // create smoother with alpha = 0.1
    auto s = dsp::algorithms::ExponentialSmoother(0.1);

    std::vector<double> samples{0.1, 2.0, 1.5, 3.2, 1.3, 2.4};

    for (auto k : samples) {
        smooth_sample = s.smooth(k);
    }


Detect spikes
-------------

This algorithm operates sample by sample and looks for upwards deflections in at least one of the channels above a certain threshold.

A spike is detected if the signal of at least one channel crosses the threshold and a local maxima is found in at least
one channel (not necessarily the same of that of threshold crossing) within a certain duration (determined by the peak lifetime)
The timestamp of the detected spike corresponds to that one of the first sample that crossed the threshold first
(independently on whether that sample belongs to the current or previous buffers)
In case a proper maximum is found on all channels, the peak values are returned, together with the threshold-crossing
timestamp; however, if on one or more channels no peaks were found, the values of the signals at the threshold-crossing
sample will be returned.


Filtering
---------

Finite impulse response (FIR) filters

Infinite impulse response (IIR) filters

