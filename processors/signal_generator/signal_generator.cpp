#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cmath>
#include <fstream>
#include <g3log/g3log.hpp>
#include <sstream>
#include <string>
#include <thread>
#include "iprocessor.hpp"
#include "portpolicy.hpp"
#include "timeseriesdata/timeseriesdata.hpp"
#include "utilities/tsc_duration_clock.cpp"

#pragma pack(push, 1)
struct CSCRecord {
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
    options::Value<unsigned, false> n_channels_{32};
    options::Value<unsigned, false> sample_freq_{1000};
    options::Value<unsigned, false> buffer_size_{1};
    // Possible values: "sine", "square" and "file"
    options::Value<std::string, false> signal_type_{"square"};
    options::Value<std::string, true> file_path_{};

    uint64_t interval_ns_;
    unsigned unique_buffers_ = 0;
    unsigned hardware_timestamp_counter_ = 0;

    int file_fd_ = -1;
    char* mmap_ptr_ = nullptr;
    size_t mmap_size_ = 0;

    size_t total_records_ = 0;
    size_t current_record_idx_ = 0;
    uint32_t current_sample_idx_ = 0;  // 0 to 511

    double file_sampling_freq_ = 0.0;
    double ad_bit_volts_ = 0.0;

    /// @brief Computes active channels based on execution mode.
    /// @note Returns 1 if `signal_type` is `"file"`, otherwise falls back to `n_channels_`.
    unsigned effective_channel_count() const noexcept {
        return signal_type_() == "file" ? 1 : n_channels_();
    }

    /// @brief Resolves system sampling rate across file-driven and synthetic configurations.
    /// @pre Requires `load_ncs_file_to_ram()` to be called if streaming from file.
    double effective_sfreq() const noexcept {
        return signal_type_() == "file" ? file_sampling_freq_ : static_cast<double>(sample_freq_());
    }

    void extract_file_metadata() {
        std::ifstream file(file_path_(), std::ios::binary);
        if (!file.is_open()) {
            throw ProcessingPreprocessingError("Unable to open offline file path: " + file_path_(),
                                               name());
        }

        std::string header_text(16384, '\0');
        file.read(&header_text[0], 16384);
        file.clear();

        std::istringstream iss(header_text);
        std::string line;
        while (std::getline(iss, line)) {
            if (line.find("-ADBitVolts") != std::string::npos) {
                std::istringstream value_stream(line.substr(line.find("-ADBitVolts") + 11));
                value_stream >> ad_bit_volts_;
            } else if (line.find("-SamplingFrequency") != std::string::npos) {
                std::istringstream value_stream(line.substr(line.find("-SamplingFrequency") + 18));
                value_stream >> file_sampling_freq_;
            }
        }

        LOG(INFO) << name() << ": NCS Header Info\n"
                  << "  - ADBitVolts:         " << std::fixed << std::setprecision(14)
                  << ad_bit_volts_ << "\n"
                  << std::defaultfloat << "  - Sampling Frequency: " << file_sampling_freq_
                  << " Hz";
    }

    void load_ncs_file_to_ram() {
        LOG(INFO) << name() << ": Memory mapping file from " << file_path_();

        file_fd_ = open(file_path_().c_str(), O_RDONLY);
        if (file_fd_ < 0) {
            throw ProcessingPreprocessingError("Unable to open file: " + file_path_(), name());
        }

        struct stat sb;
        if (fstat(file_fd_, &sb) == -1) {
            close(file_fd_);
            throw ProcessingPreprocessingError("Failed stat on: " + file_path_(), name());
        }
        mmap_size_ = sb.st_size;

        mmap_ptr_ =
            static_cast<char*>(mmap(nullptr, mmap_size_, PROT_READ, MAP_SHARED, file_fd_, 0));
        if (mmap_ptr_ == MAP_FAILED) {
            close(file_fd_);
            throw ProcessingPreprocessingError("Mmap failed for: " + file_path_(), name());
        }

        constexpr size_t header_offset = 16384;
        if (mmap_size_ <= header_offset) {
            throw ProcessingPreprocessingError("File contains no records: " + file_path_(), name());
        }

        total_records_ = (mmap_size_ - header_offset) / sizeof(CSCRecord);
        current_record_idx_ = 0;
        current_sample_idx_ = 0;
    }

    void generate_signal_samples(TimeSeriesType<double>::Data* data_out) {
        constexpr size_t header_offset = 16384;
        const CSCRecord* records = reinterpret_cast<const CSCRecord*>(mmap_ptr_ + header_offset);

        for (unsigned int sample_idx = 0; sample_idx < buffer_size_(); ++sample_idx) {
            unsigned int current_ts = hardware_timestamp_counter_ + sample_idx;

            if (signal_type_() == "file" && total_records_ > 0) {
                const CSCRecord& rec = records[current_record_idx_];

                // 1. Calculate microsecond interpolation
                double dt_us = 1000000.0 / static_cast<double>(rec.sample_freq_hz);
                uint64_t interpolated_ts =
                    rec.timestamp_us +
                    static_cast<uint64_t>(std::round(current_sample_idx_ * dt_us));

                // 2. Assign real timestamp
                data_out->set_sample_timestamp(sample_idx,
                                               static_cast<unsigned int>(interpolated_ts));

                // 3. Process voltage value
                double val = static_cast<double>(rec.samples[current_sample_idx_]) * ad_bit_volts_;

                auto data_iter = data_out->begin_sample(sample_idx);
                *data_iter = val;

                // 4. Advance stream offsets safely
                current_sample_idx_++;
                if (current_sample_idx_ >= rec.num_valid_samples) {
                    current_sample_idx_ = 0;
                    current_record_idx_ = (current_record_idx_ + 1) % total_records_;
                }
            } else {
                // Fallback path for synthetic signals
                data_out->set_sample_timestamp(sample_idx, current_ts);
                auto data_iter = data_out->begin_sample(sample_idx);
                double t = static_cast<double>(current_ts) / effective_sfreq();

                for (unsigned int elect_idx = 0; elect_idx < effective_channel_count();
                     ++elect_idx) {
                    double val = 0.0;
                    if (signal_type_() == "sine") {
                        val = std::sin(2.0 * M_PI * 10.0 * t + elect_idx);
                    } else if (signal_type_() == "square") {
                        val = (std::sin(2.0 * M_PI * 5.0 * t) >= 0.0) ? 1.0 : -1.0;
                    }
                    *data_iter = val;
                    ++data_iter;
                }
            }
        }
    }

   public:
    SignalGenerator() {
        add_option("n_channels", n_channels_, "Number of simulated channels.");
        add_option("sample_freq", sample_freq_, "Sampling frequency in Hz.");
        add_option("buffer_size", buffer_size_, "Sampling frequency in Hz.");
        add_option("signal_type", signal_type_, "Type of signal: 'sine', 'square', or 'file'.");
        add_option("file_path", file_path_, "Path to the offline signal file.");
    }

    void CreatePorts() override {
        if (signal_type_() == "file") {
            extract_file_metadata();
        }

        auto params = TimeSeriesType<double>::Parameters(effective_channel_count(), buffer_size_(),
                                                         effective_sfreq());
        output_port_ =
            create_output_port<TimeSeriesType<double>>("signal", params, PortOutPolicy());
        interval_ns_ = 1e9 / (static_cast<double>(effective_sfreq()) / buffer_size_());
    }

    void Preprocess(ProcessingContext& _) override {
        if (signal_type_() == "file") {
            load_ncs_file_to_ram();
        }
    }

    void Process(ProcessingContext& context) override {
        uint64_t last_loop_tsc = TscDurationClock::tsc();

        while (!context.terminated()) {
            uint64_t now_tsc = TscDurationClock::tsc();
            std::chrono::nanoseconds elapsed = TscDurationClock::duration_since_tsc(last_loop_tsc);

            if (elapsed.count() < interval_ns_) {
                auto remaining = std::chrono::nanoseconds(interval_ns_) - elapsed;
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
            unique_buffers_++;

            auto* data_out = output_port_->slot(0)->ClaimData(false);

            generate_signal_samples(data_out);

            data_out->set_hardware_timestamp(hardware_timestamp_counter_);
            data_out->set_source_timestamp();
            data_out->set_ingestion_ns();

            output_port_->slot(0)->PublishData();

            hardware_timestamp_counter_ += buffer_size_();
        }
    }

    void Postprocess(ProcessingContext& _) override {
        LOG(INFO) << name() << ": Generated " << unique_buffers_ << " simulated buffers.";
        if (mmap_ptr_ && mmap_ptr_ != MAP_FAILED) {
            munmap(mmap_ptr_, mmap_size_);
            mmap_ptr_ = nullptr;
        }
        if (file_fd_ >= 0) {
            close(file_fd_);
            file_fd_ = -1;
        }
    }
};

REGISTERPROCESSOR(SignalGenerator)
