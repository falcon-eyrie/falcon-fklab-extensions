#ifndef LIKELIHOODDATA_HPP
#define	LIKELIHOODDATA_HPP

#include "idata.hpp"
#include <valarray> 
#include "utilities/string.hpp"
#include "utilities/math_numeric.hpp"

const size_t HIGH_GRID_SIZE = 1000;

namespace nsLikelihoodType {

using Base = AnyType;

struct Parameters : Base::Parameters {
    Parameters(size_t size=1) : Base::Parameters(), default_grid_size(size){}

    size_t default_grid_size;

};


class Capabilities : public Base::Capabilities {
public:
    virtual void Validate( const Parameters & parameters ) const {
       Base::Capabilities::Validate(parameters);
       if ( parameters.default_grid_size == 0 ) {
            throw std::runtime_error( "Grid size must be different from zero");
       }
       if ( parameters.default_grid_size > HIGH_GRID_SIZE ) {
            throw std::runtime_error( "Grid size of " + std::to_string(parameters.default_grid_size) + " might be too high." );

       }
    }
};

class Data : public Base::Data {

public:

    Data( size_t grid_size=1) {

        Initialize( grid_size);
    };

    void Initialize( const Parameters & parameters ) {
        Initialize(parameters.default_grid_size);
    };

    void Initialize( size_t grid_size=1);

    virtual void ClearData() override {

        n_spikes_ = 0;
        log_likelihood_.resize(grid_size());
        log_likelihood_ = DEFAULT_LOG_LIKELIHOOD_VALUE;
        likelihood_is_updated_ = true; // default_likelihood = exp(default_log_likelihood)
        cached_likelihood_.resize(grid_size());
        cached_likelihood_ = DEFAULT_LIKELIHOOD_VALUE;
        cached_integral_likelihood_ = cached_likelihood_.sum();
    };

    //friend double& operator[](const Data& log_likelihood, const size_t idx) ; // no [] access to likelihood in regular space; remove it if creates confusion

    size_t grid_size() const { return grid_size_; };
    double time_bin() const { return time_bin_; };

    void set_time_bin( double time_bin ) {
       if ( time_bin <= 0 ) {
            throw std::runtime_error( "Bin size must be positive ");
       }
       time_bin_ = time_bin;
    }

    unsigned int n_spikes() const;
    void add_spikes(unsigned int nspikes);
    
    /* returns decoding bin [ms] */

    void set_log_likelihood(std::valarray<double>& log_likelihood); // copy
    void set_log_likelihood(double log_likelihood_value, size_t grid_index);
    void set_log_likelihood(double* log_likelihood_values, size_t grid_size);
    
    const std::valarray<double>& log_likelihood() const;

    double likelihood(size_t grid_index);
    const std::valarray<double>& likelihood();
    
    double integral_likelihood( bool nan_aware = false );
    
    void decrement_loglikelihood(double amount, size_t grid_index);
    void increment_loglikelihood(double amount, size_t grid_index);
    
    void multiply_likelihood_inplace( Data* other, bool log_space = true );
    
    void accumulate_likelihood( Data* other, bool log_space = true );
    
    size_t argmax() const;
    
    double mua() const;
    
    void YAMLDescription( YAML::Node & node,
        Serialization::Format format = Serialization::Format::FULL ) const override;

    void SerializeBinary( std::ostream& stream,
        Serialization::Format format = Serialization::Format::FULL) const override;
    
    void SerializeYAML( YAML::Node & node,
        Serialization::Format format = Serialization::Format::FULL ) const override;
    
protected:
    void YAMLDescriptionCompact( YAML::Node & node ) const;
    void SerializeBinaryCompact( std::ostream& stream ) const;
    
public:
    static constexpr double DEFAULT_LOG_LIKELIHOOD_VALUE = 0;
    static constexpr double DEFAULT_LIKELIHOOD_VALUE = std::exp(DEFAULT_LOG_LIKELIHOOD_VALUE);
    
protected:
    std::valarray<double> compute_likelihood( std::valarray<double> log_likelihood ) const;
    std::valarray<double> normalized_log_likelihood();
    
protected:
    std::valarray<double> log_likelihood_;
    bool likelihood_is_updated_;
    std::valarray<double> cached_likelihood_;
    double cached_integral_likelihood_;
    size_t grid_size_; // # points of the 1-D decoding grid
    double time_bin_; // duration (in ms) of the buffer of data used to compute the likelihood
    unsigned int n_spikes_; // number of spikes that were used to compute the likelihood

protected:
    const std::string N_SPIKES_S = "n spikes";
    const std::string TIME_BIN_S = "time bin";
    const std::string LOG_LIKELIHOOD_S = "log likelihood";
    const std::string LIKELIHOOD_S = "likelihood";
    const double INTEGRAL_OBSOLETE = -1;
};

}


class LikelihoodType {
public:
    static const std::string datatype() { return "likelihood"; }
    static const std::string dataname() { return "data"; }

    using Base = nsLikelihoodType::Base;
    using Parameters = nsLikelihoodType::Parameters;
    using Capabilities = nsLikelihoodType::Capabilities;
    using Data = nsLikelihoodType::Data;
};


#endif	/// likelihooddata.hpp
