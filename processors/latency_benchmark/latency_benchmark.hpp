#pragma once

#include <string>

#include "iprocessor.hpp"
#include "utilities/general.hpp"
#include "utilities/time.hpp"

struct LatencySample {
    int64_t ingested_ns;
    int64_t benchmarked_ns;
};

class LatencyBenchmark : public IProcessor {
    // CONSTRUCTOR and OVERLOADED METHODS
   public:
    LatencyBenchmark();
    void CreatePorts() override;
    void Preprocess(ProcessingContext& context) override;
    void Process(ProcessingContext& context) override;
    void Postprocess(ProcessingContext& context) override;

    // CONSTANTS
   public:
    static constexpr size_t BATCH = 128;

    // DATA PORTS
   protected:
    PortIn<AnyType>* data_in_port_;

    // VARIABLES
   protected:
    std::ofstream output_file_;
    std::vector<LatencySample> samples_buffer_;
};
