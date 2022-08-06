#define CATCH_CONFIG_MAIN

#include "CVRange.hpp"
#undef WARN

#include "catch.hpp"

using namespace Chinenual;
using namespace MIDIRecorder;

TEST_CASE("enums select correct range")
{
    REQUIRE(CVRanges[CV_RANGE_n10_10].low == -10.0f);
    REQUIRE(CVRanges[CV_RANGE_n10_10].high == 10.0f);
}

TEST_CASE("7bit conversions low/high")
{
    for (int i = CV_RANGE_0_10; i <= CV_RANGE_0_3; i++) {
        CVRange r = CVRanges[i];
        REQUIRE(r.to7bit(r.low) == 0);
        REQUIRE(r.to7bit(r.high) == 127);
    }
}
TEST_CASE("14bit conversions low/high")
{
    for (int i = CV_RANGE_0_10; i <= CV_RANGE_0_3; i++) {
        CVRange r = CVRanges[i];
        REQUIRE(r.to14bit(r.low) == 0);
        REQUIRE(r.to14bit(r.high) == 16383);
    }
}
TEST_CASE("14bit conversions msb/lsb")
{
    for (int i = CV_RANGE_0_10; i <= CV_RANGE_0_3; i++) {
        CVRange r = CVRanges[i];
        auto v = r.to14bit(r.low);
        int msb, lsb;
        r.split14bit(v, msb, lsb);
        REQUIRE(lsb == 0);
        REQUIRE(msb == 0);

        v = r.to14bit(r.high);
        r.split14bit(v, msb, lsb);
        REQUIRE(lsb == 127);
        REQUIRE(msb == 127);
    }
}
