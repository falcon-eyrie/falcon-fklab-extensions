#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "iprocessor.hpp"
#include "timeseriesdata/timeseriesdata.hpp"

class SignalPlotter : public IProcessor {
   private:
    std::vector<std::vector<double>> channel_buffers_;
    std::atomic<size_t> write_index_{0};            // Atomic for lock-free reader access
    std::atomic<bool> buffers_initialized_{false};  // Atomic flag

    size_t samples_since_last_plot_ = 0;

    std::thread plot_thread_;
    std::mutex cv_mutex_;  // Mutex ONLY for condition variable signaling
    std::condition_variable cv_;
    std::atomic<bool> running_{false};
    std::atomic<bool> plot_ready_{false};

    FILE* gnuplot_pipe_ = nullptr;
    PortIn<TimeSeriesType<double>>* data_in_port_;

    options::Value<unsigned int, false> plot_history_size_{2 * 32768};
    options::Value<double, false> y_min_{-1000.0};
    options::Value<double, false> y_max_{1000.0};
    options::Value<double, false> y_scale_factor_{1e6};

    std::vector<char> text_stream_buffer_;

    void plot_worker_loop_() {
        while (running_) {
            std::unique_lock<std::mutex> lock(cv_mutex_);
            cv_.wait(lock, [this] { return plot_ready_ || !running_; });

            if (!running_) break;
            plot_ready_ = false;
            lock.unlock();  // Release CV lock immediately

            if (!buffers_initialized_.load(std::memory_order_acquire)) continue;

            // Take a snapshot of the current state atomically without blocking Process()
            size_t total_channels = channel_buffers_.size();
            size_t total_points = plot_history_size_();
            size_t snapshot_write_index = write_index_.load(std::memory_order_acquire);

            double channel_offset = (y_max_() - y_min_()) * 0.8;
            double global_y_max = y_max_() + ((total_channels - 1) * channel_offset);

            if (text_stream_buffer_.size() < total_points * (total_channels * 24 + 16)) {
                text_stream_buffer_.resize(total_points * (total_channels * 24 + 16));
            }

            fprintf(gnuplot_pipe_, "set yr [%.2f:%.2f]\n", y_min_(), global_y_max);

            std::string command = "plot '-' using 1:2 with lines lw 1.2 title 'Ch 0'";
            for (size_t c = 1; c < total_channels; ++c) {
                command += ", '-' using 1:2 with lines lw 1.2 title 'Ch " + std::to_string(c) + "'";
            }
            command += "\n";
            fputs(command.c_str(), gnuplot_pipe_);

            size_t buf_pos = 0;
            char* ptr = text_stream_buffer_.data();

            for (size_t c = 0; c < total_channels; ++c) {
                for (size_t i = 0; i < total_points; ++i) {
                    // Read directly from the shared buffer using the snapped index
                    size_t target_idx = (snapshot_write_index + i) % total_points;
                    double shifted_val =
                        channel_buffers_[c][target_idx] * y_scale_factor_() + (c * channel_offset);
                    int written = snprintf(ptr + buf_pos, 32, "%zu %.4f\n", i, shifted_val);
                    if (written > 0) buf_pos += written;
                }
                ptr[buf_pos++] = 'e';
                ptr[buf_pos++] = '\n';
            }

            fwrite(text_stream_buffer_.data(), 1, buf_pos, gnuplot_pipe_);
            fflush(gnuplot_pipe_);
        }
    }

   public:
    SignalPlotter() : IProcessor() {
        add_option("history_size", plot_history_size_, "Points on screen.", false);
        add_option("y_min", y_min_, "Y min.", false);
        add_option("y_max", y_max_, "Y max.", false);
        add_option("y_scale_factor", y_scale_factor_, "Y axis multiplier (e.g. 1e6 for uV).",
                   false);
    }

    ~SignalPlotter() {
        running_ = false;
        cv_.notify_all();
        if (plot_thread_.joinable()) plot_thread_.join();
        if (gnuplot_pipe_) pclose(gnuplot_pipe_);
    }

    void CreatePorts() override {
        data_in_port_ = create_input_port<TimeSeriesType<double>>(
            "signal", TimeSeriesType<double>::Capabilities(ChannelRange(0, 1024)),
            PortInPolicy(SlotRange(0, 256)));
    }

    void Prepare(GlobalContext& _) override {
        write_index_.store(0, std::memory_order_release);
        samples_since_last_plot_ = 0;
        buffers_initialized_.store(false, std::memory_order_release);
    }

    void Preprocess(ProcessingContext& _) override {
        gnuplot_pipe_ = popen("gnuplot -p", "w");
        if (!gnuplot_pipe_) {
            throw ProcessingPreprocessingError("Gnuplot pipe failed to open.");
        }

        fprintf(gnuplot_pipe_, "set term x11 noraise\n");
        fprintf(gnuplot_pipe_, "set title 'Realtime Multi-Channel Signal Monitor'\n");
        fprintf(gnuplot_pipe_, "set noautoscale y\n");
        fprintf(gnuplot_pipe_, "set xr [0:%u]\n", plot_history_size_() - 1);
        fprintf(gnuplot_pipe_, "set grid\n");
        fflush(gnuplot_pipe_);

        running_ = true;
        plot_thread_ = std::thread(&SignalPlotter::plot_worker_loop_, this);
    }

    void Process(ProcessingContext& context) override {
        TimeSeriesType<double>::Data* data_in = nullptr;
        const size_t total_points = plot_history_size_();

        while (!context.terminated()) {
            bool has_data = data_in_port_->slot(0)->RetrieveData(data_in);
            if (!has_data) break;

            size_t sample_count = data_in->nsamples();
            size_t channel_count = data_in->ncolumns();

            // Allocation happens once at start; thread-safe because reader checks initialization
            // flag
            if (!buffers_initialized_.load(std::memory_order_acquire) ||
                channel_buffers_.size() != channel_count) [[unlikely]] {
                channel_buffers_.resize(channel_count);
                for (size_t c = 0; c < channel_count; ++c) {
                    channel_buffers_[c].assign(total_points, 0.0);
                }
                buffers_initialized_.store(true, std::memory_order_release);
            }

            // Local cache of atomic index to avoid reading atomic inside inner loops
            size_t local_idx = write_index_.load(std::memory_order_relaxed);

            for (size_t r = 0; r < sample_count; ++r) {
                auto row_iter = data_in->begin_sample(r);
                for (size_t c = 0; c < channel_count; ++c) {
                    channel_buffers_[c][local_idx] = *row_iter;
                    ++row_iter;
                }
                local_idx = (local_idx + 1) % total_points;
            }

            // Publish index to the worker thread
            write_index_.store(local_idx, std::memory_order_release);

            samples_since_last_plot_ += sample_count;

            if (samples_since_last_plot_ >= total_points) {
                samples_since_last_plot_ %= total_points;

                if (!plot_ready_.load(std::memory_order_relaxed)) {
                    // Lock and signal only when a frame is ready
                    std::lock_guard<std::mutex> lock(cv_mutex_);
                    plot_ready_ = true;
                    cv_.notify_one();
                }
            }

            data_in_port_->slot(0)->ReleaseData();
        }
    }
};

REGISTERPROCESSOR(SignalPlotter)
