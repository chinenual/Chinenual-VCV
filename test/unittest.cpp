#define CATCH_CONFIG_MAIN

#include "CVRange.hpp"
#include "PitchNote.hpp"
#include "TintQuantizer.hpp"
#undef WARN

#include "catch.hpp"

using namespace Chinenual;
using namespace MIDIRecorder;
using namespace Catch;

TEST_CASE("enums select correct range")
{
    CHECK(CVRanges[CV_RANGE_n10_10].low == -10.0f);
    CHECK(CVRanges[CV_RANGE_n10_10].high == 10.0f);

    CHECK(CVRanges[CV_RANGE_0_10].low == 0.0f);
    CHECK(CVRanges[CV_RANGE_0_10].high == 10.0f);

    CHECK(CVRanges[CV_RANGE_n5_5].low == -5.0f);
    CHECK(CVRanges[CV_RANGE_n5_5].high == 5.0f);

    CHECK(CVRanges[CV_RANGE_0_5].low == 0.0f);
    CHECK(CVRanges[CV_RANGE_0_5].high == 5.0f);

    CHECK(CVRanges[CV_RANGE_n3_3].low == -3.0f);
    CHECK(CVRanges[CV_RANGE_n3_3].high == 3.0f);

    CHECK(CVRanges[CV_RANGE_0_3].low == 0.0f);
    CHECK(CVRanges[CV_RANGE_0_3].high == 3.0f);
}

TEST_CASE("enums select correct strings")
{
    CHECK(CVRangeNames[CV_RANGE_n10_10] == "-10 to 10");
    CHECK(CVRangeNames[CV_RANGE_0_10] == "  0 to 10");
    CHECK(CVRangeNames[CV_RANGE_n5_5] == " -5 to  5");
    CHECK(CVRangeNames[CV_RANGE_0_5] == "  0 to  5");
    CHECK(CVRangeNames[CV_RANGE_n3_3] == " -3 to  3");
    CHECK(CVRangeNames[CV_RANGE_0_3] == "  0 to  3");
}

TEST_CASE("7bit conversions low/high")
{
    for (int i = CV_RANGE_0_10; i <= CV_RANGE_0_3; i++) {
        CVRange r = CVRanges[i];
        CHECK(r.to7bit(r.low) == 0);
        CHECK(r.to7bit(r.high) == 127);
    }
}
TEST_CASE("14bit conversions low/high")
{
    for (int i = CV_RANGE_0_10; i <= CV_RANGE_0_3; i++) {
        CVRange r = CVRanges[i];
        CHECK(r.to14bit(r.low) == 0);
        CHECK(r.to14bit(r.high) == 16383);
    }
}
TEST_CASE("14bit conversions msb/lsb")
{
    for (int i = CV_RANGE_0_10; i <= CV_RANGE_0_3; i++) {
        CVRange r = CVRanges[i];
        auto v = r.to14bit(r.low);
        int msb, lsb;
        r.split14bit(v, msb, lsb);
        CHECK(lsb == 0);
        CHECK(msb == 0);

        v = r.to14bit(r.high);
        r.split14bit(v, msb, lsb);
        CHECK(lsb == 127);
        CHECK(msb == 127);
    }
}

// -----------------------------------------------

TEST_CASE("v/oct pitch conversion basics")
{
    CHECK(voltageToPitch(1.f) == 72); // C5
    CHECK(voltageToPitch(0.f) == 60); // C4
    CHECK(voltageToPitch(-1.f) == 48); // C3
    // extreme ranges:
    CHECK(voltageToPitch(PITCH_VOCT_MIN) == PITCH_NOTE_MIN); // C-6
    CHECK(voltageToPitch(PITCH_VOCT_MAX) == PITCH_NOTE_MAX); // C14
}

TEST_CASE("v/oct pitch reverse conversion basics")
{
    CHECK((1.f) == pitchToVoltage(72)); // C5
    CHECK((0.f) == pitchToVoltage(60)); // C4
    CHECK((-1.f) == pitchToVoltage(48)); // C3
    // extreme ranges:
    CHECK((PITCH_VOCT_MIN) == pitchToVoltage(PITCH_NOTE_MIN)); // C-6
    CHECK((PITCH_VOCT_MAX) == pitchToVoltage(PITCH_NOTE_MAX)); // C14
}

TEST_CASE("micro v/oct pitch conversion basics")
{
    CHECK(voltageToMicroPitch(1.f) == 72.f); // C5
    CHECK(voltageToMicroPitch(0.f) == 60.f); // C4
    CHECK(voltageToMicroPitch(-1.f) == 48.f); // C3
}

TEST_CASE("micro v/oct pitch reverse conversion")
{
    float epsilon = 0.0001f;
    REQUIRE_THAT(microPitchToVoltage(72.f), WithinAbs(1.f, epsilon)); // C5
    REQUIRE_THAT(microPitchToVoltage(60.f), WithinAbs(0.f, epsilon)); // C4
    REQUIRE_THAT(microPitchToVoltage(48.f), WithinAbs(-1.f, epsilon)); // C3

    REQUIRE_THAT(microPitchToVoltage(60.5f), WithinAbs(0.0416666f, epsilon)); // C4 +50c
}

TEST_CASE("micro v/oct pitch conversion deviations")
{
    float semi = .0833333333f;
    float cent = semi / 100.f;
    float epsilon = 0.0001f;
    REQUIRE_THAT(voltageToPitchDeviation(0.f), WithinAbs(0.f, epsilon)); // C4 exactly
    REQUIRE_THAT(voltageToPitchDeviation(1.f), WithinAbs(0.f, epsilon)); // C5 exactly
    REQUIRE_THAT(voltageToPitchDeviation(20 * cent), WithinAbs(0.2f, epsilon)); // C4 +20c
    REQUIRE_THAT(voltageToPitchDeviation(-20 * cent), WithinAbs(-0.2f, epsilon)); // C4 -20c
}

TEST_CASE("pitch string conversion basics")
{
    std::string str;
    pitchToText(str, 72, 0.f); // C5
    CHECK("C5" == str);
    pitchToText(str, 60, 0.f); // C4
    CHECK("C4" == str);
    pitchToText(str, 48, 0.f); // C3
    CHECK("C3" == str);
}

TEST_CASE("pitch string conversion semitones")
{
    std::string str;
    pitchToText(str, 60, 0.f); // C4
    CHECK("C4" == str);
    pitchToText(str, 61, 0.f); // C#4
    CHECK("C#4" == str);
    pitchToText(str, 62, 0.f); // D4
    CHECK("D4" == str);
    pitchToText(str, 63, 0.f); // D#4
    CHECK("D#4" == str);
    pitchToText(str, 64, 0.f); // E4
    CHECK("E4" == str);
    pitchToText(str, 65, 0.f); // F4
    CHECK("F4" == str);
    pitchToText(str, 66, 0.f); // F#4
    CHECK("F#4" == str);
    pitchToText(str, 67, 0.f); // G4
    CHECK("G4" == str);
    pitchToText(str, 68, 0.f); // G#4
    CHECK("G#4" == str);
    pitchToText(str, 69, 0.f); // A4
    CHECK("A4" == str);
    pitchToText(str, 70, 0.f); // A#4
    CHECK("A#4" == str);
    pitchToText(str, 71, 0.f); // B4
    CHECK("B4" == str);
    pitchToText(str, 72, 0.f); // C5
    CHECK("C5" == str);
}

TEST_CASE("pitch string conversion cents")
{
    std::string str;
    pitchToText(str, 60, 0.49); // C4 +49c
    CHECK("C4 +49c" == str);
    pitchToText(str, 60, -0.49); // C4 -49c
    CHECK("C4 -49c" == str);
    pitchToText(str, 60, 0.5); // C4 -50c
    CHECK("C#4 -50c" == str);
    pitchToText(str, 60, -0.4); // C4 -40c
    CHECK("C4 -40c" == str);
    pitchToText(str, 60, 1.f); // C#4
    CHECK("C#4" == str);
    pitchToText(str, 60, 2.f); // D4
    CHECK("D4" == str);
}

TEST_CASE("pitch string conversion extremes")
{
    std::string str;
    pitchToText(str, voltageToPitch(10.f), 0.f); // C14
    CHECK("C14" == str);
    pitchToText(str, voltageToPitch(-10.f), 0.f); // C-6
    CHECK("C-6" == str);
}

TEST_CASE("pitch string octaves")
{
    std::string str;
    pitchToText(str, voltageToPitch(-10.f), 0.f);
    CHECK("C-6" == str);
    pitchToText(str, voltageToPitch(-9.f), 0.f);
    CHECK("C-5" == str);
    pitchToText(str, voltageToPitch(-8.f), 0.f);
    CHECK("C-4" == str);
    pitchToText(str, voltageToPitch(-7.f), 0.f);
    CHECK("C-3" == str);
    pitchToText(str, voltageToPitch(-6.f), 0.f);
    CHECK("C-2" == str);
    pitchToText(str, voltageToPitch(-5.f), 0.f);
    CHECK("C-1" == str);
    pitchToText(str, voltageToPitch(-4.f), 0.f);
    CHECK("C0" == str);
    pitchToText(str, voltageToPitch(-3.f), 0.f);
    CHECK("C1" == str);
    pitchToText(str, voltageToPitch(-2.f), 0.f);
    CHECK("C2" == str);
    pitchToText(str, voltageToPitch(-1.f), 0.f);
    CHECK("C3" == str);
    pitchToText(str, voltageToPitch(0.f), 0.f);
    CHECK("C4" == str);
    pitchToText(str, voltageToPitch(1.f), 0.f);
    CHECK("C5" == str);
    pitchToText(str, voltageToPitch(2.f), 0.f);
    CHECK("C6" == str);
    pitchToText(str, voltageToPitch(3.f), 0.f);
    CHECK("C7" == str);
    pitchToText(str, voltageToPitch(4.f), 0.f);
    CHECK("C8" == str);
    pitchToText(str, voltageToPitch(5.f), 0.f);
    CHECK("C9" == str);
    pitchToText(str, voltageToPitch(6.f), 0.f);
    CHECK("C10" == str);
    pitchToText(str, voltageToPitch(7.f), 0.f);
    CHECK("C11" == str);
    pitchToText(str, voltageToPitch(8.f), 0.f);
    CHECK("C12" == str);
    pitchToText(str, voltageToPitch(9.f), 0.f);
    CHECK("C13" == str);
    pitchToText(str, voltageToPitch(10.f), 0.f);
    CHECK("C14" == str);
}

TEST_CASE("pitch string octaves - w/ deviations")
{
    std::string str;
    pitchToText(str, voltageToPitch(-10.f), 0.4f);
    CHECK("C-6 +40c" == str);
    pitchToText(str, voltageToPitch(-9.f), 0.4f);
    CHECK("C-5 +40c" == str);
    pitchToText(str, voltageToPitch(-8.f), 0.4f);
    CHECK("C-4 +40c" == str);
    pitchToText(str, voltageToPitch(-7.f), 0.4f);
    CHECK("C-3 +40c" == str);
    pitchToText(str, voltageToPitch(-6.f), 0.4f);
    CHECK("C-2 +40c" == str);
    pitchToText(str, voltageToPitch(-5.f), 0.4f);
    CHECK("C-1 +40c" == str);
    pitchToText(str, voltageToPitch(-4.f), 0.4f);
    CHECK("C0 +40c" == str);
    pitchToText(str, voltageToPitch(-3.f), 0.4f);
    CHECK("C1 +40c" == str);
    pitchToText(str, voltageToPitch(-2.f), 0.4f);
    CHECK("C2 +40c" == str);
    pitchToText(str, voltageToPitch(-1.f), 0.4f);
    CHECK("C3 +40c" == str);
    pitchToText(str, voltageToPitch(0.f), 0.4f);
    CHECK("C4 +40c" == str);
    pitchToText(str, voltageToPitch(1.f), 0.4f);
    CHECK("C5 +40c" == str);
    pitchToText(str, voltageToPitch(2.f), 0.4f);
    CHECK("C6 +40c" == str);
    pitchToText(str, voltageToPitch(3.f), 0.4f);
    CHECK("C7 +40c" == str);
    pitchToText(str, voltageToPitch(4.f), 0.4f);
    CHECK("C8 +40c" == str);
    pitchToText(str, voltageToPitch(5.f), 0.4f);
    CHECK("C9 +40c" == str);
    pitchToText(str, voltageToPitch(6.f), 0.4f);
    CHECK("C10 +40c" == str);
    pitchToText(str, voltageToPitch(7.f), 0.4f);
    CHECK("C11 +40c" == str);
    pitchToText(str, voltageToPitch(8.f), 0.4f);
    CHECK("C12 +40c" == str);
    pitchToText(str, voltageToPitch(9.f), 0.4f);
    CHECK("C13 +40c" == str);
    pitchToText(str, voltageToPitch(10.f), 0.4f);
    CHECK("C14 +40c" == str);
}

// -----------------------------------------------
using namespace Tint;

// note pitches (MIDI note values) used in the tests
#define C4 60
#define D4 62
#define E4 64
#define F4 65
#define G4 67
#define A4 69
#define B4 71

#define C3 (C4 - 12)
#define D3 (D4 - 12)
#define E3 (E4 - 12)
#define F3 (F4 - 12)
#define G3 (G4 - 12)
#define A3 (A4 - 12)
#define B3 (B4 - 12)

#define C5 (C4 + 12)
#define D5 (D4 + 12)
#define E5 (E4 + 12)
#define F5 (F4 + 12)
#define G5 (G4 + 12)
#define A5 (A4 + 12)
#define B5 (B4 + 12)

TEST_CASE("tintinabulator: basic quantization")
{
    TintQuantizer tq;

    tq.reset();
    tq.mode = TintQuantizer::MODE_QUANTIZE;
    tq.octave = 0;

    tq.chordInputVoltageState[0] = pitchToVoltage(C4);
    tq.chordInputVoltageState[1] = pitchToVoltage(E4);
    tq.chordInputVoltageState[2] = pitchToVoltage(G4);

    tq.setChordFreqs(3);

    // since the reference chord is using 12-TET, make the assertions in terms of pitch "note" rather than
    // frequency.  easier to debug if something goes haywire for the basics.
    CHECK(C4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3))));
    CHECK(C4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4))));
    CHECK(E4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4))));
    CHECK(E4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4))));
    CHECK(G4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4))));
    CHECK(G4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4))));

    // and should work at any octave:

    CHECK(C4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3 + 12))));
    CHECK(C4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4 + 12))));
    CHECK(E4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4 + 12))));
    CHECK(E4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4 + 12))));
    CHECK(G4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4 + 12))));
    CHECK(G4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4 + 12))));
}

TEST_CASE("tintinabulator: tint UP")
{
    TintQuantizer tq;

    tq.reset();
    tq.mode = TintQuantizer::MODE_UP;
    tq.octave = 0;

    tq.chordInputVoltageState[0] = pitchToVoltage(C4);
    tq.chordInputVoltageState[1] = pitchToVoltage(E4);
    tq.chordInputVoltageState[2] = pitchToVoltage(G4);

    tq.setChordFreqs(3);

    // since the reference chord is using 12-TET, make the assertions in terms of pitch "note" rather than
    // frequency.  easier to debug if something goes haywire for the basics.
    CHECK(C4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3))));
    CHECK(E4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4))));
    CHECK(G4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4))));
    CHECK(G4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4))));
    CHECK(C5 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4))));
    CHECK(C5 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4))));

    // and should work at any octave:

    CHECK(C4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3 + 12))));
    CHECK(E4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4 + 12))));
    CHECK(G4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4 + 12))));
    CHECK(G4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4 + 12))));
    CHECK(C5 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4 + 12))));
    CHECK(C5 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4 + 12))));
}

TEST_CASE("tintinabulator: tint DOWN")
{
    TintQuantizer tq;

    tq.reset();
    tq.mode = TintQuantizer::MODE_DOWN;
    tq.octave = 0;

    tq.chordInputVoltageState[0] = pitchToVoltage(C4);
    tq.chordInputVoltageState[1] = pitchToVoltage(E4);
    tq.chordInputVoltageState[2] = pitchToVoltage(G4);

    tq.setChordFreqs(3);

    // since the reference chord is using 12-TET, make the assertions in terms of pitch "note" rather than
    // frequency.  easier to debug if something goes haywire for the basics.
    CHECK(G3 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3))));
    CHECK(G3 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4))));
    CHECK(C4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4))));
    CHECK(E4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4))));
    CHECK(E4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4))));
    CHECK(G4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4))));

    // and should work at any octave:

    CHECK(G3 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3 + 12))));
    CHECK(G3 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4 + 12))));
    CHECK(C4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4 + 12))));
    CHECK(E4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4 + 12))));
    CHECK(E4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4 + 12))));
    CHECK(G4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4 + 12))));

    // and the octave param should do its thing:
    tq.octave = 1;
    CHECK(G3 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3))));
    CHECK(G3 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4))));
    CHECK(C4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4))));
    CHECK(E4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4))));
    CHECK(E4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4))));
    CHECK(G4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4))));

    tq.octave = 2;
    CHECK(G3 + 2 * 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3))));
    CHECK(G3 + 2 * 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4))));
    CHECK(C4 + 2 * 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4))));
    CHECK(E4 + 2 * 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4))));
    CHECK(E4 + 2 * 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4))));
    CHECK(G4 + 2 * 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4))));

    tq.octave = -1;
    CHECK(G3 - 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3))));
    CHECK(G3 - 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4))));
    CHECK(C4 - 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4))));
    CHECK(E4 - 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4))));
    CHECK(E4 - 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4))));
    CHECK(G4 - 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4))));
}

TEST_CASE("tintinabulator: tint UP2")
{
    TintQuantizer tq;

    tq.reset();
    tq.mode = TintQuantizer::MODE_UP2;
    tq.octave = 0;

    tq.chordInputVoltageState[0] = pitchToVoltage(C4);
    tq.chordInputVoltageState[1] = pitchToVoltage(E4);
    tq.chordInputVoltageState[2] = pitchToVoltage(G4);

    tq.setChordFreqs(3);

    // since the reference chord is using 12-TET, make the assertions in terms of pitch "note" rather than
    // frequency.  easier to debug if something goes haywire for the basics.
    CHECK(E4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3))));
    CHECK(G4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4))));
    CHECK(C5 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4))));
    CHECK(C5 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4))));
    CHECK(E5 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4))));
    CHECK(E5 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4))));

    // and should work at any octave:

    CHECK(E4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3 + 12))));
    CHECK(G4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4 + 12))));
    CHECK(C5 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4 + 12))));
    CHECK(C5 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4 + 12))));
    CHECK(E5 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4 + 12))));
    CHECK(E5 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4 + 12))));
}

TEST_CASE("tintinabulator: tint DOWN2")
{
    TintQuantizer tq;

    tq.reset();
    tq.mode = TintQuantizer::MODE_DOWN2;
    tq.octave = 0;

    tq.chordInputVoltageState[0] = pitchToVoltage(C4);
    tq.chordInputVoltageState[1] = pitchToVoltage(E4);
    tq.chordInputVoltageState[2] = pitchToVoltage(G4);

    tq.setChordFreqs(3);

    // since the reference chord is using 12-TET, make the assertions in terms of pitch "note" rather than
    // frequency.  easier to debug if something goes haywire for the basics.
    CHECK(E3 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3))));
    CHECK(E3 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4))));
    CHECK(G3 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4))));
    CHECK(C4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4))));
    CHECK(C4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4))));
    CHECK(E4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4))));

    // and should work at any octave:

    CHECK(E3 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3 + 12))));
    CHECK(E3 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4 + 12))));
    CHECK(G3 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4 + 12))));
    CHECK(C4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4 + 12))));
    CHECK(C4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4 + 12))));
    CHECK(E4 + 12 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4 + 12))));
}

TEST_CASE("tintinabulator: tint UP_DOWN")
{
    TintQuantizer tq;

    tq.reset();
    tq.mode = TintQuantizer::MODE_UP_DOWN;
    tq.octave = 0;

    tq.chordInputVoltageState[0] = pitchToVoltage(C4);
    tq.chordInputVoltageState[1] = pitchToVoltage(E4);
    tq.chordInputVoltageState[2] = pitchToVoltage(G4);

    tq.setChordFreqs(3);

    // since the reference chord is using 12-TET, make the assertions in terms of pitch "note" rather than
    // frequency.  easier to debug if something goes haywire for the basics.
    tq.upDown = false;
    CHECK(G3 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4))));
    tq.upDown = true;
    CHECK(E4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4))));
    tq.upDown = false;
    CHECK(G3 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4))));
}

TEST_CASE("tintinabulator: tint UP2_DOWN2")
{
    TintQuantizer tq;

    tq.reset();
    tq.mode = TintQuantizer::MODE_UP2_DOWN2;
    tq.octave = 0;

    tq.chordInputVoltageState[0] = pitchToVoltage(C4);
    tq.chordInputVoltageState[1] = pitchToVoltage(E4);
    tq.chordInputVoltageState[2] = pitchToVoltage(G4);

    tq.setChordFreqs(3);

    // since the reference chord is using 12-TET, make the assertions in terms of pitch "note" rather than
    // frequency.  easier to debug if something goes haywire for the basics.
    tq.upDown = false;
    CHECK(E3 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4))));
    tq.upDown = true;
    CHECK(G4 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4))));
    tq.upDown = false;
    CHECK(E3 == voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4))));
}

TEST_CASE("tintinabulator: quantization with non-12-TET reference")
{
    TintQuantizer tq;

    tq.reset();
    tq.mode = TintQuantizer::MODE_QUANTIZE;
    tq.octave = 0;

    const float PC4 = C4 + 0.2f;
    const float PE4 = E4 - 0.4f;
    const float PG4 = G4 + 0.6f;

    tq.chordInputVoltageState[0] = microPitchToVoltage(PC4);
    tq.chordInputVoltageState[1] = microPitchToVoltage(PE4);
    tq.chordInputVoltageState[2] = microPitchToVoltage(PG4);

    tq.setChordFreqs(3);

    float epsilon = 0.0001f;

    REQUIRE_THAT(PC4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3))), epsilon));
    REQUIRE_THAT(PC4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4))), epsilon));
    REQUIRE_THAT(PE4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4))), epsilon));
    REQUIRE_THAT(PE4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4))), epsilon));
    REQUIRE_THAT(PG4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4))), epsilon));
    REQUIRE_THAT(PG4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4))), epsilon));

    // and should work at any octave:

    REQUIRE_THAT(PC4 + 12, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3 + 12))), epsilon));
    REQUIRE_THAT(PC4 + 12, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4 + 12))), epsilon));
    REQUIRE_THAT(PE4 + 12, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4 + 12))), epsilon));
    REQUIRE_THAT(PE4 + 12, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4 + 12))), epsilon));
    REQUIRE_THAT(PG4 + 12, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4 + 12))), epsilon));
    REQUIRE_THAT(PG4 + 12, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4 + 12))), epsilon));
}
