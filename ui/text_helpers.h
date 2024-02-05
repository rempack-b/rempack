//
// Created by brant on 2/5/24.
//

#pragma once

#include <rmkit.h>

namespace utils{
    inline std::tuple<int,int> measure_string(std::string s, short fontSize){
        auto v = stbtext::get_text_size(s,fontSize);
        return make_tuple(v.w, v.h);
    }

    inline std::string clip_string(std::string s, int w, int h, short fontSize){
        std::stringstream ss;
        auto line_height = stbtext::get_line_height(fontSize);
        auto lines = split_lines(s);
        int x = 0;
        int y = 0;
        for(const auto &line : lines) {
            y += line_height;
            auto tokens = split(line, ' ');
            for (std::string &tk: tokens) {
                auto token = tk;
                //TODO: do this less bad
                token.append(" ");
                auto v = stbtext::get_text_size(token, fontSize);
                if(x + v.w < w)
                    x += v.w;
                else{
                    if(y + line_height > h)
                        return ss.str();
                    y += line_height;
                    x = v.w;
                }
                ss << token;
            }
            ss << std::endl;
        }
        return s;
    }
}