//
// Created by brant on 2/25/24.
//
#pragma once

namespace semver{
    inline void Parse(int result[3], const std::string& input)
    {
        std::istringstream parser(input);
        parser >> result[0];
        for(int idx = 1; idx < 3; idx++)
        {
            parser.get(); //Skip period
            parser >> result[idx];
        }
    }

    inline bool LessThanVersion(const std::string& a,const std::string& b)
    {
        int parsedA[3], parsedB[3];
        Parse(parsedA, a);
        Parse(parsedB, b);
        return std::lexicographical_compare(parsedA, parsedA + 3, parsedB, parsedB + 3);
    }
}