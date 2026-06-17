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
    std::atomic<size_t> write_index_{0};
    std::atomic<bool> buffers_initialized_{false};

    size_t samples_since_last_plot_ = 0;
    std::atomic<size_t> total_samples_written_{0};

    std::thread plot_thread_;
    std::mutex cv_mutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_{false};
    std::atomic<bool> plot_ready_{false};

    FILE* gnuplot_pipe_ = nullptr;
    PortIn<TimeSeriesType<double>>* data_in_port_;

    options::Value<unsigned int, false> plot_history_size_{1000};

    std::vector<char> text_stream_buffer_;

    void plot_worker_loop_() {
        gnuplot_pipe_ = popen("gnuplot", "w");
        if (!gnuplot_pipe_) {
            return;
        }

        setvbuf(gnuplot_pipe_, nullptr, _IOFBF, 524288);

        fprintf(gnuplot_pipe_, "set term x11 title '%s' noraise\n", name().c_str());
        fprintf(gnuplot_pipe_, "unset mouse\n");
        fprintf(gnuplot_pipe_, "set grid\n");
        fprintf(gnuplot_pipe_, "unset title\n");
        fprintf(gnuplot_pipe_, "unset key\n");
        fflush(gnuplot_pipe_);

        std::vector<double> binary_block;

        while (running_) {
            std::unique_lock<std::mutex> lock(cv_mutex_);
            cv_.wait(lock, [this] { return plot_ready_ || !running_; });

            if (!running_) break;
            plot_ready_ = false;
            lock.unlock();

            if (!buffers_initialized_.load(std::memory_order_acquire)) continue;

            size_t total_channels = channel_buffers_.size();
            size_t total_points = plot_history_size_();
            size_t snapshot_write_index = write_index_.load(std::memory_order_acquire);
            size_t snapshot_total_samples = total_samples_written_.load(std::memory_order_acquire);

            size_t x_max =
                (snapshot_total_samples > 0) ? (snapshot_total_samples - 1) : (total_points - 1);
            size_t x_min = (x_max >= total_points) ? (x_max - total_points + 1) : 0;

            binary_block.resize(total_points * 2);

            fprintf(gnuplot_pipe_, "set multiplot layout %zu, 1\n", total_channels);

            for (size_t i = 0; i < total_points; ++i) {
                binary_block[i * 2] = static_cast<double>(x_min + i);
            }

            for (size_t c = 0; c < total_channels; ++c) {
                fprintf(gnuplot_pipe_, "set xr [%zu:%zu]\n", x_min, x_max);
                fprintf(gnuplot_pipe_,
                        "plot '-' binary record=%zu format='%%double%%double' using 1:2 with lines "
                        "lw 1.2 lc %zu notitle\n",
                        total_points, c + 1);

                const auto& channel_buf = channel_buffers_[c];

                for (size_t i = 0; i < total_points; ++i) {
                    size_t target_idx = (snapshot_write_index + i) % total_points;
                    binary_block[i * 2 + 1] = channel_buf[target_idx];
                }

                fwrite(binary_block.data(), sizeof(double), total_points * 2, gnuplot_pipe_);
            }
            fprintf(gnuplot_pipe_, "unset multiplot\n");
            fflush(gnuplot_pipe_);
        }

        if (gnuplot_pipe_) {
            pclose(gnuplot_pipe_);
            gnuplot_pipe_ = nullptr;
        }
    }
    void reset_state_() {
        write_index_.store(0, std::memory_order_release);
        total_samples_written_.store(0, std::memory_order_release);
        samples_since_last_plot_ = 0;
        buffers_initialized_.store(false, std::memory_order_release);
        plot_ready_ = false;
        channel_buffers_.clear();
    }

   public:
    SignalPlotter() : IProcessor() {
        add_option("history_size", plot_history_size_, "Points on screen.", false);
    }

    ~SignalPlotter() {
        // Fallback cleanup in case Postprocess wasn't called
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

    void Prepare(GlobalContext& _) override { reset_state_(); }

    void Preprocess(ProcessingContext& _) override {
        running_ = true;
        plot_ready_ = false;
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

            if (!buffers_initialized_.load(std::memory_order_acquire) ||
                channel_buffers_.size() != channel_count) [[unlikely]] {
                channel_buffers_.resize(channel_count);
                for (size_t c = 0; c < channel_count; ++c) {
                    channel_buffers_[c].assign(total_points, 0.0);
                }
                buffers_initialized_.store(true, std::memory_order_release);
            }

            size_t local_idx = write_index_.load(std::memory_order_relaxed);

            for (size_t r = 0; r < sample_count; ++r) {
                auto row_iter = data_in->begin_sample(r);
                for (size_t c = 0; c < channel_count; ++c) {
                    channel_buffers_[c][local_idx] = *row_iter;
                    ++row_iter;
                }
                local_idx = (local_idx + 1) % total_points;
            }

            write_index_.store(local_idx, std::memory_order_release);
            total_samples_written_.fetch_add(sample_count, std::memory_order_release);

            samples_since_last_plot_ += sample_count;

            if (samples_since_last_plot_ >= total_points) {
                samples_since_last_plot_ %= total_points;

                if (!plot_ready_.load(std::memory_order_relaxed)) {
                    std::lock_guard<std::mutex> lock(cv_mutex_);
                    plot_ready_ = true;
                    cv_.notify_one();
                }
            }

            data_in_port_->slot(0)->ReleaseData();
        }
    }

    void Postprocess(ProcessingContext& _) override {
        running_ = false;
        {
            std::lock_guard<std::mutex> lock(cv_mutex_);
            plot_ready_ = true;  // Wake up the thread immediately
        }
        cv_.notify_all();

        if (plot_thread_.joinable()) {
            plot_thread_.join();
        }

        reset_state_();
    }
};

REGISTERPROCESSOR(SignalPlotter)
