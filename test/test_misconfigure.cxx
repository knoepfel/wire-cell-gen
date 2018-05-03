#include "WireCellIface/IFrameFilter.h"
#include "WireCellIface/IConfigurable.h"
#include "WireCellIface/SimpleFrame.h"
#include "WireCellIface/SimpleTrace.h"
#include "WireCellUtil/PluginManager.h"
#include "WireCellUtil/Binning.h"
#include "WireCellUtil/NamedFactory.h"

#include <iostream>

#include "TCanvas.h"
#include "TFile.h"
#include "TStyle.h"
#include "TH2F.h"


using namespace std;
using namespace WireCell;
using namespace WireCell::Waveform;

// Make up a Gaussian charge distribution 
std::vector<float> blip(Binning bins,
                        double time=1*units::ms, double sigma = 3*units::us, double mag = 10000.0)
{
    int nticks = bins.nbins();
    std::vector<float> charge(nticks,0);
    for (int ind=0; ind<nticks; ++ind) {
        const double t = bins.center(ind);
        const double rel = (t-time)/sigma;
        const double val = mag * exp(-0.5*rel*rel);
        charge[ind] = val;
    }
    return charge;
}

float maximum(const std::vector<float>& wave) {
    float m=0;
    for (auto q : wave ) {
        if (q > m) m = q;
    }
    return m;
}


TH1F* plot_wave(TCanvas& canvas, const std::vector<float>& wave, std::string name, std::string title)
{
    TH1F* hist = new TH1F(name.c_str(), title.c_str(), wave.size(), 0, wave.size()+1);
    int tick = 0;
    for (auto q : wave) {
        hist->Fill(tick+0.5, q);
        ++tick;
    }
    hist->SetXTitle("tick");
    hist->SetYTitle("amplitude");
    hist->Draw("hist");
    canvas.Print("test-misconfigure.pdf");
    return hist;
}

TH2F* plot_ratio(TCanvas& canvas, TH2F* f1, TH2F* f2)
{
    TH2F* out = (TH2F*)f1->Clone("ratio");
    out->SetTitle("Ratio");
    out->Divide(f2);
    out->SetXTitle("ticks");
    out->SetYTitle("channels");
    out->Draw("colz");
    canvas.Print("test-misconfigure.pdf");
    return out;
}

TH2F* plot_frame(TCanvas& canvas, IFrame::pointer frame, std::string name, double mtick=-1, double mchan=-1);
TH2F* plot_frame(TCanvas& canvas, IFrame::pointer frame, std::string name, double mtick, double mchan)
{
    auto traces = frame->traces();

    int maxchan=0, maxtick=0;
    for (auto trace : (*traces)) {
        auto& charge = trace->charge();
        int tick = trace->tbin() + charge.size();
        if (tick > maxtick) {
            maxtick = tick;
        }
        int chan = trace->channel();
        if (chan > maxchan) {
            maxchan = chan;
        }        
        cerr << name << " " << chan << " " << tick << " " << maximum(charge) << "\n";
    }

    TH2F* hist = new TH2F(name.c_str(), name.c_str(),
                          maxtick+1, 0, maxtick+1,
                          maxchan+1, 0, maxchan+1);
    
    for (auto trace : (*traces)) {
        int tick = trace->tbin();
        int chan = trace->channel();
        auto& charge = trace->charge();
        for (float q : charge) {
            hist->Fill(tick+0.5, chan+0.5, q);
            ++tick;
        }
    }
    hist->SetXTitle("ticks");
    hist->SetYTitle("channels");
    hist->Draw("colz");
    if (mtick > 0) {
        hist->GetXaxis()->SetRangeUser(0, mtick);
    }
    if (mchan > 0) {
        hist->GetYaxis()->SetRangeUser(0, mchan);
    }

    canvas.Print("test-misconfigure.pdf","pdf");
    return hist;
}


int main(int argc, char* argv[])
{
    PluginManager& pm = PluginManager::instance();
    pm.add("WireCellGen");
    pm.add("WireCellSio");

    double gain, shaping, tick;
    {
        IConfigurable::pointer mccfg = Factory::lookup<IConfigurable>("Misconfigure");
        Configuration cfg = mccfg->default_configuration();
        cfg["nsamples"] = 100;
        cfg["truncate"] = true;
        // use defaults locally
        gain = cfg["from"]["gain"].asDouble();
        shaping = cfg["from"]["shaping"].asDouble();
        tick = cfg["tick"].asInt();
        mccfg->configure(cfg);
    }
    IFrameFilter::pointer mc = Factory::lookup<IFrameFilter>("Misconfigure");

    Response::ColdElec ce(gain, shaping);

    auto resp = ce.generate(Binning(200, 0, 200*tick));
    auto resp2 = ce.generate(Binning(400, 0, 400*tick));
    auto resp_spec = Waveform::dft(resp);
    auto resp_spec2 = Waveform::dft(resp2);

    ITrace::vector q_traces;
    ITrace::vector out_traces;

    int qchannel = 0;
    int channel = 0;
    for (int nbins : {50, 100, 200, 400}) {
        Binning bins(nbins, 0, nbins*tick);
            
        auto q1 = blip(bins, 0.25 * nbins * tick);
        auto q2 = blip(bins, 0.50 * nbins * tick, 0.5*units::us);
        auto q3 = blip(bins, 0.75 * nbins * tick);
        auto q4 = Waveform::realseq_t(nbins, 0);
        q4[nbins/2] = maximum(q1);
    
        q_traces.push_back(std::make_shared<SimpleTrace>(qchannel++, 0, q1));
        q_traces.push_back(std::make_shared<SimpleTrace>(qchannel++, 0, q2));
        q_traces.push_back(std::make_shared<SimpleTrace>(qchannel++, 0, q3));
        q_traces.push_back(std::make_shared<SimpleTrace>(qchannel++, 0, q4));

        auto e1 = linear_convolve(q1, resp);
        auto e2 = linear_convolve(q2, resp);
        auto e3 = linear_convolve(q3, resp);
        auto e4 = linear_convolve(q4, resp);

        out_traces.push_back(std::make_shared<SimpleTrace>(channel++, 0, e1));
        out_traces.push_back(std::make_shared<SimpleTrace>(channel++, 0, e2));
        out_traces.push_back(std::make_shared<SimpleTrace>(channel++, 0, e3));
        out_traces.push_back(std::make_shared<SimpleTrace>(channel++, 0, e4));
    }

    auto qorig = std::make_shared<SimpleFrame>(0, 0.0, q_traces, tick);
    auto orig = std::make_shared<SimpleFrame>(0, 0.0, out_traces, tick);
    IFrame::pointer mced, dummy;
    {
        bool ok = (*mc)(orig, mced);
        if (!ok) { return -1; }
    }


    gStyle->SetOptStat(0);
    gStyle->SetOptFit(0);
    TCanvas canvas("canvas","canvas", 1200, 800);

    canvas.Print("test-misconfigure.pdf[","pdf");

    plot_wave(canvas,resp,"resp","Electronics response waveform");
    plot_wave(canvas,resp2,"resp1","Electronics response waveform");
    plot_wave(canvas, Waveform::real(resp_spec),"respect","Electronics response spectrum");
    plot_wave(canvas, Waveform::real(resp_spec2),"respect2","Electronics response spectrum");

    plot_frame(canvas, qorig, "ChargeZoomed", 50., 5.);
    plot_frame(canvas, orig, "ShapedZoomed", 50., 5.);
    plot_frame(canvas, mced, "MisconfiguredZoomed", 50, 5);

    plot_frame(canvas, qorig, "Charge");
    auto f1 = plot_frame(canvas, orig, "Shaped");
    auto f2 = plot_frame(canvas, mced, "Misconfigured");
    plot_ratio(canvas, f1, f2);
    canvas.Print("test-misconfigure.pdf]","pdf");

    return 0;
}
