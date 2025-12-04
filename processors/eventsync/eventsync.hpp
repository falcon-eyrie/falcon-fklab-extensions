// ---------------------------------------------------------------------
// This file is part of falcon-core.
//
// Copyright (C) 2015, 2016, 2017 Neuro-Electronics Research Flanders
//
// Falcon-server is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Falcon-server is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with falcon-core. If not, see <http://www.gnu.org/licenses/>.
// ---------------------------------------------------------------------

#pragma once

#include <string>

#include "eventdata/eventdata.hpp"
#include "iprocessor.hpp"
#include "utilities/general.hpp"
#include "utilities/time.hpp"

class EventSync : public IProcessor {
    // CONSTRUCTOR and OVERLOADED METHODS
  public:
    EventSync();
    void CreatePorts() override;
    void Process(ProcessingContext &context) override;
    void Postprocess(ProcessingContext &context) override;

    // METHODS
  protected:
    /* Keep in memory the latest source and hardware timestamps
     * @input data_in Current event data from the input port
     */
    void update_latest_ts(EventType::Data *data_in);

    /*
     * During post-processing, log as info if all counters are consistent
     * and as warning if not, all counters used: number of events received,
     * with the number of targeted, and non-target and finally reset them.
     *
     * @input port_name name of the input port
     * @input counter counter structure with three counters to log
     * (all_received, target, non_target)
     */
    void log_and_reset_counters(std::string port_name, EventCounter &counter);

    // DATA PORTS
  protected:
    PortIn<EventType> *data_in_port_;
    PortOut<EventType> *data_out_port_;

    // variables
  protected:
    EventCounter event_counter_;
    uint64_t n_events_synced_;
    TimestampRegister timestamps_;

    // OPTIONS
  protected:
    options::Value<EventType::Data, false> target_event_{
        DEFAULT_EVENT, options::notempty<EventType::Data>()};
};
