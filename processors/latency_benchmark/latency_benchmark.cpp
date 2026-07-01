#include <chrono>
#include <cstdint>
#include <fstream>
#include <limits>
#include <string>
#include <vector>
#include "iprocessor.hpp"

#include "utilities/tsc_duration_clock.cpp"

class LatencyBenchmark : public IProcessor {
   private:
    PortIn<AnyType>* data_in_port_;
    std::ofstream output_file_;
    std::vector<uint64_t> samples_buffer_;
    static constexpr size_t BATCH = 50;

   public:
    LatencyBenchmark() : IProcessor() {}

    void CreatePorts() override {
        data_in_port_ = create_input_port<AnyType>("data", AnyType::Capabilities(),
                                                   PortInPolicy(SlotRange(1, 256), false));
    }

    void Preprocess(ProcessingContext& context) override {
        samples_buffer_.reserve(BATCH);

        auto file_name = "latency_benchmark_data.bin";
        auto file_path = context.resolve_path("run://") + "/" + file_name;
        output_file_.open(file_path, std::ios::binary);
        if (!output_file_.is_open()) {
            throw std::runtime_error("Cannot open latency benchmark output file: " + file_path);
        }
    }

    void Process(ProcessingContext& context) override {
        AnyType::Data* data_in = nullptr;
        auto min_latency_ns = std::numeric_limits<uint64_t>::max();
        auto max_latency_ns = 0;
        while (!context.terminated()) {
            for (SlotType s = 0; s < data_in_port_->number_of_slots(); ++s) {
                if (!data_in_port_->slot(s)->RetrieveData(data_in)) {
                    break;
                }
                auto ingestedAt = data_in->ingestion_tsc();

                data_in_port_->slot(s)->ReleaseData();

                auto latency = TscDurationClock::duration_since_tsc(ingestedAt).count();
                samples_buffer_.push_back(latency);

                auto separate_digits = [](auto n) {
                    std::string num_str = std::to_string(n);
                    int insert_position = num_str.length() - 3;
                    while (insert_position > 0) {
                        num_str.insert(insert_position, "_");
                        insert_position -= 3;
                    }
                    return num_str;
                };
                if (latency > max_latency_ns) {
                    max_latency_ns = latency;
                }
                if (latency < min_latency_ns) {
                    min_latency_ns = latency;
                }
                auto min_latency_str = separate_digits(min_latency_ns);
                auto max_latency_str = separate_digits(max_latency_ns);

                if (samples_buffer_.size() == BATCH) {
                    LOG(INFO) << "min: " << min_latency_str << " max: " << max_latency_str
                              << " nanoseconds";
                    output_file_.write(reinterpret_cast<const char*>(samples_buffer_.data()),
                                       samples_buffer_.size() * sizeof(uint64_t));
                    output_file_.flush();
                    samples_buffer_.clear();
                    samples_buffer_.reserve(BATCH);
                    max_latency_ns = 0;
                }
            }
        }
    }

    void Postprocess(ProcessingContext& _) override {
        if (output_file_.is_open()) {
            output_file_.close();
        }
        samples_buffer_.clear();
    }
};

REGISTERPROCESSOR(LatencyBenchmark)
