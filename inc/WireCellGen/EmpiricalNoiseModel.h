/** 
    A noise model based on empirically measured noise spectra. 

    It requires configuration file holding a list of dictionary which
    provide association between wire length and the noise spectrum.

    TBD: document JSON file format for providing spectra and any other parameters.

*/
    

#ifndef WIRECELLGEN_EMPERICALNOISEMODEL
#define WIRECELLGEN_EMPERICALNOISEMODEL

#include "WireCellIface/IChannelSpectrum.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IAnodePlane.h"

#include "WireCellUtil/Units.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace WireCell {
    namespace Gen {

        class EmpiricalNoiseModel : public IChannelSpectrum , public IConfigurable {
        public:
            EmpiricalNoiseModel(const std::string& spectra_file = "",
                                const int nsamples = 10000/2, // assuming 10k samples 
                                const double period = 0.5*units::us,
                                const double wire_length_scale = 1.0*units::cm,
				const double time_scale = 1.0*units::ns,
				const double gain_scale = 1.0*units::volt/units::eplus,
                                const std::string anode_tn = "AnodePlane");

            virtual ~EmpiricalNoiseModel();

            /// IChannelSpectrum
            virtual const amplitude_t& operator()(int chid) const;

	    // get constant term
	    // virtual const double constant(int chid) const;
	    // get gain 
	    virtual const double gain(int chid) const;
	    // get shaping time
	    virtual const double shaping_time(int chid) const;
	    

            /// IConfigurable
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;


            // Local methods

            // fixme: this should be factored out into wire-cell-util.
            struct NoiseSpectrum {
                int plane;      // plane identifier number
                int nsamples;   // number of samples used in preparing spectrum
                double period;  // sample period [time] used in preparing spectrum
                double gain;    // amplifier gain [voltage/charge]
                double shaping; // amplifier shaping time [time]
                double wirelen; // total length of wire conductor [length]
                double constant; // amplifier independent constant noise component [voltage/frequency]
                std::vector<float> freqs; // the frequencies at which the spectrum is sampled
                std::vector<float> amps;  // the amplitude [voltage/frequency] of the spectrum.
            };

            // Resample a NoiseSpectrum to match what the model was
            // configured to provide.  This method modifies in place.
            void resample(NoiseSpectrum& spectrum) const;

            // Return a new amplitude which is the interpolation
            // between those given in the spectra file.
            amplitude_t interpolate(int plane, double wire_length) const;


        private:
            IAnodePlane::pointer m_anode;

            std::string m_spectra_file;
            int m_nsamples;
	    double m_period, m_wlres;
            double m_tres, m_gres;
	    std::string m_anode_tn;
            

            std::map<int, std::vector<NoiseSpectrum*> > m_spectral_data;

            // cache amplitudes to the nearest integral distance.
            mutable std::unordered_map<int, int> m_chid_to_intlen;

            typedef std::unordered_map<int, amplitude_t> len_amp_cache_t;
            mutable std::vector<len_amp_cache_t> m_amp_cache;

        };

    }
}


#endif
