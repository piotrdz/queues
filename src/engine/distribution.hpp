#pragma once

enum class DistributionType
{
    Constant,
    Uniform,
    Normal,
    Exponential
};

struct Distribution
{
    DistributionType type;
    double param1, param2;

    Distribution()
     : type(DistributionType::Exponential)
     , param1(60.0)
     , param2(0.0)
    {}
};
