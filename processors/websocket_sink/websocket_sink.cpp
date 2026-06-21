#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <memory>
#include <mutex>
#include <sstream>
#include <string_view>
#include <thread>
#include <vector>
#include "iprocessor.hpp"
#include "serialization.hpp"
#include "uws_server.cpp"

class WebsocketSink : public IProcessor {
   private:
    PortIn<AnyType>* data_in_port_ = nullptr;
    options::Value<unsigned int, false> port_{5550};
    options::Value<unsigned int, false> interval_ms_{1000};

    UWSServerController uws_server_controller_;
    std::vector<std::thread> worker_threads_;

    alignas(std::hardware_destructive_interference_size) std::atomic<bool> running_{false};
    std::vector<SinkSlotAccumulator> slot_accumulators_;

   public:
    WebsocketSink() : IProcessor() {
        add_option("port", port_, "Network port for WebSocket server.", false);
        add_option("interval", interval_ms_, "Batch publish interval (ms)", false);
    }

    ~WebsocketSink() {
        running_ = false;
        uws_server_controller_.stop();
        join_threads_();
    }

    void CreatePorts() override {
        data_in_port_ = create_input_port<AnyType>("input", AnyType::Capabilities(),
                                                   PortInPolicy(SlotRange(0, 256)));
    }

    void Prepare(GlobalContext& _) override { reset_state_(); }

    void Preprocess(ProcessingContext& _) override {
        unsigned int listen_port = port_();
        std::string proc_name = name();

        auto nslots = data_in_port_->number_of_slots();

        slot_accumulators_.clear();
        slot_accumulators_.reserve(nslots);

        for (decltype(nslots) k = 0; k < nslots; ++k) {
            auto slot = data_in_port_->slot(k);
            std::string upstream_addr =
                slot->upstream_address().processor() + "." + slot->upstream_address().port();

            SinkSlotAccumulator acc;
            acc.upstream_address = std::move(upstream_addr);

            slot_accumulators_.push_back(std::move(acc));
        }

        running_ = true;
        uws_server_controller_.start(listen_port, proc_name, running_);
    }

    void Process(ProcessingContext& context) override {
        auto nslots = data_in_port_->number_of_slots();

        for (decltype(nslots) k = 0; k < nslots; ++k) {
            worker_threads_.emplace_back([this, k, &context]() {
                AnyType::Data* data_in = nullptr;
                auto* slot = data_in_port_->slot(k);
                auto thread_buffer = std::make_shared<ThreadBuffer>();
                auto& acc = slot_accumulators_[k];

                while (running_ && !context.terminated()) {
                    if (!slot->RetrieveData(data_in)) {
                        break;
                    }

                    if (data_in == nullptr) {
                        continue;
                    }

                    thread_buffer->stream.clear();
                    thread_buffer->stream.str("");

                    data_in->SerializeBinary(thread_buffer->stream, Serialization::Format::COMPACT);

                    std::string_view frame = thread_buffer->stream.view();
                    if (!frame.empty()) {
                        std::lock_guard<std::mutex> lock(acc.mutex);
                        acc.active_bytes.insert(acc.active_bytes.end(), frame.begin(), frame.end());
                    }

                    slot->ReleaseData();
                }
            });
        }

        while (!context.terminated() && running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms_()));
            publish_batch_();
        }
    }

    void Postprocess(ProcessingContext& _) override {
        running_ = false;
        uws_server_controller_.stop();
        join_threads_();
        reset_state_();
    }

   private:
    void publish_batch_() {
        auto clients_snapshot = uws_server_controller_.get_clients_snapshot();
        if (clients_snapshot->empty()) {
            return;
        }

        size_t total_payload_size = 0;
        for (auto& acc : slot_accumulators_) {
            {
                std::lock_guard<std::mutex> lock(acc.mutex);
                std::swap(acc.active_bytes, acc.flush_bytes);
            }
            if (!acc.flush_bytes.empty()) {
                total_payload_size +=
                    (1 + acc.upstream_address.size() + 4 + acc.flush_bytes.size());
            }
        }

        if (total_payload_size > 0) {
            std::vector<char> packet_buffer;
            packet_buffer.reserve(total_payload_size);

            for (auto& acc : slot_accumulators_) {
                if (acc.flush_bytes.empty()) {
                    continue;
                }

                uint8_t addr_len =
                    static_cast<uint8_t>(std::min<size_t>(acc.upstream_address.size(), 255));
                packet_buffer.push_back(static_cast<char>(addr_len));
                packet_buffer.insert(packet_buffer.end(), acc.upstream_address.begin(),
                                     acc.upstream_address.begin() + addr_len);

                uint32_t payload_len = static_cast<uint32_t>(acc.flush_bytes.size());
                const char* len_ptr = reinterpret_cast<const char*>(&payload_len);
                packet_buffer.insert(packet_buffer.end(), len_ptr, len_ptr + 4);

                packet_buffer.insert(packet_buffer.end(), acc.flush_bytes.begin(),
                                     acc.flush_bytes.end());
                acc.flush_bytes.clear();
            }

            auto shared_packet = std::make_shared<std::vector<char>>(std::move(packet_buffer));
            uws_server_controller_.broadcast(clients_snapshot, shared_packet);
        }
    }

    void join_threads_() {
        for (auto& th : worker_threads_) {
            if (th.joinable()) {
                th.join();
            }
        }
    }

    void reset_state_() {
        worker_threads_.clear();
        slot_accumulators_.clear();
    }
};

REGISTERPROCESSOR(WebsocketSink)
