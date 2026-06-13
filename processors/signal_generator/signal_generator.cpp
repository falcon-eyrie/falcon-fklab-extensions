#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "context.hpp"
#include "iprocessor.hpp"
#include "portpolicy.hpp"
#include "timeseriesdata/timeseriesdata.hpp"
#include "utilities/tsc_duration_clock.cpp"

#pragma pack(push, 1)
struct NcsRecord {
    uint64_t timestamp_us;
    uint32_t channel_number;
    uint32_t sample_freq_hz;
    uint32_t num_valid_samples;
    int16_t samples[512];
};
#pragma pack(pop)

class SignalGenerator : public IProcessor {
   private:
    PortOut<TimeSeriesType<double>>* output_port_;
    options::Value<unsigned, false> n_channels{32};
    options::Value<unsigned, false> sample_freq{1000};
    options::Value<std::string, false> signal_type{"square"};  // "sine", "square", "file"
    options::Value<std::string, false> file_path{""};

    const unsigned BUFFER_SIZE = 30;
    uint64_t interval_ns;
    unsigned unique_buffers = 0;
    unsigned hardware_timestamp_counter = 0;

    std::vector<double> file_buffer;
    size_t file_sample_offset_ = 0;
    double file_sampling_freq = 0.0;

    void load_ncs_file_to_ram() {
        LOG(INFO) << name() << ": Preloading file from " << file_path();

        std::ifstream file(file_path(), std::ios::binary);
        if (!file.is_open()) {
            throw ProcessingPreprocessingError("Unable to open offline file path: " + file_path(),
                                               name());
        }

        std::string header_text(16384, '\0');
        file.read(&header_text[0], 16384);

        double ad_bit_volts = 0.0;

        std::istringstream iss(header_text);
        std::string line;
        while (std::getline(iss, line)) {
            if (line.find("-ADBitVolts") != std::string::npos) {
                std::istringstream value_stream(line.substr(line.find("-ADBitVolts") + 11));
                value_stream >> ad_bit_volts;
            } else if (line.find("-SamplingFrequency") != std::string::npos) {
                std::istringstream value_stream(line.substr(line.find("-SamplingFrequency") + 18));
                value_stream >> file_sampling_freq;
            }
        }

        LOG(INFO) << name() << ": NCS Header - ADBitVolts: " << ad_bit_volts
                  << ", Sampling Frequency: " << file_sampling_freq << " Hz";

        NcsRecord record;
        while (file.read(reinterpret_cast<char*>(&record), sizeof(NcsRecord))) {
            for (uint32_t i = 0; i < record.num_valid_samples; ++i) {
                file_buffer.push_back(static_cast<double>(record.samples[i]) * ad_bit_volts);
            }
        }
        file_sample_offset_ = 0;
    }

    void generate_signal_samples(TimeSeriesType<double>::Data* data_out) {
        for (unsigned int sample_idx = 0; sample_idx < BUFFER_SIZE; ++sample_idx) {
            unsigned int current_ts = hardware_timestamp_counter + sample_idx;
            data_out->set_sample_timestamp(sample_idx, current_ts);

            auto data_iter = data_out->begin_sample(sample_idx);
            auto sfreq = signal_type() == "file" ? file_sampling_freq : sample_freq();
            double t = static_cast<double>(current_ts) / sfreq;
            auto channel_count = signal_type() == "file" ? 1 : n_channels();
            for (unsigned int elect_idx = 0; elect_idx < channel_count; ++elect_idx) {
                double val = 0.0;

                if (signal_type() == "sine") {
                    val = std::sin(2.0 * M_PI * 10.0 * t + elect_idx);
                } else if (signal_type() == "square") {
                    val = (std::sin(2.0 * M_PI * 5.0 * t) >= 0.0) ? 1.0 : -1.0;
                } else if (signal_type() == "file") {
                    if (elect_idx == 0 && !file_buffer.empty()) {
                        val = file_buffer[file_sample_offset_];
                        file_sample_offset_ = (file_sample_offset_ + 1) % file_buffer.size();
                    }
                }

                *data_iter = val;
                ++data_iter;
            }
        }
    }

   public:
    SignalGenerator() {
        add_option("n_channels", n_channels, "Number of simulated channels.");
        add_option("sample_freq", sample_freq, "Sampling frequency in Hz.");
        add_option("signal_type", signal_type, "Type of signal: 'sine', 'square', or 'file'.");
        add_option("file_path", file_path, "Path to the offline signal file.");
    }

    void CreatePorts() override {
        if (signal_type() == "file") {
            load_ncs_file_to_ram();
        }
        auto sfreq = signal_type() == "file" ? file_sampling_freq : sample_freq();

        auto channel_count = signal_type() == "file" ? 1 : n_channels();

        auto params = TimeSeriesType<double>::Parameters(channel_count, BUFFER_SIZE, sfreq);
        output_port_ =
            create_output_port<TimeSeriesType<double>>("signal", params, PortOutPolicy());
        interval_ns = 1e9 / (static_cast<double>(sfreq) / BUFFER_SIZE);
    }

    void Process(ProcessingContext& context) override {
        uint64_t last_loop_tsc = TscDurationClock::tsc();

        while (!context.terminated()) {
            uint64_t now_tsc = TscDurationClock::tsc();
            std::chrono::nanoseconds elapsed = TscDurationClock::duration_since_tsc(last_loop_tsc);

            if (elapsed.count() < interval_ns) {
                auto remaining = std::chrono::nanoseconds(interval_ns) - elapsed;
                if (remaining.count() > 2000) {
                    auto sleep_duration = (remaining * 95) / 100;
                    std::this_thread::sleep_until(std::chrono::steady_clock::now() +
                                                  sleep_duration);
                } else {
                    asm volatile("pause" ::: "memory");
                }
                continue;
            }

            last_loop_tsc = now_tsc;
            unique_buffers++;

            auto* data_out = output_port_->slot(0)->ClaimData(false);

            generate_signal_samples(data_out);

            data_out->set_hardware_timestamp(hardware_timestamp_counter);
            data_out->set_source_timestamp();
            data_out->set_ingestion_ns();

            output_port_->slot(0)->PublishData();

            hardware_timestamp_counter += BUFFER_SIZE;
        }
    }

    void Postprocess(ProcessingContext& _) override {
        LOG(INFO) << name() << ": Generated " << unique_buffers << " simulated buffers.";
    }
};

REGISTERPROCESSOR(SignalGenerator)
