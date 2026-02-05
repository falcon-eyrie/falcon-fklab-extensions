// // ---------------------------------------------------------------------
// // This file is part of falcon-core.
// //
// // Copyright (C) 2015, 2016, 2017 Neuro-Electronics Research Flanders
// //
// // Falcon-server is free software: you can redistribute it and/or modify
// // it under the terms of the GNU General Public License as published by
// // the Free Software Foundation, either version 3 of the License, or
// // (at your option) any later version.
// //
// // Falcon-server is distributed in the hope that it will be useful,
// // but WITHOUT ANY WARRANTY; without even the implied warranty of
// // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// // GNU General Public License for more details.
// //
// // You should have received a copy of the GNU General Public License
// // along with falcon-core. If not, see <http://www.gnu.org/licenses/>.
// // ---------------------------------------------------------------------

// #include "dummysink.hpp"

// #include <chrono>
// #include <string>
// #include <vector>

// #include "idata.hpp"
// #include "timeseriesdata/timeseriesdata.hpp"
// #include "utilities/time.hpp"

// void DummySink::CreatePorts() {
//     data_port_ =
//         create_input_port<AnyType>("data", AnyType::Capabilities(), PortInPolicy(SlotRange(1)));

//     tickle_state_ = create_static_state("tickle", false, true, Permission::WRITE);
//     expose_method("kick", &DummySink::Kick);
// }

// YAML::Node DummySink::Kick(const YAML::Node& node) {
//     LOG(INFO) << name() << " says: I got kicked!";
//     return YAML::Node();
// }

// void DummySink::Process(ProcessingContext& context) {
//     uint64_t packet_counter = 0;
//     uint64_t retrieve_counter = 0;
//     std::vector<AnyType::Data*> data;
//     auto address = data_port_->slot(0)->upstream_address();

//     LOG(DEBUG) << "slot is connected to " << address.string();

//     auto start = Clock::now();
//     bool tickling = false;

//     while (!context.terminated()) {
//         if (!data_port_->slot(0)->RetrieveDataAll(data)) {
//             LOG(DEBUG) << name() << " : received finish signal while waiting for data!";
//             break;
//         }

//         ++retrieve_counter;
//         packet_counter += data.size();
//         data_port_->slot(0)->ReleaseData();

//         if (tickle_state_->get() != tickling) {
//             tickling = !tickling;
//             if (tickling) {
//                 LOG(INFO) << "Hi hi, that tickles!";
//             } else {
//                 LOG(INFO) << "Why stop tickling?";
//             }
//         }
//     }

//     std::chrono::milliseconds runtime(
//         std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start));

//     LOG(UPDATE) << name() << ": retrieved " << packet_counter << " packets in " <<
//     retrieve_counter
//                 << " batches over " << static_cast<double>(runtime.count()) / 1000.0 << "seconds
//                 ("
//                 << static_cast<double>(packet_counter) / retrieve_counter << " packets/batch and
//                 "
//                 << packet_counter / (static_cast<double>(runtime.count()) / 1000.0)
//                 << " packets/second).";
// }

// REGISTERPROCESSOR(DummySink)

// Temporarily lives in this file, will be moved to its own folder later

#include "iprocessor.hpp"

#include <dsp/filter.hpp>
#include <vector>

#include "eventdata/eventdata.hpp"
#include "iprocessor.hpp"
#include "timeseriesdata/timeseriesdata.hpp"

class ChainProcessor : public IProcessor {
   public:
    ChainProcessor() : IProcessor() {}

    void insertChainProcessor(std::unique_ptr<IProcessor> processor) override {
        chainedProcessors_.push_back(std::move(processor));
    }

    void CreatePorts() override {
        in_port_ = create_input_port<TimeSeriesType<double>>(
            "input", TimeSeriesType<double>::Capabilities(ChannelRange(1, 384)),
            PortInPolicy(SlotRange(0, 384)));

        out_port_ = create_output_port<EventType>("output", EventType::Parameters(),
                                                  PortOutPolicy(SlotRange(1)));
    }

    void CompleteStreamInfo() override {
        if (in_port_->number_of_slots() != out_port_->number_of_slots()) {
            auto err_msg = "Number of output slots (" +
                           std::to_string(out_port_->number_of_slots()) + ") on port '" +
                           out_port_->name() + "' does not match number of input slots (" +
                           std::to_string(in_port_->number_of_slots()) + ") on port '" +
                           in_port_->name() + "'.";
            throw ProcessingStreamInfoError(err_msg, name());
        }

        for (int k = 0; k < in_port_->number_of_slots(); ++k) {
            out_port_->streaminfo(k).set_stream_parameters(in_port_->streaminfo(k));

            // out_port_->streaminfo(k).set_parameters(in_port_->prototype(k).parameters());
        }
    }
    void Process(ProcessingContext& context) override {
        TimeSeriesType<double>::Data* data_in = nullptr;
        EventType::Data* data_out = nullptr;

        std::vector<IProcessor*> filters;
        IProcessor* detector = nullptr;

        for (auto& proc : chainedProcessors_) {
            if (proc->type() == "MultiChannelFilter") {
                filters.push_back(proc.get());
            } else if (proc->type() == "LevelCrossingDetector") {
                detector = proc.get();
            }
        }

        // std::vector<double> current_out;

        // std::vector<double> buffer(1000);  // temporary buffer for processing, buffer size should
        //                                    // be calculated via input size * some constant
        // std::vector<double> workspace{buffer};

        while (!context.terminated()) {
            uint64_t sync_start = __rdtsc();
            if (!in_port_->slot(0)->RetrieveData(data_in)) break;
            data_out = out_port_->slot(0)->ClaimData(false);

            uint64_t sync_end = __rdtsc();

            uint64_t work_start = __rdtsc();

            std::vector<double> current_in{data_in->data()};

            std::vector<double> current_out = std::vector<double>(current_in.size());

            for (auto* filter : filters) {
                filter->ExecuteStep(current_in, current_out);

                current_in = current_out;
            }

            detector->ExecuteStep(current_in, data_out);

            filters[0]->ExecuteStep(current_in, current_out);
            current_in = current_out;
            filters[1]->ExecuteStep(current_in, current_out);
            current_in = current_out;
            detector->ExecuteStep(current_in, data_out);

            data_out->CloneTimestamps(*data_in);
            data_out->set_serial_number(data_in->serial_number());
            data_out->forward_ingestion_ns(*data_in);
            out_port_->slot(0)->PublishData();
            in_port_->slot(0)->ReleaseData();
            uint64_t work_end = __rdtsc();
            record_metrics(sync_end - sync_start, work_end - work_start);
        }
    }

    void Postprocess(ProcessingContext& _) override { dump_benchmarks(); }

   private:
    std::vector<std::unique_ptr<IProcessor>> chainedProcessors_;
    PortIn<TimeSeriesType<double>>* in_port_;
    PortOut<EventType>* out_port_;
};

REGISTERPROCESSOR(ChainProcessor)