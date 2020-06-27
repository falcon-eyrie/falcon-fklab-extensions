/* 
 * File:   behaviordata.hpp
 * Author: davide
 *
 * Created on 08 February 2016, 10:55
 */

#ifndef BEHAVIORDATA_HPP
#define	BEHAVIORDATA_HPP

#include "idata.hpp"
#include "dsp/behavior_algorithms.hpp"

#include <array>

enum BehaviorUnit {
    PIXEL,
    CM
};

const std::string PIXEL_S = "pixel";
const std::string CM_S = "cm";
const std::size_t UNIT_S_SIZE = 5;


namespace nsBehaviorType {

using Base = AnyType;

struct Parameters : Base::Parameters {
    Parameters(BehaviorUnit unit = BehaviorUnit::CM, double sample_rate=0) : Base::Parameters(), default_unit(unit), default_sample_rate(sample_rate){}

    BehaviorUnit default_unit;
    double default_sample_rate;

};

class Capabilities : public Base::Capabilities {
public:
    virtual void Validate( const Parameters & parameters ) const {
       Base::Capabilities::Validate(parameters);
       if ( parameters.default_unit != BehaviorUnit::CM and parameters.default_unit != BehaviorUnit::PIXEL ) {throw std::runtime_error( "Unit is either pixel or cm");}
    }
};

class Data : public Base::Data {

public:

    Data( BehaviorUnit unit = BehaviorUnit::CM, double sample_rate=0) {

        Initialize( unit, sample_rate);
    };

    void Initialize( const Parameters & parameters ) {
        Initialize(parameters.default_unit, parameters.default_sample_rate);
    };

    void Initialize( BehaviorUnit unit, double sample_rate){
        unit_ = unit;
        sample_rate_ = sample_rate;
    };


    virtual void ClearData() override;

    BehaviorUnit unit() const {return unit_;};
    double sample_rate() const { return sample_rate_;};

    std::string unit_string() const;
    
    void set_linear_position( double linear_position );
    double linear_position() const;
    
    void set_speed( double speed );
    void set_speed_sign( dsp::behavior_algorithms::SpeedSign sign );
    double speed() const;
    double signed_speed() const;
    
    void set_head_direction( bool head_direction_ );
    bool head_direction() const;
    
    void convert_to_pixel( double cm_to_pixel );
    void convert_to_cm( double pixel_to_cm );
    
    virtual void SerializeBinary( std::ostream& stream,
        Serialization::Format format = Serialization::Format::FULL ) const override;
    virtual void SerializeYAML( YAML::Node & node,
        Serialization::Format format = Serialization::Format::FULL ) const  override;
    virtual void YAMLDescription( YAML::Node & node,
        Serialization::Format format = Serialization::Format::FULL ) const override;
    
protected:
    BehaviorUnit unit_;
    double sample_rate_;
    double linear_position_;
    double speed_;
    dsp::behavior_algorithms::SpeedSign speed_sign_;
    
protected:
    std::string POSITION_S = "linear_position";
    std::string SPEED_S = "speed";
    std::string UNIT_S = "unit";
    std::string SPEED_SIGN_S = "speed_sign";
};

}

class BehaviorType {
public:
    static const std::string datatype() { return "behavior"; }
    static const std::string dataname() { return "behavior"; }

    using Base = nsBehaviorType::Base;
    using Parameters = nsBehaviorType::Parameters;
    using Capabilities = nsBehaviorType::Capabilities;
    using Data = nsBehaviorType::Data;
};


#endif	// behaviordata.hpp

