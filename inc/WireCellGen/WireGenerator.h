#ifndef WIRECELLGEN_WIREGENERATOR
#define WIRECELLGEN_WIREGENERATOR

#include "WireCellIface/IWireGenerator.h"
#include "WireCellIface/IWire.h"

#include <deque>

namespace WireCell {

    /** A source of wire (segment) geometry as generated from parameters.
     *
     * All wires in one plane are constructed to be parallel to
     * one-another and to be equally spaced between neighbors and
     * perpendicular to the drift direction.
     */

    class WireGenerator : public IWireGenerator {
    public:
	WireGenerator();
	virtual ~WireGenerator();

	virtual bool insert(const input_pointer& in);
	virtual bool extract(output_pointer& out) {
	    out = m_output;
	    return true;
	}

    private:

	output_pointer m_output;
    };


}

#endif
