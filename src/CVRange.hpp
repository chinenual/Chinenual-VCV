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
};

enum CVRangeIndex
{
    CV_RANGE_n10_10,
    CV_RANGE_0_10,
    CV_RANGE_n5_,
    CV_RANGE_0_5,
};