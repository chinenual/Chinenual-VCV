#define CATCH_CONFIG_MAIN

#include "CVRange.hpp"
#undef WARN

#include "catch.hpp"

using namespace Chinenual;
using namespace MIDIRecorder;

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
