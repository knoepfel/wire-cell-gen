#include "WireCellGen/DumpFrames.h"
#include "WireCellUtil/Units.h"
#include "WireCellUtil/NamedFactory.h"

#include <iostream>

WIRECELL_FACTORY(DumpFrames, WireCell::Gen::DumpFrames, WireCell::IFrameSink)

using namespace std;
using namespace WireCell;

Gen::DumpFrames::DumpFrames()
{
}


Gen::DumpFrames::~DumpFrames()
{
}


bool Gen::DumpFrames::operator()(const IFrame::pointer& frame)
{
    if (!frame) {
        cerr << "Gen::DumpFrames sees EOS\n";
        return true;
    }
    auto traces = frame->traces();
    const int ntraces = traces->size();
    
    cerr << "Gen::Frame: #" << frame->ident()
         << " @" << frame->time()/units::ms
         << " with " << ntraces << " traces" << endl;
    {
        string comma = "";
        cerr << "\tframe tags:[";
        for (auto ftag : frame->frame_tags()) {
            cerr << comma << ftag;
            comma = ", ";
        }
        cerr << "]\n";
    }
    {
        string comma = "";
        cerr << "\ttrace tags:[";
        for (auto ftag : frame->trace_tags()) {
            cerr << comma << ftag;
            comma = ", ";
        }
        cerr << "]\n";
    }
    return true;
}

