#pragma once

#include <sched.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <time.h>
#include <chrono>
#include <thread>
#include "eventdata/eventdata.hpp"
#include "iprocessor.hpp"
#include "utilities/tsc_duration_clock.cpp"

class PCIeParallelOutput : public IProcessor {
   private:
    PortIn<EventType>* event_port_;
    options::Value<EventType::Data, false> target_event_{std::string("default"),
                                                         options::notempty<EventType::Data>()};
    options::Value<unsigned int, false> pin_nr_{5};
    options::Value<unsigned int, false> pulse_width_ns_{1'000'000'000};

   public:
    PCIeParallelOutput() : IProcessor() {
        add_option("target_event", target_event_, "The event to trigger the pulse.");
        add_option("pin_nr", pin_nr_, "The target parallel pin number.");
        add_option("pulse_width_ns", pulse_width_ns_, "Pulse duration in nanoseconds.");
    }

    void CreatePorts() override {
        event_port_ = create_input_port<EventType>("events", EventType::Capabilities(),
                                                   PortInPolicy(SlotRange(1)));
    }

    void Process(ProcessingContext& context) override {
        EventType::Data* data;

        const unsigned short base_addr = 0x3030;
        const unsigned short control_reg = base_addr + 0x002;

        auto delay_ns = pulse_width_ns_();

        unsigned char low = 0x00;
        unsigned char high = 1 << (pin_nr_() - 2);

        if (ioperm(base_addr, 4, 1) < 0) {
            throw ProcessingError("ioperm failed. Run with root privileges.");
        }

        outb(low, control_reg);

        while (!context.terminated()) {
            if (!event_port_->slot(0)->RetrieveData(data)) {
                break;
            }

            event_port_->slot(0)->ReleaseData();

            if (*data == target_event_()) {
                outb(high, base_addr);

                uint64_t pulse_start_tsc = TscDurationClock::tsc();
                while (true) {
                    std::chrono::nanoseconds elapsed =
                        TscDurationClock::duration_since_tsc(pulse_start_tsc);
                    if (elapsed.count() >= delay_ns) {
                        break;
                    }

                    auto remaining = std::chrono::nanoseconds(delay_ns) - elapsed;
                    if (remaining.count() > 2000) {
                        auto sleep_duration = (remaining * 95) / 100;
                        std::this_thread::sleep_until(std::chrono::steady_clock::now() +
                                                      sleep_duration);
                    } else {
                        asm volatile("pause" ::: "memory");
                    }
                }

                outb(low, base_addr);
            }
        }

        outb(low, base_addr);
        ioperm(base_addr, 4, 0);
    }
};

REGISTERPROCESSOR(PCIeParallelOutput)
