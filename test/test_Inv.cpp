#define CATCH_CONFIG_MAIN

#include "PitchInverter.hpp"
#undef WARN

#include "catch.hpp"

using namespace Chinenual;
using namespace Inv;
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

TEST_CASE("inverter: chromatic")
{
    PitchInverter inv;

    inv.reset();
    inv.mode = PitchInverter::MODE_CHROMATIC;

    float epsilon = 0.0001f;

    REQUIRE_THAT(C4, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4))), epsilon));
    REQUIRE_THAT(C4 - 1, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 + 1))), epsilon));
    REQUIRE_THAT(C4 - 2, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 + 2))), epsilon));
    REQUIRE_THAT(C4 - 3, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 + 3))), epsilon));
    REQUIRE_THAT(C4 - 4, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 + 4))), epsilon));
    REQUIRE_THAT(C4 - 5, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 + 5))), epsilon));
    REQUIRE_THAT(C4 - 6, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 + 6))), epsilon));
    REQUIRE_THAT(C4 - 7, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 + 7))), epsilon));
    REQUIRE_THAT(C4 - 8, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 + 8))), epsilon));
    REQUIRE_THAT(C4 - 9, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 + 9))), epsilon));
    REQUIRE_THAT(C4 - 10, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 + 10))), epsilon));
    REQUIRE_THAT(C4 - 11, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 + 11))), epsilon));
    REQUIRE_THAT(C4 + 1, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 - 1))), epsilon));
    REQUIRE_THAT(C4 + 2, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 - 2))), epsilon));
    REQUIRE_THAT(C4 + 3, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 - 3))), epsilon));
    REQUIRE_THAT(C4 + 4, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 - 4))), epsilon));
    REQUIRE_THAT(C4 + 5, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 - 5))), epsilon));
    REQUIRE_THAT(C4 + 6, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 - 6))), epsilon));
    REQUIRE_THAT(C4 + 7, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 - 7))), epsilon));
    REQUIRE_THAT(C4 + 8, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 - 8))), epsilon));
    REQUIRE_THAT(C4 + 9, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 - 9))), epsilon));
    REQUIRE_THAT(C4 + 10, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 - 10))), epsilon));
    REQUIRE_THAT(C4 + 11, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4), pitchToVoltage(C4 - 11))), epsilon));

    REQUIRE_THAT(C4 + 24, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4))), epsilon));
    REQUIRE_THAT(C4 + 24 - 1, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 + 1))), epsilon));
    REQUIRE_THAT(C4 + 24 - 2, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 + 2))), epsilon));
    REQUIRE_THAT(C4 + 24 - 3, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 + 3))), epsilon));
    REQUIRE_THAT(C4 + 24 - 4, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 + 4))), epsilon));
    REQUIRE_THAT(C4 + 24 - 5, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 + 5))), epsilon));
    REQUIRE_THAT(C4 + 24 - 6, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 + 6))), epsilon));
    REQUIRE_THAT(C4 + 24 - 7, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 + 7))), epsilon));
    REQUIRE_THAT(C4 + 24 - 8, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 + 8))), epsilon));
    REQUIRE_THAT(C4 + 24 - 9, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 + 9))), epsilon));
    REQUIRE_THAT(C4 + 24 - 10, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 + 10))), epsilon));
    REQUIRE_THAT(C4 + 24 - 11, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 + 11))), epsilon));
    REQUIRE_THAT(C4 + 24 + 1, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 - 1))), epsilon));
    REQUIRE_THAT(C4 + 24 + 2, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 - 2))), epsilon));
    REQUIRE_THAT(C4 + 24 + 3, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 - 3))), epsilon));
    REQUIRE_THAT(C4 + 24 + 4, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 - 4))), epsilon));
    REQUIRE_THAT(C4 + 24 + 5, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 - 5))), epsilon));
    REQUIRE_THAT(C4 + 24 + 6, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 - 6))), epsilon));
    REQUIRE_THAT(C4 + 24 + 7, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 - 7))), epsilon));
    REQUIRE_THAT(C4 + 24 + 8, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 - 8))), epsilon));
    REQUIRE_THAT(C4 + 24 + 9, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 - 9))), epsilon));
    REQUIRE_THAT(C4 + 24 + 10, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 - 10))), epsilon));
    REQUIRE_THAT(C4 + 24 + 11, WithinAbs(voltageToMicroPitch(inv.invert(pitchToVoltage(C4 + 12), pitchToVoltage(C4 - 11))), epsilon));
}
