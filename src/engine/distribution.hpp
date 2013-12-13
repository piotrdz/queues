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
     : type(DistributionType::Constant)
     , param1(0.0)
     , param2(0.0)
    {}
};
