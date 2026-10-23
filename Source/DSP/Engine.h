// Disco Sauce 2 Pro - complete signal chain (pure C++). The JUCE processor only feeds it parameters and audio.
//   in gain -> [mid: pitch correction | side: aligned delay] -> EQ -> de-esser -> compressor -> saturation
//   -> doubler -> delay -> reverb -> dry/wet (latency-aligned dry) -> out gain -> limiter
#pragma once
#include "PitchCorrector.h"
#include "Effects.h"
#include "../Params.h"
#include <cstring>

namespace ds {

class Engine
{
public:
    void prepare(double sr, int maxBlock)
    {
        const size_t mb = (size_t) std::max(maxBlock, 64);
        wetL.reserve(mb); wetR.reserve(mb); dryL.reserve(mb); dryR.reserve(mb);
        fs = sr; pc.prepare(sr); eq.prepare(sr); comp.prepare(sr); dees.prepare(sr); dbl.prepare(sr); dly.prepare(sr); rev.prepare(sr); lim.prepare(sr);
        satur.reset();
        tuneLat = pc.latency();
        lat = tuneLat + Saturator::latency() + lim.latency();              // everything the host must compensate
        size_t n = 1; while ((int) n < lat + 8) n <<= 1;
        for (int i = 0; i < dsp2::kNumParams; ++i) smoothable[i] = dsp2::paramDef(i).kind == dsp2::KFloat && std::strcmp(dsp2::paramDef(i).group, "AUTO-TUNE") != 0 && std::strcmp(dsp2::paramDef(i).group, "NOTE MAP") != 0;
        smInit = false;
        dm.assign(n, 0.0f); dsd.assign(n, 0.0f); dmask = (int64_t) n - 1; dw = 0;
        prevIn = prevOut = 1.0f; wasTune = false;
    }
    void reset() { pc.reset(); eq.prepare(fs); comp.prepare(fs); dees.prepare(fs); satur.reset(); dbl.prepare(fs); dly.prepare(fs); rev.prepare(fs); lim.prepare(fs); smInit = false; std::fill(dm.begin(), dm.end(), 0.0f); std::fill(dsd.begin(), dsd.end(), 0.0f); wasTune = false; }
    int latency() const { return lat; }

    // p = array of kNumParams plain (real-unit) values; bpm used for tempo-synced delay
    void process(float* l, float* r, int n, const float* pRaw, double bpm)
    {
        using namespace dsp2;
        // parameter smoothing (about 25 ms) so automation / knob moves never click or zipper
        if (!smInit) { for (int i = 0; i < kNumParams; ++i) sm[i] = pRaw[i]; smInit = true; }
        const float coef = 1.0f - std::exp(-(float) n / (0.025f * (float) fs));
        for (int i = 0; i < kNumParams; ++i) sm[i] = smoothable[i] ? sm[i] + (pRaw[i] - sm[i]) * coef : pRaw[i];
        const float* p = sm;
        auto V = [&](int i) { return p[i]; };
        const float inG = dbToLin(V(P_InGain)), outG = dbToLin(V(P_OutGain)), mixW = V(P_Mix) * 0.01f;
        const bool tuneOn = V(P_TuneOn) > 0.5f;

        // --- settings
        TuneSettings S;
        S.key = (int) V(P_Key); S.scale = (int) V(P_Scale);
        for (int i = 0; i < 12; ++i) { S.noteOn[i] = V(P_NoteOn0 + i) > 0.5f; S.noteCents[i] = V(P_NoteCt0 + i); }
        S.amount = V(P_TuneAmt) * 0.01f; S.retuneMs = V(P_Retune); S.humanize = V(P_Humanize) * 0.01f; S.hystCents = V(P_Hyst);
        S.transpose = V(P_Transpose); S.fine = V(P_Fine); S.refA = V(P_RefA); S.rangeLow = V(P_RangeLow); S.rangeHigh = std::max(V(P_RangeHigh), V(P_RangeLow) + 50.0f);
        S.sensitivity = V(P_Sens) * 0.01f; S.gateDb = V(P_GateDb);
        pc.setSettings(S);
        if (tuneOn && !wasTune) pc.reset();
        wasTune = tuneOn;

        EqParams e; e.on = V(P_EqOn) > 0.5f; e.hpFreq = V(P_HpFreq); e.lsFreq = V(P_LsFreq); e.lsGain = V(P_LsGain);
        e.f[0] = V(P_B1F); e.g[0] = V(P_B1G); e.q[0] = V(P_B1Q); e.f[1] = V(P_B2F); e.g[1] = V(P_B2G); e.q[1] = V(P_B2Q);
        e.f[2] = V(P_B3F); e.g[2] = V(P_B3G); e.q[2] = V(P_B3Q); e.hsFreq = V(P_HsFreq); e.hsGain = V(P_HsGain); e.lpFreq = V(P_LpFreq);
        eq.setParams(e);
        DeEssParams d; d.on = V(P_DsOn) > 0.5f; d.freq = V(P_DsFreq); d.thr = V(P_DsThr); d.amount = V(P_DsAmt) * 0.01f; dees.setParams(d);
        CompParams c; c.on = V(P_CpOn) > 0.5f; c.thr = V(P_CpThr); c.ratio = V(P_CpRatio); c.atkMs = V(P_CpAtk); c.relMs = V(P_CpRel); c.knee = V(P_CpKnee); c.makeup = V(P_CpMake); c.mix = V(P_CpMix) * 0.01f; comp.setParams(c);
        SatParams sat; sat.on = V(P_SatOn) > 0.5f; sat.drive = V(P_SatDrive) * 0.01f; sat.mix = V(P_SatMix) * 0.01f;
        DblParams b; b.amount = V(P_Dbl) * 0.01f; b.voices = (int) std::lround(V(P_DblVoices)); b.delayMs = V(P_DblDelay); b.detune = V(P_DblDetune); b.width = V(P_DblWidth) * 0.01f; b.tone = V(P_DblTone); b.octave = V(P_DblOct) * 0.01f; dbl.setParams(b);
        DelayParams y; y.on = V(P_DlOn) > 0.5f; y.mix = V(P_DlMix) * 0.01f; y.fb = V(P_DlFb) * 0.01f; y.tone = V(P_DlTone); y.hp = V(P_DlHp); y.ping = V(P_DlPing) > 0.5f;
        y.timeMs = V(P_DlSync) > 0.5f ? (float)(60000.0 / std::max(20.0, std::min(400.0, bpm)) * divisionQuarterNotes((int) V(P_DlDiv))) : V(P_DlTime);
        y.timeMs = std::min(y.timeMs, 2000.0f);
        dly.setParams(y);
        ReverbParams rv; rv.on = V(P_RvOn) > 0.5f; rv.mix = V(P_RvMix) * 0.01f; rv.size = V(P_RvSize) * 0.01f; rv.damp = V(P_RvDamp) * 0.01f; rv.preMs = V(P_RvPre); rv.width = V(P_RvWidth) * 0.01f; rv.low = V(P_RvLow); rv.high = V(P_RvHigh); rev.setParams(rv);
        lim.on = V(P_LimOn) > 0.5f; lim.ceilDb = V(P_LimCeil);

        // --- input gain (ramped) , mid/side split, latency-aligned paths
        const float gs = (inG - prevIn) / (float) std::max(n, 1);
        float g = prevIn;
        wetL.resize((size_t) n); wetR.resize((size_t) n); dryL.resize((size_t) n); dryR.resize((size_t) n);
        for (int i = 0; i < n; ++i) {
            g += gs;
            const float a = l[i] * g, bb = r[i] * g;
            const float mid = 0.5f * (a + bb), side = 0.5f * (a - bb);
            dm[(size_t)(dw & dmask)] = mid; dsd[(size_t)(dw & dmask)] = side;
            const size_t ri = (size_t)((dw - tuneLat) & dmask);                                  // wet path source
            const size_t rd = (size_t)((dw - tuneLat - Saturator::latency()) & dmask);          // dry path: matches wet latency
            const float dMid = dm[ri], dSide = dsd[ri], dryMid = dm[rd], drySide = dsd[rd];
            ++dw;
            const float tm = tuneOn ? pc.process(mid) : dMid;
            wetL[(size_t) i] = tm + dSide; wetR[(size_t) i] = tm - dSide;
            dryL[(size_t) i] = dryMid + drySide; dryR[(size_t) i] = dryMid - drySide;
        }
        prevIn = inG;
        float* wl = wetL.data(); float* wr = wetR.data();
        eq.process(wl, wr, n); dees.process(wl, wr, n); comp.process(wl, wr, n); satur.process(sat, wl, wr, n);
        dbl.process(wl, wr, n); dly.process(wl, wr, n); rev.process(wl, wr, n);

        const float os = (outG - prevOut) / (float) std::max(n, 1);
        float og = prevOut;
        for (int i = 0; i < n; ++i) {
            og += os;
            l[i] = (dryL[(size_t) i] * (1.0f - mixW) + wl[i] * mixW) * og;
            r[i] = (dryR[(size_t) i] * (1.0f - mixW) + wr[i] * mixW) * og;
        }
        prevOut = outG;
        lim.process(l, r, n);
    }

    float compGrDb() const { return comp.grDb(); }
    float inputCents() const { return pc.lastInputCents(); }
    float targetCents() const { return pc.lastTargetCents(); }
    bool  voiced() const { return pc.lastVoiced(); }

private:
    double fs = 48000; int lat = 0, tuneLat = 0; float sm[dsp2::kNumParams] = {}; bool smoothable[dsp2::kNumParams] = {}, smInit = false;
    PitchCorrector pc; Eq eq; Compressor comp; DeEsser dees; Saturator satur; Doubler dbl; Delay dly; Reverb rev; Limiter lim;
    std::vector<float> dm, dsd, wetL, wetR, dryL, dryR; int64_t dmask = 0, dw = 0; float prevIn = 1, prevOut = 1; bool wasTune = false;
};

} // namespace ds
