#pragma once
#include "plugin.hpp"

struct CVRange
{
    float low;
    float high;

    CVRange(const float low, const float high) : low(low), high(high) {}

    int to7bit(float voltage)
    {
        return (int)std::round(((voltage - low) / high) * 127);
    }

    int to14bit(float voltage)
    {
        return (int)std::round(((voltage - low) / high) * 16384);
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

enum CVRangeIndex
{
    CV_RANGE_n10_10,
    CV_RANGE_0_10,
    CV_RANGE_n5_5,
    CV_RANGE_0_5,
    CV_RANGE_n3_3,
    CV_RANGE_0_3,
    CV_RANGE_n1_1,
    CV_RANGE_0_1,
};