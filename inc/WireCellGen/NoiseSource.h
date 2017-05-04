/** This frame source provides frames filled with noise.
    
    Each time it is called it produces a fixed readout length of
    voltage-level noise which spans all channels.

 */

#ifndef WIRECELLGEN_NOISESOURCE
#define WIRECELLGEN_NOISESOURCE

#include "WireCellIface/IFrameSource.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IAnodePlane.h"
#include "WireCellUtil/Waveform.h"

#include <string>

namespace WireCell {
    namespace Gen {

        class NoiseSource : public IFrameSource, public IConfigurable {
        public:
            NoiseSource();
            // fixme: add constructor that set parameter defaults from c++ for unit tests
            virtual ~NoiseSource();

            /// IFrameSource 
            virtual bool operator()(IFrame::pointer& frame);

            /// IConfigurable
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;


            virtual Waveform::realseq_t waveform(int channel_ident);

        private:
            IAnodePlane::pointer m_anode;
            double m_time, m_readout, m_tick;
            int m_frame_count;
            std::string m_anode_tn;
            
        };
    }
}

#endif
