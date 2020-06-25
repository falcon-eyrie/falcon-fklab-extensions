#include <iostream>

#include "likelihooddata.hpp"


using namespace nsLikelihoodType;

void Data::Initialize ( size_t grid_size) {
    
    grid_size_ = grid_size;
    this->ClearData();
}


unsigned int Data::n_spikes() const {
    
    return n_spikes_;
}

void Data::add_spikes(unsigned int nspikes) {
    
    n_spikes_ += nspikes;
}


void Data::set_log_likelihood(std::valarray<double>& log_likelihood) {
    
    log_likelihood_ = log_likelihood;
    likelihood_is_updated_ = false;
}

void Data::set_log_likelihood(double log_likelihood_value, size_t grid_index) {
    
    log_likelihood_[grid_index] = log_likelihood_value;
    likelihood_is_updated_ = false;
}

void Data::set_log_likelihood(double* log_likelihood_values, size_t grid_size) {
    
    for (decltype(grid_size) g = 0; g < grid_size; ++g) {
        // additional checks are fine because this method is not going to be used
        // in graphs dedicated to real-time processing 
        if ((log_likelihood_values + g) != nullptr) {
            log_likelihood_[g] = log_likelihood_values[g];
        } else {
            throw std::runtime_error("Invalid address at grid " + std::to_string(g));
        }
    }
    likelihood_is_updated_ = false;
    cached_integral_likelihood_ = INTEGRAL_OBSOLETE;
}

const std::valarray<double>& Data::log_likelihood() const {
    
    return log_likelihood_;
}

double Data::likelihood(std::size_t grid_index) {
    
    return likelihood()[grid_index];
}

void Data::decrement_loglikelihood(double amount, size_t grid_index) {
    
    log_likelihood_[grid_index] -= amount;
    likelihood_is_updated_ = false;
    cached_integral_likelihood_ = INTEGRAL_OBSOLETE;
}

void Data::increment_loglikelihood(double amount, size_t grid_index) {
    
    log_likelihood_[grid_index] += amount;
    likelihood_is_updated_ = false;
    cached_integral_likelihood_ = INTEGRAL_OBSOLETE;
}

const std::valarray<double>& Data::likelihood() {
    
    if (!likelihood_is_updated_) {
        cached_likelihood_ = compute_likelihood( log_likelihood_ );
        likelihood_is_updated_ = true;
    }
    return cached_likelihood_;
}

std::valarray<double> Data::compute_likelihood(
    std::valarray<double> log_likelihood ) const {
    
    std::valarray<double> normalized_log_likelihood =
        log_likelihood - log_likelihood.max();

    return std::exp( normalized_log_likelihood );
}

double Data::integral_likelihood( bool nan_aware ) {
    
    if ( cached_integral_likelihood_ == INTEGRAL_OBSOLETE ) {
        if ( nan_aware ) {
            // small bug here: no track of whether the cached likelihood was
            // computed with or without nan awareness
            cached_integral_likelihood_ = nan_sum( std::begin(likelihood()),
                std::end(likelihood()) );
        } else {
            cached_integral_likelihood_ = likelihood().sum();
        }
    }
    return cached_integral_likelihood_;
}

void Data::multiply_likelihood_inplace ( Data* other,
    bool log_space ) {
    
    if (log_space) {
        log_likelihood_ += other->log_likelihood();
        likelihood_is_updated_ = false;
        cached_integral_likelihood_ = INTEGRAL_OBSOLETE;
    } else {
        cached_likelihood_ = this->likelihood() * other->likelihood();
        cached_integral_likelihood_ = INTEGRAL_OBSOLETE;
        // because we want always an up-to-date log_likelihood:
        log_likelihood_ = std::exp(cached_likelihood_);
    }
    this->add_spikes( other->n_spikes() );
}

void Data::accumulate_likelihood( Data* other,  bool log_space ) {
    
    this->multiply_likelihood_inplace(other, log_space);
    this->set_time_bin( this->time_bin() + other->time_bin() );
    this->add_spikes( other->n_spikes() );
}

std::size_t Data::argmax() const {
    
    std::size_t argmax = 0, i;
    double value = log_likelihood_[argmax];
    double max_current_value = value;
    
    assert (grid_size() == log_likelihood_.size());
    for (i=1; i < grid_size(); ++ i) {
        value = log_likelihood_[i];
        if ( value > max_current_value ) {
            max_current_value = value;
            argmax = i;
        }
    }
    return argmax;
}

double Data::mua() const {
    
    return n_spikes_ / (time_bin() * 1e-3);
}

/*namespace nsLikelihoodType {
    double& operator[](const Data &log_likelihood, const size_t idx) {
        return log_likelihood_[idx];
    }
}*/

void Data::YAMLDescription( YAML::Node & node, Serialization::Format format ) const {

    Base::Data::YAMLDescription( node, format );
    
    if ( format==Serialization::Format::FULL ) {
        YAMLDescriptionCompact( node );
        node.push_back( LIKELIHOOD_S + " " + get_type_string<double>() +
        " (" + std::to_string(grid_size()) + ")" );
    }
    
    if ( format == Serialization::Format::COMPACT ) {
        YAMLDescriptionCompact( node );
    }
}

void Data::YAMLDescriptionCompact( YAML::Node & node ) const {
    
    node.push_back( N_SPIKES_S + " " +
        get_type_string<decltype(n_spikes_)>() + " (1)" );
    node.push_back( TIME_BIN_S + " " +
        get_type_string<decltype(time_bin())>() + " (1)" );
    node.push_back( LOG_LIKELIHOOD_S + " " + get_type_string<double>()  +
        " (" + std::to_string(grid_size()) + ")" );
}

void Data::SerializeBinary( std::ostream& stream, Serialization::Format format ) const {

    Base::Data::SerializeBinary( stream, format );
    
    if ( format==Serialization::Format::COMPACT ) {
        SerializeBinaryCompact( stream );
    }
    
    if ( format==Serialization::Format::FULL ) {
        // likelihood() method cannot be used because it will break the const-ness
        std::valarray<double> likelihood; 
        SerializeBinaryCompact( stream );
        if (!likelihood_is_updated_) {
            likelihood = compute_likelihood( log_likelihood_ );
        } else {
            likelihood = cached_likelihood_;
        }
        stream.write( reinterpret_cast<const char*>( &likelihood[0] ),
            sizeof( decltype( &likelihood[0] ) ) * grid_size() );
    }
}

void Data::SerializeYAML( YAML::Node & node, Serialization::Format format ) const {

    Base::Data::SerializeYAML( node, format );
    
    if ( format==Serialization::Format::FULL || format==Serialization::Format::COMPACT ) {
        // valarrays are not accepted by YAML and must be converted into vectors
        std::vector<double> log_likelihood( log_likelihood_.size() );
        for ( size_t i=0; i < log_likelihood_.size(); ++i) {
            log_likelihood[i] = log_likelihood_[i];
        }
        node[N_SPIKES_S] =  n_spikes_;
        node[TIME_BIN_S] = time_bin();
        node[LOG_LIKELIHOOD_S] = log_likelihood;
    }
    
    if ( format==Serialization::Format::FULL ) {
        std::valarray<double> likelihood;
        if (!likelihood_is_updated_) {
            likelihood = compute_likelihood( log_likelihood_ );
        } else {
            likelihood = cached_likelihood_;
        }
        // valarrays are not accepted by YAML and must be converted into vectors
        std::vector<double> likelihood_v( likelihood.size() );
        for ( size_t i=0; i < likelihood.size(); ++i) {
            likelihood_v[i] = likelihood[i];
        }
        node[LIKELIHOOD_S] = likelihood_v;   
    }
}

void Data::SerializeBinaryCompact( std::ostream& stream ) const {
    
    stream.write( reinterpret_cast<const char*>( &n_spikes_ ) ,
        sizeof( decltype(n_spikes_)) );
    stream.write( reinterpret_cast<const char*>( &time_bin_ ) ,
        sizeof( decltype(time_bin())) );
    stream.write( reinterpret_cast<const char*>( &log_likelihood_[0] ),
        sizeof( decltype(log_likelihood_[0]) ) * grid_size() );
}


