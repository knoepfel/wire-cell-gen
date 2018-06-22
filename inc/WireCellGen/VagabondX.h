#ifndef WIRECELL_VAGABONDX
#define WIRECELL_VAGABONDX

#include "WireCellIface/IDrifter.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/IRandom.h"
#include "WireCellUtil/Units.h"

#include <deque>

namespace WireCell {

    namespace Gen {

        /** This drifter assumes nominal drift direction is in the +/-
         * X direction.  It assumes infinite anode/cathode planes
         * perpendicular to the X-axis and specified by their
         * intercepts.  Any depos which do not lie between an anode
         * and cathode plane are dropped.  
         *
         * Diffusion and absorption effects and optional fluctuations
         * are applied.  Input depositions must be ordered in absolute
         * time (not drift time) and output depositions are produced
         * in ordered by their time at the anode plane.
         * 
         * Note that the input electrons are assumed to be already
         * free from Fano and Recombination.
         */
        class VagabondX : public IDrifter, public IConfigurable {
        public:
            VagabondX();
            virtual ~VagabondX();

            virtual void reset();
            virtual bool operator()(const input_pointer& depo, output_queue& outq);

            /// WireCell::IConfigurable interface.
            virtual void configure(const WireCell::Configuration& config);
            virtual WireCell::Configuration default_configuration() const;


            // Implementation methods.

            // Do actual transport, producing a new depo
            IDepo::pointer transport(IDepo::pointer depo);

            // Return the "proper time" for a deposition
            double proper_time(IDepo::pointer depo);

            bool insert(const input_pointer& depo);
            void flush(output_queue& outq);
            void flush_ripe(output_queue& outq, double now);

        private:

            IRandom::pointer m_rng;
            std::string m_rng_tn;

            // Longitudinal and Transverse coefficients of diffusion
            // in units of [length^2]/[time].
            double m_DL, m_DT;

            // Electron absorption lifetime.
            double m_lifetime;          

            // If true, fluctuate by number of absorbed electrons.
            bool m_fluctuate;

            double m_speed;   // drift speeds
            int n_dropped, n_drifted;

            // A little helper to carry the region extent and depo buffers.
            struct Xregion {
                Xregion(double ax, double cx);
                double anode, cathode;
                std::vector<IDepo::pointer> depos; // buffer depos

                bool inside(double x) const;

            };
            std::vector<Xregion> m_xregions;  

            struct IsInside {
                const input_pointer& depo;
                IsInside(const input_pointer& depo) : depo(depo) {}
                bool operator()(const Xregion& xr) const { return xr.inside(depo->pos().x()); }
            };

        };

    }

}

#endif