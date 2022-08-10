#pragma once
#include "plugin.hpp"

namespace Chinenual {
namespace MIDIRecorder {

    struct CVRange {
        float low;
        float high;

        CVRange(const float low, const float high)
            : low(low)
            , high(high)
        {
        }

        int to7bit(float voltage)
        {
            return clamp((int)std::round(((voltage - low) / high) * 127), 0, 127);
        }

        int to14bit(float voltage)
        {
            return clamp((int)std::round(((voltage - low) / high) * 16383), 0, 16383);
        }

        void split14bit(const int val, int& msb, int& lsb)
        {
            msb = (val >> 7) & 0x7f;
            lsb = val & 0x7f;
        }
    };

    static CVRange CVRanges[] = {
        CVRange(-10.f, 10.f),
        CVRange(0.f, 10.f),
        CVRange(-5.f, 5.f),
        CVRange(0.f, 5.f),
        CVRange(-3.f, 3.f),
        CVRange(0.f, 3.f),
        CVRange(-1.f, 1.f),
        CVRange(0.f, 1.f),
    };

    static std::vector<std::string> CVRangeNames = {
        "-10 to 10",
        "  0 to 10",
        " -5 to  5",
        "  0 to  5",
        " -3 to  3",
        "  0 to  3",
        " -1 to  1",
        "  0 to  1",
    };

    enum CVRangeIndex {
        CV_RANGE_n10_10,
        CV_RANGE_0_10,
        CV_RANGE_n5_5,
        CV_RANGE_0_5,
        CV_RANGE_n3_3,
        CV_RANGE_0_3,
        CV_RANGE_n1_1,
        CV_RANGE_0_1,
    };

} // namespace MIDIRecorder
} // namespace Chinenual
