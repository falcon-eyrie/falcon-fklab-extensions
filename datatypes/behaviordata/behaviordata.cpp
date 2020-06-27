#include "behaviordata.hpp"
#include "utilities/string.hpp"

using namespace nsBehaviorType;

void Data::ClearData() {
    
    linear_position_ = 0;
    speed_ = 0;
}

std::string Data::unit_string() const {
    
    if ( unit_ == BehaviorUnit::CM ) {return CM_S;}
    if ( unit_ == BehaviorUnit::PIXEL ) {return PIXEL_S;}
    return "none";
}

double Data::linear_position() const {
    
    return linear_position_;
}

double Data::speed() const {
    
    return speed_;    
}

double Data::signed_speed() const {
    
    if ( speed_sign_ == dsp::behavior_algorithms::SpeedSign::POSITIVE ) {return speed_;}
    return -speed_;
}

void Data::set_linear_position( double linear_position ) {
    
    linear_position_ = linear_position;
}

void Data::set_speed( double speed ) {
    
    speed_ = speed;
}

void Data::set_speed_sign( dsp::behavior_algorithms::SpeedSign sign ) {
    
    speed_sign_ = sign;
}

void Data::convert_to_pixel( double cm_to_pixel ) {
    
    if ( unit_ == BehaviorUnit::CM ) {
        linear_position_ /= cm_to_pixel; 
        speed_ /= cm_to_pixel;
        unit_ = BehaviorUnit::PIXEL;
    }
}

void Data::convert_to_cm( double pixel_to_cm ) {
    
    if ( unit_ == BehaviorUnit::PIXEL ) {
        linear_position_ /= pixel_to_cm; 
        speed_ /= pixel_to_cm;
        unit_ = BehaviorUnit::CM;
    }
}

void Data::SerializeBinary( std::ostream& stream,
    Serialization::Format format ) const {
    
    Base::Data::SerializeBinary( stream, format );
    if ( format==Serialization::Format::FULL || format==Serialization::Format::COMPACT ) {
        
        stream.write( reinterpret_cast<const char*>( &linear_position_ ),
            sizeof(decltype(linear_position_)) );
        stream.write( reinterpret_cast<const char*>( &speed_ ),
            sizeof(decltype(speed_)) );
        std::string buffer = unit_string();
        buffer.resize( UNIT_S_SIZE );
        stream.write( buffer.data(), UNIT_S_SIZE );
        
        bool speed_sign_b = false;
        if ( speed_sign_ == dsp::behavior_algorithms::SpeedSign::POSITIVE ) {
            speed_sign_b = true;
        }
        stream.write( reinterpret_cast<const char*>( &speed_sign_b ),
            sizeof(speed_sign_b) );
    }
}

void Data::SerializeYAML( YAML::Node & node,
    Serialization::Format format ) const {
    
    Base::Data::SerializeYAML( node, format );
    if ( format==Serialization::Format::FULL || format==Serialization::Format::COMPACT ) {
        node[POSITION_S] = linear_position_;
        node[SPEED_S] = speed_;
        node[UNIT_S] = unit_string();
        if ( speed_sign_ == dsp::behavior_algorithms::SpeedSign::POSITIVE ) {
            node[SPEED_SIGN_S] = true;
        } else {
            node[SPEED_SIGN_S] = false;
        }
    }
}

void Data::YAMLDescription( YAML::Node & node, Serialization::Format format ) const {
    
    Base::Data::YAMLDescription( node, format );
    if ( format==Serialization::Format::FULL || format==Serialization::Format::COMPACT ) {
        node.push_back( POSITION_S + " " + get_type_string<double>() + " (1)" );
        node.push_back( SPEED_S + " " + get_type_string<double>() + " (1)" );
        node.push_back( UNIT_S + " string (" + std::to_string( UNIT_S_SIZE ) + ")" );
        node.push_back( SPEED_SIGN_S + " " + get_type_string<bool>() + " (1)" );
    }
}