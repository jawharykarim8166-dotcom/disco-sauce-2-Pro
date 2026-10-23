// Disco Sauce 2 Pro - effect blocks (pure C++, no JUCE, unit-tested for stability and behaviour)
#pragma once
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include "Scales.h"

namespace ds {

inline float dbToLin(float db) { return std::pow(10.0f, db * 0.05f); }
inline float linToDb(float l)  { return 20.0f * std::log10(std::max(l, 1e-9f)); }
inline float flushDenormal(float v) { return (std::fabs(v) < 1e-20f) ? 0.0f : v; }

// ---------------------------------------------------------------- biquad (RBJ)
struct Biquad
{
    enum Type { LowPass, HighPass, Bell, LowShelf, HighShelf };
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;

    void reset() { z1 = z2 = 0; }
    void set(Type t, double fs, double f, double q, double gainDb)
    {
        f = std::min(std::max(f, 10.0), 0.45 * fs);
        q = std::max(q, 0.1);
        const double w0 = 2.0 * kPi * f / fs, c = std::cos(w0), s = std::sin(w0);
        const double al = s / (2.0 * q), A = std::pow(10.0, gainDb / 40.0);
        double n0, n1, n2, d0, d1, d2;
        switch (t) {
            case LowPass:  n0 = (1 - c) / 2; n1 = 1 - c; n2 = (1 - c) / 2; d0 = 1 + al; d1 = -2 * c; d2 = 1 - al; break;
            case HighPass: n0 = (1 + c) / 2; n1 = -(1 + c); n2 = (1 + c) / 2; d0 = 1 + al; d1 = -2 * c; d2 = 1 - al; break;
            case Bell:     n0 = 1 + al * A; n1 = -2 * c; n2 = 1 - al * A; d0 = 1 + al / A; d1 = -2 * c; d2 = 1 - al / A; break;
            case LowShelf: {
                const double sa = 2.0 * std::sqrt(A) * (s / 2.0 * std::sqrt(2.0));
                n0 = A * ((A + 1) - (A - 1) * c + sa); n1 = 2 * A * ((A - 1) - (A + 1) * c); n2 = A * ((A + 1) - (A - 1) * c - sa);
                d0 = (A + 1) + (A - 1) * c + sa; d1 = -2 * ((A - 1) + (A + 1) * c); d2 = (A + 1) + (A - 1) * c - sa; break; }
            default: {
                const double sa = 2.0 * std::sqrt(A) * (s / 2.0 * std::sqrt(2.0));
                n0 = A * ((A + 1) + (A - 1) * c + sa); n1 = -2 * A * ((A - 1) + (A + 1) * c); n2 = A * ((A + 1) + (A - 1) * c - sa);
                d0 = (A + 1) - (A - 1) * c + sa; d1 = 2 * ((A - 1) - (A + 1) * c); d2 = (A + 1) - (A - 1) * c - sa; break; }
        }
        b0 = n0 / d0; b1 = n1 / d0; b2 = n2 / d0; a1 = d1 / d0; a2 = d2 / d0;
    }
    float process(float x)
    {
        const double y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return (float) y;
    }
};

// ---------------------------------------------------------------- EQ
struct EqParams
{
    bool  on = true;
    float hpFreq = 20, lsFreq = 150, lsGain = 0, hsFreq = 10000, hsGain = 0, lpFreq = 20000;
    float f[3] = {300, 1500, 4000}, g[3] = {0, 0, 0}, q[3] = {1.2f, 1.0f, 1.0f};
};

class Eq
{
public:
    void prepare(double sr) { fs = sr; reset(); dirty = true; }
    void reset() { for (auto& c : ch) for (auto& b : c) b.reset(); }
    void setParams(const EqParams& p) { P = p; dirty = true; }
    void process(float* l, float* r, int n)
    {
        if (!P.on) return;
        if (dirty) { update(); dirty = false; }
        for (int i = 0; i < n; ++i) {
            float a = l[i], b = r[i];
            if (useHp) { a = ch[0][0].process(a); b = ch[1][0].process(b); }
            for (int k = 1; k < 7; ++k) {
                if (k == 1 && P.lsGain == 0.0f) continue;
                if (k == 2 && P.g[0] == 0.0f) continue;
                if (k == 3 && P.g[1] == 0.0f) continue;
                if (k == 4 && P.g[2] == 0.0f) continue;
                if (k == 5 && P.hsGain == 0.0f) continue;
                if (k == 6 && !useLp) continue;
                a = ch[0][(size_t) k].process(a); b = ch[1][(size_t) k].process(b);
            }
            l[i] = a; r[i] = b;
        }
    }
private:
    void update()
    {
        useHp = P.hpFreq > 20.5f; useLp = P.lpFreq < 19990.0f;
        for (auto& c : ch) {
            c[0].set(Biquad::HighPass, fs, P.hpFreq, 0.7071, 0);
            c[1].set(Biquad::LowShelf, fs, P.lsFreq, 0.7071, P.lsGain);
            for (int k = 0; k < 3; ++k) c[(size_t)(2 + k)].set(Biquad::Bell, fs, P.f[k], P.q[k], P.g[k]);
            c[5].set(Biquad::HighShelf, fs, P.hsFreq, 0.7071, P.hsGain);
            c[6].set(Biquad::LowPass, fs, P.lpFreq, 0.7071, 0);
        }
    }
    double fs = 48000; EqParams P; bool dirty = true, useHp = false, useLp = false;
    std::array<std::array<Biquad, 7>, 2> ch;
};

// ---------------------------------------------------------------- compressor
struct CompParams { bool on = true; float thr = -18, ratio = 3, atkMs = 8, relMs = 120, knee = 6, makeup = 0, mix = 1; };

class Compressor
{
public:
    void prepare(double sr) { fs = sr; gr = 0; lastGr = 0; }
    void setParams(const CompParams& p) { P = p; }
    float grDb() const { return lastGr; }
    void process(float* l, float* r, int n)
    {
        if (!P.on) { lastGr = 0; return; }
        const float ca = std::exp(-1.0f / (std::max(P.atkMs, 0.05f) * 0.001f * (float) fs));
        const float cr = std::exp(-1.0f / (std::max(P.relMs, 1.0f) * 0.001f * (float) fs));
        const float slope = 1.0f / std::max(P.ratio, 1.0f) - 1.0f, knee = std::max(P.knee, 0.0f);
        const float mk = dbToLin(P.makeup);
        for (int i = 0; i < n; ++i) {
            const float lv = linToDb(std::max(std::fabs(l[i]), std::fabs(r[i])));
            const float over = lv - P.thr;
            float target;
            if (2.0f * over < -knee) target = 0.0f;
            else if (2.0f * std::fabs(over) <= knee) target = slope * (over + knee * 0.5f) * (over + knee * 0.5f) / (2.0f * std::max(knee, 1e-6f));
            else target = slope * over;
            const float c = target < gr ? ca : cr;
            gr = flushDenormal(c * gr + (1.0f - c) * target);
            const float g = dbToLin(gr) * mk;
            l[i] = l[i] * (1.0f - P.mix) + l[i] * g * P.mix;
            r[i] = r[i] * (1.0f - P.mix) + r[i] * g * P.mix;
        }
        lastGr = gr;
    }
private:
    double fs = 48000; CompParams P; float gr = 0, lastGr = 0;
};

// ---------------------------------------------------------------- de-esser (split-band)
struct DeEssParams { bool on = true; float freq = 7000, thr = -30, amount = 0.5f; };

class DeEsser
{
public:
    void prepare(double sr) { fs = sr; env = 0; for (auto& b : hp) b.reset(); for (auto& b : bell) b.reset(); dirty = true; red = 0; }
    void setParams(const DeEssParams& p) { if (p.freq != P.freq) dirty = true; P = p; }
    // detection = high-passed sidechain; action = dynamic bell cut at the sibilance centre (phase-clean, no band subtraction)
    void process(float* l, float* r, int n)
    {
        if (!P.on || P.amount <= 0.0f) return;
        if (dirty) { for (auto& b : hp) b.set(Biquad::HighPass, fs, P.freq, 0.7071, 0); dirty = false; }
        const float ca = std::exp(-1.0f / (0.001f * (float) fs)), cr = std::exp(-1.0f / (0.04f * (float) fs));
        for (int i = 0; i < n; ++i) {
            const float bl = hp[0].process(l[i]), br = hp[1].process(r[i]);
            const float lvl = std::max(std::fabs(bl), std::fabs(br));
            env = flushDenormal(lvl > env ? ca * env + (1 - ca) * lvl : cr * env + (1 - cr) * lvl);
            const float over = linToDb(env) - P.thr;
            const float target = over > 0.0f ? std::min(15.0f, over * P.amount) : 0.0f;
            red += (target - red) * 0.02f;
            if ((i & 15) == 0 && (red > 0.05f || lastRed > 0.05f)) {
                Biquad t; t.set(Biquad::Bell, fs, std::min(P.freq * 1.25, 0.45 * fs), 0.9, -red);
                for (auto& b : bell) { b.b0 = t.b0; b.b1 = t.b1; b.b2 = t.b2; b.a1 = t.a1; b.a2 = t.a2; }
                lastRed = red;
            }
            if (red > 0.05f || lastRed > 0.05f) { l[i] = bell[0].process(l[i]); r[i] = bell[1].process(r[i]); }
        }
    }
private:
    double fs = 48000; DeEssParams P; float env = 0, red = 0, lastRed = 0; bool dirty = true; std::array<Biquad, 2> hp, bell;
};

// ---------------------------------------------------------------- saturator (2x oversampled, alias-suppressed)
struct SatParams { bool on = true; float drive = 0, mix = 1; };

class Saturator
{
public:
    static constexpr int kTaps = 49;                    // odd, linear phase
    static constexpr int kLatency = (kTaps - 1) / 2;    // total latency in BASE-rate samples (up + down filter) = 24

    Saturator()
    {
        const int M = (kTaps - 1) / 2;
        double sum = 0;
        for (int n = 0; n < kTaps; ++n) {
            const double k = n - M;
            const double s = (k == 0) ? 0.5 : std::sin(kPi * 0.5 * k) / (kPi * k);                       // cutoff = fs_base/2
            const double w = 0.42 - 0.5 * std::cos(2 * kPi * n / (kTaps - 1)) + 0.08 * std::cos(4 * kPi * n / (kTaps - 1));  // Blackman
            h[(size_t) n] = s * w; sum += s * w;
        }
        for (auto& v : h) v = v / sum;
        reset();
    }
    void reset() { amt = 0.0f; for (auto& c : ch) { c.up.fill(0.0); c.dn.fill(0.0); c.dry.fill(0.0f); c.pu = 0; c.pd = 0; c.pr = 0; } }
    static int latency() { return kLatency; }

    void process(const SatParams& P, float* l, float* r, int n)
    {
        const bool act = P.on && P.drive > 0.0f;
        const double pre = 1.0 + 9.0 * (double) P.drive, norm = 1.0 / std::tanh(pre), mix = (double) P.mix;
        float* io[2] = { l, r };
        for (int i = 0; i < n; ++i) {
            amt += act ? 1.0f / 256.0f : -1.0f / 256.0f; amt = std::min(1.0f, std::max(0.0f, amt));   // ~5 ms crossfade when switching
            for (int c = 0; c < 2; ++c) {
                Chan& s = ch[(size_t) c];
                const float in = io[c][i];
                s.dry[(size_t) s.pr] = in; s.pr = (s.pr + 1) % kDry;                                    // pure delay path (full bandwidth when inactive)
                const float dryOut = s.dry[(size_t)((s.pr + kDry - 1 - kLatency + kDry) % kDry)];
                if (amt <= 0.0f && !act) { io[c][i] = dryOut; continue; }
                s.pu = (s.pu + 1) % kUp; s.up[(size_t) s.pu] = in;
                double y2[2];
                for (int ph = 0; ph < 2; ++ph) {                 // polyphase upsample: even / odd taps
                    double acc = 0.0;
                    for (int j = ph, t = 0; j < kTaps; j += 2, ++t) acc += h[(size_t) j] * s.up[(size_t)((s.pu - t + kUp) % kUp)];
                    acc *= 2.0;
                    y2[ph] = acc * (1.0 - mix) + std::tanh(pre * acc) * norm * mix;
                }
                s.pd = (s.pd + 1) % kDn; s.dn[(size_t) s.pd] = y2[0];
                double out = 0.0;                                   // decimate on the even phase -> exactly kLatency base samples of delay
                for (int j = 0; j < kTaps; ++j) out += h[(size_t) j] * s.dn[(size_t)((s.pd - j + kDn) % kDn)];
                s.pd = (s.pd + 1) % kDn; s.dn[(size_t) s.pd] = y2[1];
                io[c][i] = dryOut + ((float) out - dryOut) * amt;
            }
        }
    }
private:
    static constexpr int kUp = (kTaps + 1) / 2 + 1, kDn = kTaps + 1;
    static constexpr int kDry = kLatency + 8;
    struct Chan { std::array<double, kUp> up; std::array<double, kDn> dn; std::array<float, kDry> dry; int pu = 0, pd = 0, pr = 0; };
    std::array<double, kTaps> h{};
    std::array<Chan, 2> ch;
    float amt = 0.0f;
};

// ---------------------------------------------------------------- doubler (1-4 detuned, delayed, panned voices)
struct DblParams { float amount = 0; int voices = 2; float delayMs = 22, detune = 10, width = 1, tone = 9000, octave = 0; };

class Doubler
{
public:
    void prepare(double sr)
    {
        fs = sr; size_t n = 1; while ((double) n < 0.25 * fs) n <<= 1;
        buf.assign(n, 0.0f); mask = n - 1; w = 0; ph = {0.0, 0.9, 2.1, 3.7};
        for (auto& v : lp) v = 0;
        oct.assign(n, 0.0f); octPh = 0.0;
    }
    void setParams(const DblParams& p) { P = p; }
    void process(float* l, float* r, int n)
    {
        if (P.amount <= 0.0f) { for (int i = 0; i < n; ++i) { buf[w & mask] = 0.5f * (l[i] + r[i]); ++w; } return; }
        const int V = std::min(4, std::max(1, P.voices));
        const float gain = P.amount * 0.75f / std::sqrt((float) V * 0.5f + 0.5f);
        const float a = std::exp(-2.0f * (float) kPi * std::min(P.tone, 20000.0f) / (float) fs);
        static const double rate[4] = {0.37, 0.53, 0.29, 0.61};
        const double ctr = std::pow(2.0, (double) P.detune / 1200.0) - 1.0;
        for (int i = 0; i < n; ++i) {
            const float mono = 0.5f * (l[i] + r[i]);
            buf[w & mask] = mono;
            float wl = 0, wr = 0;
            if (P.octave > 0.0f) {                       // octave-up voice: two cross-faded read heads moving at 2x speed
                oct[(size_t)(w & (int64_t) mask)] = mono;
                const double D = std::min(0.03 * fs, (double) mask - 8.0);
                octPh += 1.0 / D; if (octPh >= 1.0) octPh -= 1.0;
                float o = 0.0f;
                for (int h = 0; h < 2; ++h) {
                    double ph = octPh + 0.5 * h; if (ph >= 1.0) ph -= 1.0;
                    const double d = std::max(2.0, D * (1.0 - ph));
                    const double rp = (double) w - d; const int64_t ip = (int64_t) std::floor(rp); const float fr = (float)(rp - (double) ip);
                    const float s = oct[(size_t)(ip & (int64_t) mask)] * (1 - fr) + oct[(size_t)((ip + 1) & (int64_t) mask)] * fr;
                    const float g = (float) std::sin(kPi * ph);
                    o += s * g * g;
                }
                wl += o * P.octave * 1.2f; wr += o * P.octave * 1.2f;
            }
            for (int v = 0; v < V; ++v) {
                const double f = rate[v];
                const double depthMs = 1000.0 * ctr / (2.0 * kPi * f);
                ph[(size_t) v] += 2.0 * kPi * f / fs; if (ph[(size_t) v] > 2 * kPi) ph[(size_t) v] -= 2 * kPi;
                const double lfo = 0.7 * std::sin(ph[(size_t) v]) + 0.3 * std::sin(2.7 * ph[(size_t) v] + 1.3);
                const double dMs = (double) P.delayMs * (1.0 + 0.4 * v) + depthMs * lfo;
                double d = std::min(std::max(dMs * 0.001 * fs, 2.0), (double) mask - 4.0);
                const double rp = (double) w - d; const int64_t ip = (int64_t) std::floor(rp); const float fr = (float)(rp - (double) ip);
                const float s = buf[(size_t)(ip & (int64_t) mask)] * (1 - fr) + buf[(size_t)((ip + 1) & (int64_t) mask)] * fr;
                lp[(size_t) v] = flushDenormal(s * (1 - a) + lp[(size_t) v] * a);
                const float side = (v % 2 == 0) ? -1.0f : 1.0f, spread = (v < 2 ? 1.0f : 0.5f) * P.width;
                const float pan = side * spread, ang = (pan + 1.0f) * (float) kPi * 0.25f;
                wl += lp[(size_t) v] * std::cos(ang) * 1.4142f; wr += lp[(size_t) v] * std::sin(ang) * 1.4142f;
            }
            ++w;
            l[i] += wl * gain; r[i] += wr * gain;
        }
    }
private:
    double fs = 48000; DblParams P; std::vector<float> buf, oct; size_t mask = 0; int64_t w = 0; double octPh = 0; std::array<double, 4> ph{}; std::array<float, 4> lp{};
};

// ---------------------------------------------------------------- delay
struct DelayParams { bool on = true; float timeMs = 350, fb = 0.3f, tone = 5000, hp = 150, mix = 0; bool ping = true; };

class Delay
{
public:
    void prepare(double sr)
    {
        fs = sr; size_t n = 1; while ((double) n < 2.2 * fs) n <<= 1;
        bl.assign(n, 0.0f); br.assign(n, 0.0f); mask = (int64_t) n - 1; w = 0; cur = -1; dirty = true;
        for (auto& b : lpf) b.reset();
        for (auto& b : hpf) b.reset();
    }
    void setParams(const DelayParams& p) { if (p.tone != P.tone || p.hp != P.hp) dirty = true; P = p; }
    void process(float* l, float* r, int n)
    {
        if (!P.on || P.mix <= 0.0f) { for (int i = 0; i < n; ++i) { bl[(size_t)(w & mask)] = br[(size_t)(w & mask)] = 0; ++w; } return; }
        if (dirty) { for (int c = 0; c < 2; ++c) { lpf[(size_t) c].set(Biquad::LowPass, fs, P.tone, 0.7071, 0); hpf[(size_t) c].set(Biquad::HighPass, fs, P.hp, 0.7071, 0); } dirty = false; }
        const double target = std::min(std::max((double) P.timeMs * 0.001 * fs, 8.0), (double) mask - 8.0);
        if (cur < 0) cur = target;
        const float fb = std::min(P.fb, 0.95f);
        for (int i = 0; i < n; ++i) {
            cur += (target - cur) * 0.0005;
            const double rp = (double) w - cur; const int64_t ip = (int64_t) std::floor(rp); const float fr = (float)(rp - (double) ip);
            const size_t i0 = (size_t)(ip & mask), i1 = (size_t)((ip + 1) & mask);
            const float dl = bl[i0] * (1 - fr) + bl[i1] * fr, dr = br[i0] * (1 - fr) + br[i1] * fr;
            const float fl = hpf[0].process(lpf[0].process(dl)), fr2 = hpf[1].process(lpf[1].process(dr));
            float wl, wr;
            if (P.ping) { wl = 0.5f * (l[i] + r[i]) + fb * fr2; wr = fb * fl; }
            else        { wl = l[i] + fb * fl;                 wr = r[i] + fb * fr2; }
            bl[(size_t)(w & mask)] = flushDenormal(std::max(-8.0f, std::min(8.0f, wl)));
            br[(size_t)(w & mask)] = flushDenormal(std::max(-8.0f, std::min(8.0f, wr)));
            ++w;
            l[i] += dl * P.mix; r[i] += dr * P.mix;
        }
    }
private:
    double fs = 48000, cur = -1; DelayParams P; std::vector<float> bl, br; int64_t mask = 0, w = 0; bool dirty = true;
    std::array<Biquad, 2> lpf, hpf;
};

// ---------------------------------------------------------------- reverb (Freeverb core + predelay + wet filters)
struct ReverbParams { bool on = true; float size = 0.55f, damp = 0.5f, preMs = 20, width = 1, low = 200, high = 10000, mix = 0; };

class Reverb
{
public:
    void prepare(double sr)
    {
        fs = sr;
        static const int ct[8] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617}, at[4] = {556, 441, 341, 225};
        const double k = sr / 44100.0;
        for (int c = 0; c < 2; ++c) {
            for (int i = 0; i < 8; ++i) { combs[(size_t) c][(size_t) i].init((int)(ct[i] * k) + c * (int)(23 * k)); }
            for (int i = 0; i < 4; ++i) { aps[(size_t) c][(size_t) i].init((int)(at[i] * k) + c * (int)(23 * k)); }
        }
        diff[0].init((int)(142 * k)); diff[1].init((int)(107 * k));
        size_t n = 1; while ((double) n < 0.25 * sr) n <<= 1; pre.assign(n, 0.0f); pmask = (int64_t) n - 1; pw = 0; dirty = true;
        for (auto& b : lo) b.reset();
        for (auto& b : hi) b.reset();
    }
    void setParams(const ReverbParams& p) { if (p.low != P.low || p.high != P.high) dirty = true; P = p; }
    void process(float* l, float* r, int n)
    {
        if (!P.on || P.mix <= 0.0f) { for (int i = 0; i < n; ++i) { pre[(size_t)(pw & pmask)] = 0; ++pw; } return; }
        if (dirty) { for (int c = 0; c < 2; ++c) { lo[(size_t) c].set(Biquad::HighPass, fs, P.low, 0.7071, 0); hi[(size_t) c].set(Biquad::LowPass, fs, P.high, 0.7071, 0); } dirty = false; }
        const float fbk = 0.7f + 0.28f * std::min(1.0f, std::max(0.0f, P.size));
        const float damp = 0.05f + 0.85f * std::min(1.0f, std::max(0.0f, P.damp));
        const int64_t pd = std::min<int64_t>((int64_t) std::lround(P.preMs * 0.001 * fs), pmask - 2);
        const float w1 = P.width * 0.5f + 0.5f, w2 = (1.0f - P.width) * 0.5f;
        for (int i = 0; i < n; ++i) {
            pre[(size_t)(pw & pmask)] = (l[i] + r[i]) * 0.015f;
            float in = pre[(size_t)((pw - pd) & pmask)]; ++pw;
            in = diff[1].process(diff[0].process(in));          // input diffusion: denser, smoother tail
            float o[2] = {0, 0};
            for (int c = 0; c < 2; ++c) {
                for (auto& cb : combs[(size_t) c]) o[c] += cb.process(in, fbk, damp);
                for (auto& ap : aps[(size_t) c]) o[c] = ap.process(o[c]);
                o[c] = hi[(size_t) c].process(lo[(size_t) c].process(o[c]));
            }
            const float wl = (o[0] * w1 + o[1] * w2) * 3.0f, wr = (o[1] * w1 + o[0] * w2) * 3.0f;
            l[i] += wl * P.mix; r[i] += wr * P.mix;
        }
    }
private:
    struct Comb { std::vector<float> b; size_t i = 0; float st = 0;
        void init(int n) { b.assign((size_t) std::max(n, 2), 0.0f); i = 0; st = 0; }
        float process(float in, float fb, float damp) { const float y = b[i]; st = flushDenormal(y * (1 - damp) + st * damp); b[i] = flushDenormal(in + st * fb); if (++i >= b.size()) i = 0; return y; } };
    struct Ap { std::vector<float> b; size_t i = 0;
        void init(int n) { b.assign((size_t) std::max(n, 2), 0.0f); i = 0; }
        float process(float in) { const float y = b[i]; const float o = y - in; b[i] = flushDenormal(in + y * 0.5f); if (++i >= b.size()) i = 0; return o; } };
    double fs = 48000; ReverbParams P; bool dirty = true;
    std::array<std::array<Comb, 8>, 2> combs; std::array<std::array<Ap, 4>, 2> aps; std::array<Ap, 2> diff;
    std::vector<float> pre; int64_t pmask = 0, pw = 0; std::array<Biquad, 2> lo, hi;
};

// ---------------------------------------------------------------- lookahead limiter (true ceiling, no overshoot)
// gain = box-filter( running-min( required gain ) ), audio delayed by N-1 samples: the gain has fully arrived when a peak plays.
struct Limiter
{
    bool on = true; float ceilDb = -0.3f; double fs = 48000; int N = 72;
    int latency() const { return N - 1; }
    void prepare(double sr)
    {
        fs = sr; N = std::max(3, (int) std::lround(0.0015 * sr));
        dl.assign((size_t) N, 0.0f); dr.assign((size_t) N, 0.0f);
        reqRing.assign((size_t) N, 1.0f); relRing.assign((size_t) N, 1.0f); dqIdx.assign((size_t) N + 2, 0); dqVal.assign((size_t) N + 2, 1.0f);
        head = tail = 0; cnt = 0; relState = 1.0f; boxSum = (double) N; t = 0;
    }
    void process(float* l, float* r, int n)
    {
        const float c = dbToLin(ceilDb), rel = 1.0f - std::exp(-1.0f / (0.08f * (float) fs));
        const size_t cap = dqIdx.size();
        for (int i = 0; i < n; ++i) {
            const float inL = l[i], inR = r[i];
            const float pk = std::max(std::fabs(inL), std::fabs(inR));
            const float req = (on && pk > c) ? c / pk : 1.0f;
            // running minimum of req over the last N samples (monotonic deque)
            while (cnt > 0 && dqVal[(tail + cap - 1) % cap] >= req) { tail = (tail + cap - 1) % cap; --cnt; }
            dqIdx[tail] = t; dqVal[tail] = req; tail = (tail + 1) % cap; ++cnt;
            while (cnt > 0 && dqIdx[head] <= t - N) { head = (head + 1) % cap; --cnt; }
            const float gmin = dqVal[head];
            relState = gmin < relState ? gmin : relState + (gmin - relState) * rel;          // instant attack, smooth release
            const size_t k = (size_t)(t % N);
            boxSum += (double) relState - (double) relRing[k]; relRing[k] = relState;         // box filter over the last N values
            const float g = (float)(boxSum / (double) N);
            // audio delayed by N-1 samples: slot (t+1)%N holds the sample written N-1 steps ago
            const size_t idx = (size_t)(t % N), oidx = (size_t)((t + 1) % N);
            dl[idx] = inL; dr[idx] = inR;
            float outL = dl[oidx] * g, outR = dr[oidx] * g;
            if (on) { outL = std::max(-c, std::min(c, outL)); outR = std::max(-c, std::min(c, outR)); }
            l[i] = outL; r[i] = outR;
            ++t;
        }
    }
private:
    std::vector<float> dl, dr, reqRing, relRing, dqVal; std::vector<int64_t> dqIdx;
    size_t head = 0, tail = 0, cnt = 0; float relState = 1.0f; double boxSum = 0; int64_t t = 0;
};

} // namespace ds
