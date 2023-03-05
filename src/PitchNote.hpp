#pragma once
#include <rack.hpp>

namespace Chinenual {

inline float pitchToVoltage(int note)
{
    // based on VCV CORE MIDI_CV:
    return (note - 60.f) / 12.f;
}
inline int voltageToPitch(float v)
{
    // based on VCV CORE CV_MIDI:
    return (int)std::round(v * 12.f + 60.f);
}
inline float voltageToMicroPitch(float v)
{
    // based on VCV CORE CV_MIDI:
    return (v * 12.f + 60.f);
}
/* deviation from equal temperament */
inline float voltageToPitchDeviation(float v)
{
    return voltageToMicroPitch(v) - (float)voltageToPitch(v);
}

/* Careful: noteDeviation is scalled by note value - not by voltage */
inline void pitchToText(std::string& text, int note, float noteDeviation)
{
    // in case noteDeviation is larger than a semitone, apply it first and compute a new "smaller than semitone" deviation:
    int n = std::round(note + noteDeviation);
    float nDeviation = noteDeviation - (n - note);

    const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int nameIndex = n % 12;
    int octave = (n / 12) - 1; // simple division produces 60=C5; we want to display that as C4, so subtract 1
    auto absDeviation = std::abs(nDeviation);
    if (absDeviation >= 0.01f) {
        text = rack::string::f("%s%d %s%dc", noteNames[nameIndex], octave,
            (nDeviation > 0 ? "+" : "-"),
            (int)(absDeviation * 100));
    } else {
        text = rack::string::f("%s%d", noteNames[nameIndex], octave);
    }
}

}
