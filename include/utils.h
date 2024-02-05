//
// Created by brant on 1/26/24.
//

#pragma once

#include <cmath>
#include <string>
#include <algorithm>
#include <vector>

namespace utils{
    inline float sigmoid(float t, float a, float b){
        return 1 / (1 + std::exp(a * t + b));
    }

    // trim from start (in place)
    inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }

    // trim from end (in place)
    inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    inline void trim(std::string &s){
        ltrim(s);
        rtrim(s);
    }
}
