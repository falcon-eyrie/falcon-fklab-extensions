#pragma once
#include <memory>
#include <string>

#include "dsp/algorithms.hpp"
#include "eventdata/eventdata.hpp"
#include "iprocessor.hpp"
#include "options/options.hpp"
#include "options/units.hpp"
#include "timeseriesdata/timeseriesdata.hpp"

class RippleDetector : public IProcessor {
   public:
    RippleDetector();
    void CreatePorts() override;
    void CompleteStreamInfo() override;

    void Preprocess(ProcessingContext& context) override;
    /** @brief Primary execution loop processing streaming samples, updating tracking metrics, and
     * broadcasting detections. */
    void Process(ProcessingContext& context) override;
    void Postprocess(ProcessingContext& context) override;

   protected:
    /** @brief Extracts or transforms a single data point from the input pipeline for threshold
     * analysis. */
    double compute_value(TimeSeriesType<double>::Data* data_in, unsigned int sample);

    /** @var data_in_port_
        @brief High-density LFP input stream (1 to 256 channels). */
    PortIn<TimeSeriesType<double>>* data_in_port_;
    /** @var event_out_port_
        @brief Zero-latency output channel for discrete SWR event timestamps. */
    PortOut<EventType>* event_out_port_;
    /** @var stats_out_port_
        @brief High-throughput telemetry pipeline for baseline tracking analysis. */
    PortOut<TimeSeriesType<double>>* stats_out_port_;

    ProducerState<double>* threshold_;
    ProducerState<double>* signal_mean_;
    ProducerState<double>* signal_dev_;
    BroadcasterState<bool>* ripple_;
    StaticState<double>* threshold_dev_;
    StaticState<double>* detection_lockout_time_;
    FollowerState<bool>* detection_enabled_;
    StaticState<bool>* stream_events_;
    StaticState<double>* smooth_time_;
    StaticState<bool>* stats_out_;

    bool stats_out_enabled_;
    std::uint64_t stats_nsamples_;
    std::uint64_t block_;
    std::uint64_t burn_in_;
    double sample_rate_;
    double acc_;
    std::unique_ptr<dsp::algorithms::RunningMeanMAD> running_statistics_;
    std::unique_ptr<dsp::algorithms::ThresholdCrosser> threshold_detector_;

   public:
    const std::vector<std::string> STATS_LABEL = {"statistics", "threshold", "deviation"};
    const std::string THRESHOLD_DEV = "threshold_dev";
    const std::string SMOOTH_TIME = "smooth_time";
    const std::string DETECTION_LOCKOUT_TIME = "analysis_lockout_time";
    const std::string STREAM_EVENTS = "stream_events";
    const std::string STREAM_STATISTICS = "stream_statistics";

   protected:
    /** @var initial_threshold_dev_
        @brief Configurable baseline multiplier defining default SWR detection sensitivity. */
    options::Double initial_threshold_dev_{6.};
    /** @var initial_smooth_time_
        @brief Measurement option managing baseline initialization window duration. */
    options::Measurement<double, false> initial_smooth_time_{10., "second",
                                                             options::positive<double>(true)};
    /** @var initial_detection_lockout_time_
        @brief Measurement option managing the post-trigger deadtime window. */
    options::Measurement<double, false> initial_detection_lockout_time_{
        30., "ms", options::positive<double>(true)};

    /** @var default_stream_events_
        @brief Boolean option toggling automatic event packet routing on startup. */
    options::Bool default_stream_events_{true};
    /** @var initial_stats_out_
        @brief Boolean option toggling diagnostic tracking streams on startup. */
    options::Bool initial_stats_out_{true};
    /** @var stats_buffer_size_
        @brief Measurement option allocating continuous data memory capacity thresholds. */
    options::Measurement<double, false> stats_buffer_size_{0.5, "second",
                                                           options::positive<double>(true)};
    /** @var stats_downsample_factor_
        @brief Downsampling selection defining telemetry output bandwidth scaling. */
    options::Value<unsigned int, false> stats_downsample_factor_{
        1, options::positive<unsigned int>(true)};
    /** @var use_power_
        @brief Boolean option tracking whether evaluation uses root-mean or raw metrics. */
    options::Bool use_power_{true};
};
