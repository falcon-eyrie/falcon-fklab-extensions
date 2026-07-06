#include <App.h>
#include <atomic>
#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_set>
#include <vector>

#include "g3log/g3log.hpp"
#include "logging/g3loglevels.hpp"

struct SinkSlotAccumulator {
    std::string upstream_address;
    std::mutex mutex;
    std::vector<char> active_bytes;
    std::vector<char> flush_bytes;

    SinkSlotAccumulator() = default;

    SinkSlotAccumulator(const SinkSlotAccumulator&) = delete;
    SinkSlotAccumulator& operator=(const SinkSlotAccumulator&) = delete;

    SinkSlotAccumulator(SinkSlotAccumulator&& other) noexcept {
        std::lock_guard<std::mutex> lock(other.mutex);
        upstream_address = std::move(other.upstream_address);
        active_bytes = std::move(other.active_bytes);
        flush_bytes = std::move(other.flush_bytes);
    }

    SinkSlotAccumulator& operator=(SinkSlotAccumulator&& other) noexcept {
        if (this != &other) {
            std::lock_guard<std::mutex> lock1(mutex);
            std::lock_guard<std::mutex> lock2(other.mutex);
            upstream_address = std::move(other.upstream_address);
            active_bytes = std::move(other.active_bytes);
            flush_bytes = std::move(other.flush_bytes);
        }
        return *this;
    }
};

struct ThreadBuffer {
    std::ostringstream stream;
};

using ClientSet = std::unordered_set<uWS::WebSocket<false, true, std::string>*>;

class UWSServerController {
   public:
    UWSServerController() = default;

    ~UWSServerController() { stop(); }

    void start(unsigned int port, const std::string& proc_name, std::atomic<bool>& running) {
        server_thread_ = std::thread([this, port, proc_name, &running]() {
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

            app.listen("0.0.0.0", port, [this, proc_name, port](auto* listen_socket) {
                std::lock_guard<std::mutex> lock(loop_mutex_);
                if (listen_socket) {
                    listen_socket_ = listen_socket;
                    server_loop_ = uWS::Loop::get();
                    LOG(INFO) << proc_name << ": Websocket active on ws://0.0.0.0:" << port;
                } else {
                    LOG(ERROR) << "Failed to bind WebSocket server port.";
                }
            });

            if (running) {
                app.run();
            }
        });
    }

    void stop() {
        std::lock_guard<std::mutex> lock(loop_mutex_);
        auto empty_set = std::make_shared<const ClientSet>();
        auto clients_to_close = active_clients_.exchange(empty_set);

        if (server_loop_) {
            server_loop_->defer([listen_sock = listen_socket_, clients_to_close]() {
                if (clients_to_close) {
                    for (auto* ws : *clients_to_close) {
                        ws->close();
                    }
                }
                if (listen_sock) {
                    us_listen_socket_close(0, listen_sock);
                }
            });
            listen_socket_ = nullptr;
            server_loop_ = nullptr;
        }

        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }

    std::shared_ptr<const ClientSet> get_clients_snapshot() { return active_clients_.load(); }

    void broadcast(const std::shared_ptr<const ClientSet>& clients_snapshot,
                   const std::shared_ptr<std::vector<char>>& shared_packet) {
        uWS::Loop* target_loop = nullptr;
        {
            std::lock_guard<std::mutex> lock(loop_mutex_);
            target_loop = server_loop_;
        }

        if (target_loop) {
            target_loop->defer([this, clients_snapshot, shared_packet]() {
                auto current_active = active_clients_.load();
                std::string_view payload(shared_packet->data(), shared_packet->size());

                for (auto* ws : *clients_snapshot) {
                    if (current_active->count(ws)) {
                        if (ws->getBufferedAmount() < 2 * 1024 * 1024) {
                            ws->send(payload, uWS::OpCode::BINARY);
                        }
                    }
                }
            });
        }
    }

   private:
    std::thread server_thread_;
    alignas(std::hardware_destructive_interference_size)
        std::atomic<std::shared_ptr<const ClientSet>> active_clients_{
            std::make_shared<const ClientSet>()};

    us_listen_socket_t* listen_socket_ = nullptr;
    uWS::Loop* server_loop_ = nullptr;
    std::mutex loop_mutex_;
};
