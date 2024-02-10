//
// Created by brant on 2/2/24.
//

#pragma once

#include <stdio.h>
#include <memory>
#include <vector>
#include <string>
#include <map>
using namespace std;
struct package{
    std::string Package;
    std::string Description;
    std::string Homepage;
    std::string Version;
    std::string Section;
    std::string Maintainer;
    std::string License;
    std::string Architecture;
    std::string Filename;
    std::string SHA256sum;
    uint Size;
    std::string Repo;
    std::string _depends_str;
    std::vector<std::shared_ptr<package>> Depends;
    std::string _recommends_str;
    std::vector<std::shared_ptr<package>> Recommends;
    bool autoInstalled;
    uint32_t installTime;
    std::string Status;
    std::string Alternatives;
    std::string _conflicts_str;
    std::vector<std::shared_ptr<package>> Conflicts;
    std::string _replaces_str;
    std::vector<std::shared_ptr<package>> Replaces;
    std::string Conffiles;
    std::string _provides_str;
    vector<shared_ptr<package>> Provides;
    bool Essential;
};
class opkg {
public:
    void LoadSections(std::vector<std::string> *categories, bool skipEntware = true);
    void LoadPackages(std::vector<std::string> *packages, bool skipEntware = true);
    void InitializeRepositories();
    void link_dependencies();
    std::map<std::string, std::shared_ptr<package>> packages;
    static std::string dumpPackage(const std::shared_ptr<package>& pk0){
        char buf[32768];
        auto len = snprintf(buf, 32768,
               "\tPackage: %s\n"
               "\tDescription: %s\n"
               "\tHomepage: %s\n"
               "\tVersion: %s\n"
               "\tSection: %s\n"
               "\tMaintainer: %s\n"
               "\tLicense: %s\n"
               "\tArchitecture: %s\n"
               "\tFilename: %s\n"
               "\tSHASum: %s\n"
               "\tSize: %d\n"
               "\tRepo: %s\n"
               "\tDepends: %s\n\n",
               pk0->Package.c_str(), pk0->Description.c_str(), pk0->Homepage.c_str(), pk0->Version.c_str(),
               pk0->Section.c_str(), pk0->Maintainer.c_str(), pk0->License.c_str(), pk0->Architecture.c_str(),
               pk0->Filename.c_str(), pk0->SHA256sum.c_str(), pk0->Size, pk0->Repo.c_str(), pk0->_depends_str.c_str());
        auto s =  std::string(buf, len);
        return s;
    }
private:
    bool parse_line(shared_ptr<package> &ptr, const char *line, bool update);
    bool split_str_and_find(const string& children_str, vector<shared_ptr<package>> &field);
};


