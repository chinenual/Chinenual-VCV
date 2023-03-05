#define CATCH_CONFIG_MAIN

#include "CVRange.hpp"
#include "PitchNote.hpp"
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
}

TEST_CASE("v/oct pitch reverse conversion basics")
{
    CHECK((1.f) == pitchToVoltage(72)); // C5
    CHECK((0.f) == pitchToVoltage(60)); // C4
    CHECK((-1.f) == pitchToVoltage(48)); // C3
}

TEST_CASE("micro v/oct pitch conversion basics")
{
    CHECK(voltageToMicroPitch(1.f) == 72.f); // C5
    CHECK(voltageToMicroPitch(0.f) == 60.f); // C4
    CHECK(voltageToMicroPitch(-1.f) == 48.f); // C3
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
