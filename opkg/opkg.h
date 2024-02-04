//
// Created by brant on 2/2/24.
//

#pragma once

#include <vector>
#include <string>

class opkg {
public:
    void LoadSections(std::vector<std::string> *categories, bool skipEntware = true);
    void InitializeRepositories();
};


