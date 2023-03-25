#define CATCH_CONFIG_MAIN

#include "PitchNote.hpp"
#undef WARN

#include "catch.hpp"

using namespace Chinenual;
using namespace Catch;

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
