//
// Created by brant on 1/26/24.
//

#pragma once

#include <cmath>
#include <string>
#include <algorithm>
#include <vector>

namespace utils{
    inline float sigmoid(float t, float a = -12, float b = 6){
        return 1 / (1 + std::exp(a * t + b));
    }

    std::vector<std::string> split(const std::string &s, const char delimiter)
    {
        std::vector<std::string> splits;
        std::string split;
        std::istringstream ss(s);
        while (getline(ss, split, delimiter))
        {
            if(split[0] == ' ')
                split.erase(0,1);
            splits.push_back(split);
        }
        return splits;
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
