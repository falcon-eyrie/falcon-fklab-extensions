#include "serialoutput.hpp"

#include "g3log/src/g2log.hpp"
#include "arduinoserial/arduino-serial-lib.h"
#include <iostream>
#include <fstream>

SerialOutput::SerialOutput(){
    add_option(ENABLED_S , default_enabled_, "");
    add_option(LOCKOUT_PERIOD_S, initial_lockout_period_, "");
    add_option("enable saving", save_stim_events_, "");
    add_option("port address", port_address_, "");
    add_option(MESSAGE_S, default_message_, "");
    add_option("target event", target_event_, "");
    add_option("baud rate", baudrate_, "");
    add_option("print protocol execution updates",print_transmission_updates_, "");


}

void SerialOutput::CreatePorts() {
    
    data_in_port_ = create_input_port<EventType>(
        EVENTDATA,
        EventType::Capabilities(),
        PortInPolicy( SlotRange(1) ) );
    
    enabled_ = create_static_state(
        ENABLED_S ,
        default_enabled_(),
        true,
        Permission::WRITE);
    
    lockout_period_ = create_static_state(
        LOCKOUT_PERIOD_S,
        initial_lockout_period_(),
        true,
        Permission::WRITE);
    
    message_ = create_static_state(
        MESSAGE_S,
        default_message_(),
        true,
        Permission::WRITE);
}

void SerialOutput::Preprocess( ProcessingContext& context ) {
    
    // reset counters and logs
    nreceived_events_ = 0;
    ntarget_events_ = 0;
    nprotocol_executions_ = 0;
    n_locked_out_events_ = 0;
    previous_TS_nostim_ = std::numeric_limits<uint64_t>::min();
    
    if ( context.test() ) {
        prepare_latency_test( context );
    }
    
    fd_ = serialport_init( port_address_().c_str(), baudrate_() );
    LOG(INFO) << "Serial port " << port_address_() << " opened.";
}
    
void SerialOutput::Process( ProcessingContext& context ) {
     
    EventType::Data *data_in = nullptr;
    uint64_t ts;
    
    std::string path = context.resolve_path( "run://", "run" );
    auto prefix = path + name();
    std::string filename;
    char message;
    
    while (!context.terminated()) {

        if (!data_in_port_->slot(0)->RetrieveData( data_in )) {break;}
        ++ nreceived_events_;

        // select and execute protocol based on event name

        if (enabled_->get() && target_event_() == *data_in ) {

            ++ntarget_events_;

            if ( not to_lock_out( data_in->hardware_timestamp() ) ) {
                    
                if ( context.test() ) {
                    test_source_timestamps_[nprotocol_executions_] = Clock::now();
                }

                message = message_->get();
                if ( (serialport_write( fd_, &message)) != 0 ) {
                    LOG(ERROR) <<  name() << ". Serial message " << message <<
                            " not delivered.";
                } else {
                    ++ nprotocol_executions_;
                    LOG_IF(UPDATE, print_transmission_updates_()) << name()
                        << ". Message " << message << " transmitted serially for "
                            << data_in->event() << " event.";
                }
                
                if ( save_stim_events_() ) { //save stim events to disk
                    
                    filename = STIM_EVENT_S + data_in->event();
                    // filename will also be the key to the container of files
                    // check if this type of event has been saved before
                    if ( streams_.count( STIM_EVENT_S + data_in->event() ) == 0 ) {
                        create_file( prefix, filename );
                    }
                    ts = data_in->serial_number();
                    streams_[filename]->write(
                        reinterpret_cast<const char*>( &ts ), sizeof( decltype( ts ) ) );
                }

            } else {
                ++ n_locked_out_events_;
            }
        }

       data_in_port_->slot(0)->ReleaseData();
   }
    
 }

void SerialOutput::Postprocess( ProcessingContext& context ) {
    
    LOG(INFO) << name() << ". Received " << nreceived_events_ <<
        " events, of which " << ntarget_events_ <<
        " were targets. Successfully executed stimulation protocol " <<
        nprotocol_executions_ << " times out of " << ntarget_events_ << ". " <<
        n_locked_out_events_ << " executions of the stimulation protocol were locked out.";
    
    if ( context.test() ) {
        save_source_timestamps_to_disk( nprotocol_executions_ );
    } 
    
    serialport_close( fd_ );
}

bool SerialOutput::to_lock_out( const uint64_t current_timestamp ) {
    
    if ( current_timestamp < previous_TS_nostim_ ) {
        throw ProcessingError( "Non-sequential stimulation event timestamp.", name() );
    }
    delta_TS_ = (current_timestamp - previous_TS_nostim_) / 1000;

    if ( delta_TS_ <= lockout_period_->get() ) { return true;}
    
    previous_TS_nostim_ = current_timestamp;
    return false;
}

REGISTERPROCESSOR(SerialOutput)
