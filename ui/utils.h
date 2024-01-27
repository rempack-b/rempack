//
// Created by brant on 1/26/24.
//

#pragma once

#include <cmath>
namespace utils{
    inline float sigmoid(float t, float a = -12, float b = 6){
        return 1 / (1 + std::exp(a * t + b));
    }
}
