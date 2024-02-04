//
// Created by brant on 2/2/24.
//

#include "opkg.h"
#include <stdio.h>
#include <filesystem>
#include <vector>
#include <fstream>
#include <cstdarg>
#include <string>
#include <map>
#include <unordered_set>
#include <cstring>
#include "zlib.h"
#include "utils.h"
#include <signal.h>

namespace fs = std::filesystem;
using namespace std;
struct package{
  string Package;
  string Description;
  string Homepage;
  string Version;
  string Section;
  string Maintainer;
  string License;
  string Architecture;
  string Filename;
  string SHA256sum;
  uint Size;
  string Repo;
  string _depends_str;
  vector<package*> Depends;
};
/*
 * Package: toltec-base
 * Description: Metapackage defining the base set of packages in a Toltec install
 * Homepage: https://toltec-dev.org/
 * Version: 1.2-2
 * Section: utils
 * Maintainer: Eeems <eeems@eeems.email>
 * License: MIT
 * Architecture: rm2
 * Depends: toltec-completion, toltec-bootstrap, rm2-suspend-fix
 * Filename: toltec-base_1.2-2_rm2.ipk
 * SHA256sum: f5799454493c88b3018732ec16b5585a81ad8ec63deb1806b77ca34eea2080e8
 * Size: 2122
 */

map<string, package*> packages;
const fs::path OPKG_DB{"/opt/var/opkg-lists"};

void opkg::LoadSections(std::vector<std::string> *categories, bool skipEntware) {
    std::unordered_set<string> sections;
    for (const auto& [n, pk]: packages) {
        if(skipEntware && pk->Repo == "entware")
            continue;
        sections.emplace(pk->Section);
    }
    for (const auto &section: sections) {
        categories->push_back(section);
    }
}

//after all packages are parsed, just roll through the list once to link dependent packages
void link_dependencies(){
    for(const auto &[n, pkg] : packages){
        if(pkg->_depends_str.empty())
            continue;
        auto splits = utils::split(pkg->_depends_str, ',');
        if(splits.empty()){
            printf("Failed to parse dependency list for package %s. Dependencies: %s\n", pkg->Package.c_str(), pkg->_depends_str.c_str());
        }
        for(const auto &s : splits){
            auto it = packages.find(s);
            if(it == packages.cend())
            {
                //dependency may have a version in the string: Failed to resolve dependency tarnish (= 2.6-3) for package oxide-utils
                //strip the version here and try again. This seems to catch everything
                auto dsplit = utils::split(s, ' ');
                if(!dsplit.empty()){
                    it = packages.find(dsplit[0]);
                    if(it != packages.cend()){
                        pkg->Depends.push_back(it->second);
                        continue;
                    }
                }
                printf("Failed to resolve dependency %s for package %s\n", s.c_str(), pkg->Package.c_str());
                continue;
            }
            pkg->Depends.push_back(it->second);
        }
    }
}

inline bool try_parse_str(const char* prefix, const char *line, string &field){
    if (strncmp(line, prefix, strlen(prefix)) != 0)
        return false;
    auto f = string(line + strlen(prefix) + 1);
    utils::trim(f);
    field = f;
    return true;
}

inline bool try_parse_uint(const char *prefix, const char *line, uint &field) {
    string f;
    if (!try_parse_str(prefix, line, f))
        return false;
    try {
        field = std::stoul(f);
        return true;
    }
    catch (exception) {
        return false;
    }
}

bool parse_line(package *ptr, const char *line) {
    if (ptr == nullptr)
        return false;
    switch (*line) {
        case 'A': {
            if(try_parse_str("Architecture", line, ptr->Architecture))
                break;
        }
        case 'D':{
            if(try_parse_str("Description", line, ptr->Description))
                break;
            if(try_parse_str("Depends", line, ptr->_depends_str))
                break;
        }
        case 'F':{
            if(try_parse_str("Filename", line, ptr->Filename))
                break;
        }
        case 'H':{
            if(try_parse_str("Homepage", line, ptr->Homepage))
                break;
        }
        case 'L':{
            if(try_parse_str("License", line, ptr->License))
                break;
        }
        case 'M':{
            if(try_parse_str("Maintainer", line, ptr->Maintainer))
                break;
        }
        case 'P':{
            if(try_parse_str("Package", line, ptr->Package))
                break;
        }
        case 'S':{
            if(try_parse_str("Section", line, ptr->Section))
                break;
            if(try_parse_str("SHA256Sum", line, ptr->SHA256sum))
                break;
            if(try_parse_uint("Size", line, ptr->Size))
                break;
        }
        case 'V':{
            if(try_parse_str("Version", line, ptr->Version))
                break;
        }
        case ' ':{
            //newlines in descriptions are prefixed with a space, apparently
            ptr->Description.append("\n");
            ptr->Description.append(line);
        }
        default:{
            auto dln = strlen(line);
            if(dln <= 1)
                return false;
            for(int i = 0; i < dln; i++)
                if(line[i] != ' ' && line[i] != '\n' && line[i] != '\r')
                    return true;
            return false;
        }
    }
    return true;
}

void opkg::InitializeRepositories() {
    packages.clear();
    int pc = 0;
    for(const auto &f : fs::directory_iterator(OPKG_DB)){
        printf("extracting archive %s\n", f.path().c_str());
        auto gzf = gzopen(f.path().c_str(), "rb");
        char cbuf[4096]{};
        auto *pk = new package;
        int count = 0;
        while(gzgets(gzf, cbuf, sizeof(cbuf)) != nullptr){
            count++;
            if(!parse_line(pk, cbuf)){
                if(pk->Package.empty())
                    continue;

                pk->Repo = f.path().filename();
                pc++;
                auto mp = packages.emplace(pk->Package, pk);
                if(!mp.second) {
                    printf("emplacement failed for package %d: %s\n", pc, pk->Package.c_str());

                }
                pk = new package;
                continue;
            }
        }
        printf("Read %d lines\n", count);
        delete pk;
    }
    printf("Parsed %d packages\n", pc);
    printf("Processed %d packages\n", packages.size());
    link_dependencies();
}
