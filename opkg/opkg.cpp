//
// Created by brant on 2/2/24.
//

#include "opkg.h"
#include <stdio.h>
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <string>
#include <map>
#include <unordered_set>
#include <cstring>
#include "zlib.h"
#include "utils.h"
#include <signal.h>
#include <iostream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;
using namespace std;

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

const fs::path OPKG_DB{"/opt/var/opkg-lists"};
const fs::path OPKG_LIB{"/opt/lib/opkg"}; //info(dir) lists(dir(empty?)) status(f)

void opkg::LoadSections(std::vector<std::string> *categories, bool skipEntware) {
    std::unordered_set<string> sections;
    for (const auto& [n, pk]: packages) {
        if(skipEntware && pk->Repo == "entware")
            continue;
        if(!pk->Section.empty())
            sections.emplace(pk->Section);
    }
    for (const auto &section: sections) {
        categories->push_back(section);
    }
}

void opkg::LoadPackages(std::vector<std::string> *dest, bool skipEntware) {
    for (const auto& [n, pk]: packages) {
        if(skipEntware && pk->Repo == "entware")
            continue;
        dest->emplace_back(pk->Package);
    }
}

std::vector<std::string> split(const std::string &s, const char delimiter)
{
    std::vector<std::string> splits;
    std::string _split;
    std::istringstream ss(s);
    while (getline(ss, _split, delimiter))
    {
        if(_split[0] == ' ')
            _split.erase(0,1);
        splits.push_back(_split);
    }
    return splits;
}

bool opkg::split_str_and_find(const string& children_str, vector<shared_ptr<package>> &field){
    auto splits = split(children_str, ',');
    if(splits.empty()){
        return false;
    }
    bool err = false;
    for(const auto &s : splits){
        auto it = packages.find(s);
        if(it == packages.cend())
        {
            //dependency may have a version in the string: Failed to resolve dependency tarnish (= 2.6-3) for package oxide-utils
            //strip the version here and try again. This seems to catch everything
            auto dsplit = split(s, ' ');
            if(!dsplit.empty()){
                it = packages.find(dsplit[0]);
                if(it != packages.cend()){
                    field.push_back(it->second);
                    continue;
                }
            }
            err = true;
            //printf("Failed to resolve child %s for package\n", s.c_str());
            continue;
        }
        field.push_back(it->second);
    }
    return !err;
}

//after all packages are parsed, just roll through the list once to link dependent packages
void opkg::link_dependencies(){
    for(const auto &[n, pkg] : packages){
        if(!pkg->_depends_str.empty()) {
            if (!split_str_and_find(pkg->_depends_str, pkg->Depends)) {
                //printf("Problem resolving dependencies for package %s\n", pkg->Package.c_str());
            }
        }
        if(!pkg->_recommends_str.empty()){
            if (!split_str_and_find(pkg->_recommends_str, pkg->Recommends)) {
                //printf("Problem resolving recommendations for package %s\n", pkg->Package.c_str());
            }
        }
        if(!pkg->_conflicts_str.empty()){
            if (!split_str_and_find(pkg->_conflicts_str, pkg->Conflicts)) {
                //printf("Problem resolving conflicts for package %s\n", pkg->Package.c_str());
            }
        }
        if(!pkg->_replaces_str.empty()){
            if (!split_str_and_find(pkg->_replaces_str, pkg->Replaces)) {
                //printf("Problem resolving replacing for package %s\n", pkg->Package.c_str());
            }
        }
        if(!pkg->_provides_str.empty()){
            if (!split_str_and_find(pkg->_provides_str, pkg->Provides)) {
                //printf("Problem resolving provides for package %s\n", pkg->Package.c_str());
            }
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

inline bool try_parse_bool(const char *prefix, const char* line, bool &field){
   string s;
    if(try_parse_str(prefix, line, s)){
        if(strncmp(s.c_str(),"yes",3) == 0) {
            field = true;
            return true;
        }
    }
    return false;
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

//this is mostly copied from opkg's own parser. I don't hate it?
bool opkg::parse_line(shared_ptr<package> &ptr, const char *line, bool update) {
    static bool parsing_desc = false; //this is ugly, but it's what opkg does so whatever I guess
    if (ptr == nullptr)
        return false;
    switch (*line) {
        case 'P': {
            // PACKAGE NAME HANDLING
            if (!update) {
                if (try_parse_str("Package", line, ptr->Package)) {     //we're parsing packages out of multiple disparate lists
                    parsing_desc = false;                               //check if we already have a matching package
                    break;                                              //if we do, reset the package pointer to the extant package
                }                                                       //then we can just keep processing as normal
            }                                                           //this works because Package is always the first line
            else {                                                      //in the entry
                string pn;
                if (try_parse_str("Package", line, pn)) {
                    auto it = packages.find(pn);
                    if (it != packages.end()) {
                        ptr = it->second;
                        parsing_desc = false;
                        break;
                    } else {
                        //TODO: actually handle this error?
                        printf("ERROR! PACKAGE NOT FOUND! %s\n", pn.c_str());
                        break;
                    }
                }
            }
            // /PACKAGE NAME HANDLING
            break;
        }
        case 'A': {
            if (try_parse_str("Architecture", line, ptr->Architecture)) {
                parsing_desc = false;
                break;
            }
            if (try_parse_str("Alternatives", line, ptr->Alternatives)) {
                parsing_desc = false;
                break;
            }
            if (try_parse_bool("AutoInstalled", line, ptr->autoInstalled)) {
                parsing_desc = false;
                break;
            }
            break;
        }
        case 'D': {
            if (try_parse_str("Description", line, ptr->Description)) {
                parsing_desc = true;
                break;
            }
            if (try_parse_str("Depends", line, ptr->_depends_str)) {
                parsing_desc = false;
                break;
            }
            break;
        }
        case 'C': {
            string c;
            if (try_parse_str("CPE-ID", line, c)) {
                parsing_desc = false;
                break;
            }
            break;
        }
        case 'E': {
            if (try_parse_bool("Essential", line, ptr->Essential)) {
                parsing_desc = false;
                break;
            }
            break;
        }
        case 'F': {
            if (try_parse_str("Filename", line, ptr->Filename)) {
                parsing_desc = false;
                break;
            }
            break;
        }
        case 'H': {
            if (try_parse_str("Homepage", line, ptr->Homepage)) {
                parsing_desc = false;
                break;
            }
            break;
        }
        case 'I': {
            if (try_parse_uint("Installed-Size", line, ptr->Size)) {
                parsing_desc = false;
                break;
            }
        }
        case 'L': {
            if (try_parse_str("License", line, ptr->License)) {
                parsing_desc = false;
                break;
            }
            break;
        }
        case 'M': {
            if (try_parse_str("Maintainer", line, ptr->Maintainer)) {
                parsing_desc = false;
                break;
            }
            break;
        }
        case 'R': {
            if (try_parse_str("Replaces", line, ptr->_replaces_str)) {
                parsing_desc = false;
                break;
            }
            string r;
            if (try_parse_str("Require-User", line, r)) {
                parsing_desc = false;
                break;
            }
            break;
        }
        case 'S': {
            if (try_parse_str("Section", line, ptr->Section)) {
                parsing_desc = false;
                break;
            }
            if (try_parse_str("SHA256sum", line, ptr->SHA256sum)) {
                parsing_desc = false;
                break;
            }
            if (try_parse_uint("Size", line, ptr->Size)) {
                parsing_desc = false;
                break;
            }
            break;
        }
        case 'V': {
            if (try_parse_str("Version", line, ptr->Version)) {
                parsing_desc = false;
                break;
            }
            break;
        }
        case ' ': {
            if (parsing_desc) {
                //newlines in descriptions are prefixed with a space, apparently
                ptr->Description.append("\n");
                auto ld = string(line);
                //printf("Appending line to description for package %s\n 1: %s\n 2: %s\n", ptr->Package.c_str(), ptr->Description.c_str(), ld.c_str());
                utils::trim(ld);
                ptr->Description.append(ld);
                break;
            }
        }
        default: {
            auto dln = strlen(line);
            if (dln <= 2)
                return false;
            for (int i = 0; i < dln; i++)
                if (line[i] != ' ' && line[i] != '\n' && line[i] != '\r') {
                    printf("BadChar: %d :: %2x :: %c\n", i, line[0], line[0]);
                    printf("Unhandled tag:%s\n", line);
                    return true;
                }
            printf("UNKN: ");
            for (int j = 0; j < dln; j++) {
                printf("%2x ", line[j]);
            }
            printf("\n");
            return false;
        }
    }
    return true;
}

void opkg::InitializeRepositories() {
    packages.clear();
    int pc = 0;
    char cbuf[4096]{};
    auto pk = make_shared<package>();
    for (const auto &f: fs::directory_iterator(OPKG_DB)) {
        printf("extracting archive %s\n", f.path().c_str());
        auto gzf = gzopen(f.path().c_str(), "rb");
        int count = 0;
        while (gzgets(gzf, cbuf, sizeof(cbuf)) != nullptr) {
            count++;
            if (!parse_line(pk, cbuf, false)) {     //if parse_line returns false, we're done parsing this package
                if (pk->Package.empty())
                    continue;

                pk->Repo = f.path().filename();
                pc++;
                auto mp = packages.emplace(pk->Package, pk);
                if (!mp.second) {
                    printf("emplacement failed for package %d: %s\n", pc, pk->Package.c_str());
                }
                pk = make_shared<package>();
                continue;
            }
        }
        printf("Read %d lines\n", count);
    }
    printf("Parsed %d packages\n", pc);

    //process status and info for installed packages
    auto statuspath = OPKG_LIB;
    statuspath += "/status";
    ifstream statusfile;
    statusfile.open(statuspath, ios::in);
    if(!statusfile.is_open())
        printf("fail opening status file %s\n", statuspath.c_str());

    pc = 0;
    for(string line; getline(statusfile, line);){
        parse_line(pk, line.c_str(), true);     //no need to do any logic here;
        pc++;
    }                                           //parse_line will take care of updating extant packages
    statusfile.close();

    printf("parsed %d status lines\n", pc);

    pc = 0;
    auto infopath = OPKG_LIB;
    infopath += "/info";
    int fc = 0;
    for (const auto &f: fs::directory_iterator(infopath)) {
        if(f.path().extension() == ".control"){
            fc++;
                ifstream cfile;
                cfile.open(f.path(), ios::in);
                if(!cfile.is_open())
                    printf("ERROR! Failed to open control file %s\n", f.path().c_str());
                auto pname = f.path().filename().string().substr(0, f.path().filename().string().find_last_of('.'));
                auto pit = packages.find(pname);
                if(pit == packages.end()){
                    printf("ERROR! Could not match package %s to control file %s\n", pname.c_str(), f.path().c_str());
                    continue;
                }
                pk = pit->second;
            for(string line; getline(cfile, line);){
                pc++;
                parse_line(pk, line.c_str(), false);    //no need to update extant, we know what package this is from the filename
            }
        }
    }
    printf("Processed %d control files containing %d lines\n", fc, pc);

    printf("Processed %d packages\n", packages.size());
    link_dependencies();

}
