#include <algorithm>
#include <cmath>
#include <vector>
#include "iprocessor.hpp"
#include "options/value.hpp"
#include "streamports.hpp"
#include "timeseriesdata/timeseriesdata.hpp"

class Decimator : public IProcessor {
   private:
    PortIn<TimeSeriesType<double>>* in_port_ = nullptr;
    PortOut<TimeSeriesType<double>>* out_port_ = nullptr;

    options::Value<unsigned int, false> downsample_factor_{1,
                                                           options::positive<unsigned int>(true)};
    options::Value<unsigned int, false> buffer_size_{10, options::positive<unsigned int>(true)};

    std::vector<unsigned int> sample_buffer_;
    std::vector<std::pair<double, double>> filter_history_;  // Size: n_chans

    // Persistent state variables per output slot
    std::vector<unsigned int> sample_out_counter_;
    std::vector<int> stride_offset_;  // Changed to signed int to safely handle relative offsets
    std::vector<TimeSeriesType<double>::Data*> data_out_;

    double b0_ = 1.0, b1_ = 0.0, b2_ = 0.0;
    double a1_ = 0.0, a2_ = 0.0;

   public:
    Decimator() {
        add_option("downsample_factor", downsample_factor_, "Factor for downsampling.");
        add_option("buffer_size", buffer_size_, "Output buffer size in samples.");
    }

    void CreatePorts() override {
        in_port_ = create_input_port<TimeSeriesType<double>>(
            "input", TimeSeriesType<double>::Capabilities(ChannelRange(1, 256)),
            PortInPolicy(SlotRange(1, 1)));

        out_port_ = create_output_port<TimeSeriesType<double>>(
            "output", TimeSeriesType<double>::Parameters(), PortOutPolicy());
    }

    void CompleteStreamInfo() override {
        const auto n_out_slots = out_port_->number_of_slots();
        sample_buffer_.assign(n_out_slots, 0);

        sample_out_counter_.assign(n_out_slots, 0);
        stride_offset_.assign(n_out_slots, 0);
        data_out_.assign(n_out_slots, nullptr);

        const double in_sfreq = in_port_->prototype(0).sample_rate();
        const double out_sfreq = in_sfreq / downsample_factor_();
        const unsigned int n_chans = in_port_->prototype(0).ncolumns();

        filter_history_.assign(n_chans, {0.0, 0.0});

        if (downsample_factor_() > 1) {
            const double cutoff_freq = out_sfreq / 2.0;
            const double ff = cutoff_freq / in_sfreq;
            const double ita = 1.0 / std::tan(M_PI * ff);
            const double q = std::sqrt(2.0);
            const double den = 1.0 + q * ita + ita * ita;

            b0_ = 1.0 / den;
            b1_ = 2.0 * b0_;
            b2_ = b0_;
            a1_ = 2.0 * (1.0 - ita * ita) / den;
            a2_ = (1.0 - q * ita + ita * ita) / den;
        } else {
            b0_ = 1.0;
            b1_ = 0.0;
            b2_ = 0.0;
            a1_ = 0.0;
            a2_ = 0.0;
        }

        for (unsigned int k = 0; k < n_out_slots; ++k) {
            if (buffer_size_() > 0) {
                sample_buffer_[k] = buffer_size_();
            } else {
                sample_buffer_[k] = std::max(
                    1u, static_cast<unsigned int>(
                            std::floor(in_port_->prototype(0).nsamples() / downsample_factor_())));
            }

            out_port_->streaminfo(k).set_parameters(
                TimeSeriesType<double>::Parameters(n_chans, sample_buffer_[k], out_sfreq));
            out_port_->streaminfo(k).set_stream_name(in_port_->streaminfo(0).stream_name());
            out_port_->streaminfo(k).set_stream_rate(in_port_->streaminfo(0).stream_rate() *
                                                     in_port_->prototype(0).nsamples() /
                                                     sample_buffer_[k]);
        }
    }

    void Process(ProcessingContext& context) override {
        const auto n_out_slots = out_port_->number_of_slots();
        const unsigned int factor = downsample_factor_();
        TimeSeriesType<double>::Data* data_in = nullptr;

        std::vector<double> filtered_samples;

        while (!context.terminated()) {
            if (!in_port_->slot(0)->RetrieveData(data_in)) {
                continue;
            }

            const unsigned int n_samples = data_in->nsamples();
            const unsigned int n_chans = data_in->ncolumns();
            if (filtered_samples.size() != n_chans) {
                filtered_samples.resize(n_chans);
            }

            for (unsigned int s = 0; s < n_samples; ++s) {
                for (unsigned int c = 0; c < n_chans; ++c) {
                    double x = data_in->data_sample(s, c);
                    auto& history = filter_history_[c];

                    double w0 = x - a1_ * history.first - a2_ * history.second;
                    filtered_samples[c] = b0_ * w0 + b1_ * history.first + b2_ * history.second;

                    history.second = history.first;
                    history.first = w0;
                }

                for (unsigned int k = 0; k < n_out_slots; ++k) {
                    if (static_cast<int>(s) == stride_offset_[k]) {
                        if (!data_out_[k]) {
                            data_out_[k] = out_port_->slot(k)->ClaimData(false);
                        }

                        const unsigned int out_idx = sample_out_counter_[k];

                        for (unsigned int c = 0; c < n_chans; ++c) {
                            data_out_[k]->set_data_sample(out_idx, c, filtered_samples[c]);
                        }

                        data_out_[k]->set_sample_timestamp(out_idx, data_in->sample_timestamp(s));
                        sample_out_counter_[k]++;
                        stride_offset_[k] += factor;

                        if (sample_out_counter_[k] == sample_buffer_[k]) {
                            out_port_->slot(k)->PublishData();
                            data_out_[k] = nullptr;
                            sample_out_counter_[k] = 0;
                        }
                    }
                }
            }

            for (unsigned int k = 0; k < n_out_slots; ++k) {
                stride_offset_[k] -= n_samples;
            }

            in_port_->slot(0)->ReleaseData();
        }
    }
};

REGISTERPROCESSOR(Decimator)
