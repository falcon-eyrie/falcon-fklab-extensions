/* 
 * File:   openephys.hpp
 * Author: davide
 *
 * Created on 13 March 2017, 15:36
 */

#ifndef OPENEPHYS_HPP
#define	OPENEPHYS_HPP

namespace OpenEphys {

const double SIGNAL_SAMPLING_FREQUENCY = 30000;
const double DSP_CUTOFF = 0.1;
const double LOWER_BANDWIDTH = 1;
const double UPPER_BANDWIDTH = 7500;
const bool DSP_ENABLED = true;
const double AD_BIT_MICROVOLTS = 0.195;
const int NCHANNELS_PER_PORT = 32;
const int DEFAULT_DATASTREAM = 0;
const double FOOT_CABLELENGTH = 6;

inline double ADbits_to_microvolts( int adbits ) {
    
    // how to convert to microvolts
    // https://open-ephys.atlassian.net/wiki/display/OEW/Flat+binary+format
    return AD_BIT_MICROVOLTS * adbits;
}

}

#endif	/* openephys.hpp */

