#define CATCH_CONFIG_MAIN

#include "TintQuantizer.hpp"
#undef WARN

#include "catch.hpp"

using namespace Chinenual;
using namespace Tint;
using namespace Catch;

// -----------------------------------------------

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

    tq.prepare(3);

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

    tq.prepare(3);

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

    tq.prepare(3);

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

    tq.prepare(3);

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

    tq.prepare(3);

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

    tq.prepare(3);

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

    tq.prepare(3);

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

    tq.prepare(3);

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

    // based on the 8-mar bug report, set one of the reference notes to far outside the octave.  Should
    // normalize and work as if the chord was all within an octave:

    tq.chordInputVoltageState[0] = microPitchToVoltage(PC4);
    tq.chordInputVoltageState[1] = microPitchToVoltage(PE4);
    tq.chordInputVoltageState[2] = microPitchToVoltage(PG4 + 5 * 12);

    tq.prepare(3);

    REQUIRE_THAT(PC4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3))), epsilon));
    REQUIRE_THAT(PC4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4))), epsilon));
    REQUIRE_THAT(PE4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4))), epsilon));
    REQUIRE_THAT(PE4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4))), epsilon));
    REQUIRE_THAT(PG4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4))), epsilon));
    REQUIRE_THAT(PG4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4))), epsilon));
}

TEST_CASE("tintinabulator: quantization with non-12-TET reference and non-12-TET melody")
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

    tq.prepare(3);

    float epsilon = 0.0001f;

    REQUIRE_THAT(PC4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3 + 0.2f))), epsilon));
    REQUIRE_THAT(PC4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4 + 0.2f))), epsilon));
    REQUIRE_THAT(PE4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4 + 0.2f))), epsilon));
    REQUIRE_THAT(PE4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4 + 0.2f))), epsilon));
    REQUIRE_THAT(PG4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4 + 0.2f))), epsilon));
    REQUIRE_THAT(PG4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4 + 0.2f))), epsilon));

    // and should work at any octave:

    REQUIRE_THAT(PC4 + 12, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(B3 + 0.2f + 12))), epsilon));
    REQUIRE_THAT(PC4 + 12, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(C4 + 0.2f + 12))), epsilon));
    REQUIRE_THAT(PE4 + 12, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(E4 + 0.2f + 12))), epsilon));
    REQUIRE_THAT(PE4 + 12, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(F4 + 0.2f + 12))), epsilon));
    REQUIRE_THAT(PG4 + 12, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(G4 + 0.2f + 12))), epsilon));
    REQUIRE_THAT(PG4 + 12, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(A4 + 0.2f + 12))), epsilon));
}

TEST_CASE("tintinabulator: quantization with non-12-TET melody - check distance behavior")
{
    TintQuantizer tq;

    tq.reset();
    tq.mode = TintQuantizer::MODE_QUANTIZE;
    tq.octave = 0;

    tq.chordInputVoltageState[0] = microPitchToVoltage(C4);
    tq.chordInputVoltageState[1] = microPitchToVoltage(E4);
    tq.chordInputVoltageState[2] = microPitchToVoltage(G4);

    tq.prepare(3);

    float epsilon = 0.0001f;

    REQUIRE_THAT(C4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(D4 - 0.1f))), epsilon));
    REQUIRE_THAT(C4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(D4 - 0.01f))), epsilon));
    // what happens at "equidistant" is arbitrary - the code prefers snapping up, so test it:
    REQUIRE_THAT(E4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(D4))), epsilon));
    REQUIRE_THAT(E4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(D4 + 0.01f))), epsilon));
    REQUIRE_THAT(E4, WithinAbs(voltageToMicroPitch(tq.tintinnabulate(pitchToVoltage(D4 + 0.1f))), epsilon));
}
