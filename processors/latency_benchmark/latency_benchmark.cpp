#include "latency_benchmark.hpp"

LatencyBenchmark::LatencyBenchmark() : IProcessor() {
}

void LatencyBenchmark::CreatePorts() {
    data_in_port_ = create_input_port<AnyType>("data", AnyType::Capabilities(),
                                               PortInPolicy(SlotRange(1, 256), false));
}

void LatencyBenchmark::Preprocess(ProcessingContext& context) {
    samples_buffer_.reserve(BATCH);

    auto file_name = "latency_benchmark_data.bin";
    auto file_path = context.resolve_path("run://") + "/" + file_name;
    output_file_.open(file_path, std::ios::binary);
    if (!output_file_.is_open()) {
        throw std::runtime_error("Cannot open latency benchmark output file: " + file_path);
    }
}

void LatencyBenchmark::Process(ProcessingContext& context) {
    AnyType::Data* data_in = nullptr;

    while (!context.terminated()) {
        for (SlotType s = 0; s < data_in_port_->number_of_slots(); ++s) {
            if (!data_in_port_->slot(s)->RetrieveData(data_in)) {
                break;
            }
            auto ingestedAt = data_in->ingestion_ns();

            data_in_port_->slot(s)->ReleaseData();

            auto benchmarkedAt = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                     std::chrono::steady_clock::now().time_since_epoch())
                                     .count();

            samples_buffer_.push_back({ingestedAt, benchmarkedAt});
            if (samples_buffer_.size() == BATCH) {
                output_file_.write(reinterpret_cast<const char*>(samples_buffer_.data()),
                                   samples_buffer_.size() * sizeof(LatencySample));
                samples_buffer_.clear();
                samples_buffer_.reserve(BATCH);
            }
        }
    }
}

void LatencyBenchmark::Postprocess(ProcessingContext& context) {
    if (output_file_.is_open()) {
        output_file_.close();
    }
    samples_buffer_.clear();
}

REGISTERPROCESSOR(LatencyBenchmark)
