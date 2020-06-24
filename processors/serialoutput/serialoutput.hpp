/* SerialOutput: takes an EventData stream and sends data over serial port
 * when a target event is received
 * 
 * input ports:
 * events <EventData> (1 slot)
 *
 * output ports:
 * none
 *
 * exposed states:
 * enabled <bool> - enable/disable digital output
 *
 * exposed methods:
 * none
 *
 * options:
 * enabled <bool> - default for enabled state
 * port_address <string> - address serial port
 * message <string> - default message sent to the serial port
 * baudrate <int> - baudrate serial communication
 * target_event <string> - target event
 * lockout_period_ms <int> - initial lockout period [ms]  
 */

#ifndef SERIALOUTPUT_HPP
#define SERIALOUTPUT_HPP

#include "iprocessor.hpp"
#include "eventdata/eventdata.hpp"
#include "utilities/time.hpp"
#include "options/options.hpp"
#include "options/units.hpp"


#include <fstream>

class SerialOutput : public IProcessor {

// CONSTRUCTOR and OVERLOADED METHODS
public:
    SerialOutput();
    virtual void CreatePorts() override;
    virtual void Preprocess( ProcessingContext& context ) override;
    virtual void Process( ProcessingContext& context ) override;
    virtual void Postprocess( ProcessingContext& context ) override;

// METHODS
protected:
    bool to_lock_out( const uint64_t current_timestamp );

// DATA PORTS AND STATES
protected:
    PortIn<EventType>* data_in_port_;
    StaticState<bool>* enabled_;
    StaticState<double>* lockout_period_;
    StaticState<char>* message_;

// OPTIONS
protected:

    options::Bool default_enabled_{true};
    options::Measurement<double,false>  initial_lockout_period_{
        50,
        "ms",
        options::positive<double>(true)
    };

    options::Value<char, false> default_message_{'1'};
    options::Bool save_stim_events_{false};
    options::String port_address_{"/dev/ttyACM0"};
    options::Int baudrate_{9600};
    options::Value<EventType::Data,false> target_event_{
        DEFAULT_EVENT,
        options::notempty<EventType::Data>()};
    options::Bool print_transmission_updates_{true};

// variables
protected:

    int fd_;
    uint64_t nreceived_events_;
    uint64_t ntarget_events_;
    uint64_t nprotocol_executions_;
    uint64_t n_locked_out_events_;
    
    uint64_t previous_TS_nostim_;
    uint64_t delta_TS_;


// CONSTANT
protected:
    const std::string STIM_EVENT_S = "stim_";
    const std::string ENABLED_S = "enabled";
    const std::string LOCKOUT_PERIOD_S = "lockout period";
    const std::string MESSAGE_S = "message";
};

#endif // serialoutput.hpp
