#include <App.h>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string_view>
#include <thread>
#include <unordered_set>
#include <vector>
#include "iprocessor.hpp"
#include "serialization.hpp"
#include "utilities/tsc_duration_clock.cpp"

class WebsocketSink : public IProcessor {
   private:
    using ClientSet = std::unordered_set<uWS::WebSocket<false, true, std::string>*>;

    PortIn<AnyType>* data_in_port_ = nullptr;
    options::Value<unsigned int, false> port_{5550};

    std::thread server_thread_;
    std::vector<std::thread> worker_threads_;
    std::atomic<bool> running_{false};

    std::atomic<std::shared_ptr<const ClientSet>> active_clients_{
        std::make_shared<const ClientSet>()};
    std::vector<std::string> upstream_address_headers_;

    us_listen_socket_t* listen_socket_ = nullptr;
    uWS::Loop* server_loop_ = nullptr;
    std::mutex loop_mutex_;

   public:
    WebsocketSink() : IProcessor() {
        add_option("port", port_, "Network port for WebSocket server.", false);
    }

    ~WebsocketSink() {
        running_ = false;
        stop_server_();
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
        upstream_address_headers_.resize(nslots);

        for (decltype(nslots) k = 0; k < nslots; ++k) {
            auto slot = data_in_port_->slot(k);
            std::string upstream_addr =
                slot->upstream_address().processor() + "." + slot->upstream_address().port();

            uint8_t upstream_addr_len =
                static_cast<uint8_t>(std::min<size_t>(upstream_addr.size(), 255));

            std::string header;
            header.push_back(static_cast<char>(upstream_addr_len));
            header.append(upstream_addr.data(), upstream_addr_len);

            upstream_address_headers_[k] = std::move(header);
        }

        running_ = true;

        server_thread_ = std::thread([this, listen_port, proc_name]() {
            uWS::App app;
            uWS::App::WebSocketBehavior<std::string> behavior;

            behavior.compression = uWS::DISABLED;
            behavior.maxPayloadLength = 16 * 1024 * 1024;
            behavior.idleTimeout = 16;
            behavior.maxBackpressure = 4 * 1024 * 1024;
            behavior.closeOnBackpressureLimit = true;

            behavior.open = [this, proc_name](auto* ws) {
                auto new_set = std::make_shared<ClientSet>(*active_clients_.load());
                new_set->insert(ws);
                active_clients_.store(new_set);
                LOG(INFO) << proc_name << ": Client connected. Active total: " << new_set->size();
            };

            behavior.close = [this, proc_name](auto* ws, int, std::string_view) {
                auto new_set = std::make_shared<ClientSet>(*active_clients_.load());
                new_set->erase(ws);
                active_clients_.store(new_set);
                LOG(INFO) << proc_name
                          << ": Client disconnected. Active total: " << new_set->size();
            };

            behavior.message = [](auto* _, std::string_view, uWS::OpCode) {};

            app.ws<std::string>("/*", std::move(behavior));

            app.listen("0.0.0.0", listen_port, [this, proc_name, listen_port](auto* listen_socket) {
                std::lock_guard<std::mutex> lock(loop_mutex_);
                if (listen_socket) {
                    listen_socket_ = listen_socket;
                    server_loop_ = uWS::Loop::get();
                    LOG(INFO) << proc_name << ": Universal Push Broadcaster listening on 0.0.0.0:"
                              << listen_port;
                } else {
                    LOG(ERROR) << "Failed to bind WebSocket server port.";
                }
            });

            if (running_) {
                app.run();
            }
        });
    }

    void Process(ProcessingContext& context) override {
        auto nslots = data_in_port_->number_of_slots();

        for (decltype(nslots) k = 0; k < nslots; ++k) {
            worker_threads_.emplace_back([this, k, &context]() {
                AnyType::Data* data_in = nullptr;
                std::stringstream thread_buffer;
                auto* slot = data_in_port_->slot(k);

                while (running_ && !context.terminated()) {
                    if (!slot->RetrieveData(data_in)) {
                        break;
                    }

                    if (data_in == nullptr) {
                        continue;
                    }

                    auto clients_snapshot = active_clients_.load();

                    if (!clients_snapshot->empty()) {
                        thread_buffer.str("");
                        thread_buffer.clear();

                        thread_buffer.write(upstream_address_headers_[k].data(),
                                            upstream_address_headers_[k].size());
                        data_in->SerializeBinary(thread_buffer, Serialization::Format::COMPACT);

                        auto shared_payload = std::make_shared<std::string>(thread_buffer.str());

                        uWS::Loop* target_loop = nullptr;
                        {
                            std::lock_guard<std::mutex> lock(loop_mutex_);
                            target_loop = server_loop_;
                        }

                        if (target_loop) {
                            // Defer executes strictly within the event loop thread
                            target_loop->defer([this, clients_snapshot, shared_payload]() {
                                // Double check against the most current active list
                                auto current_active = active_clients_.load();
                                for (auto* ws : *clients_snapshot) {
                                    if (current_active->count(ws)) {
                                        if (ws->getBufferedAmount() < 2 * 1024 * 1024) {
                                            ws->send(*shared_payload, uWS::OpCode::BINARY);
                                        }
                                    }
                                }
                            });
                        }
                    }

                    slot->ReleaseData();
                }
            });
        }

        while (!context.terminated() && running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    void Postprocess(ProcessingContext& _) override {
        running_ = false;

        stop_server_();
        join_threads_();
        reset_state_();
    }

   private:
    void stop_server_() {
        std::lock_guard<std::mutex> lock(loop_mutex_);

        auto empty_set = std::make_shared<const ClientSet>();
        auto clients_to_close = active_clients_.exchange(empty_set);

        if (listen_socket_) {
            us_listen_socket_close(0, listen_socket_);
            listen_socket_ = nullptr;
        }

        if (server_loop_) {
            if (clients_to_close && !clients_to_close->empty()) {
                server_loop_->defer([clients_to_close]() {
                    for (auto* ws : *clients_to_close) {
                        ws->close();
                    }
                });
            }
            server_loop_ = nullptr;
        }
    }

    void join_threads_() {
        for (auto& th : worker_threads_) {
            if (th.joinable()) {
                th.join();
            }
        }
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }

    void reset_state_() {
        worker_threads_.clear();
        upstream_address_headers_.clear();
        active_clients_.store(std::make_shared<const ClientSet>());
    }
};

REGISTERPROCESSOR(WebsocketSink)
