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
}
