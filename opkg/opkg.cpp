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
#include <cstring>
#include "zlib.h"
#include "utils.h"
#include <signal.h>
#include <iostream>
#include <sstream>
#include <string>

namespace fs = filesystem;
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

void opkg::update_lists(){
    unordered_set<string> _sections;
    for (const auto& [n, pk]: packages){
        _sections.emplace(pk->Section);
        sections_by_repo[pk->Section].emplace(pk->Repo);
    }
    for(const auto &s: _sections)
        sections.push_back(s);
}

void opkg::LoadSections(vector<string> *categories, vector<string> excludeRepos) {
    unordered_set<string> set;
    for (const auto& [n, pk]: packages) {
        if(!excludeRepos.empty() && find(excludeRepos.begin(), excludeRepos.end(), pk->Repo) == excludeRepos.end())
            continue;
        if(!pk->Section.empty())
            set.emplace(pk->Section);
    }
    for (const auto &section: set) {
        categories->push_back(section);
    }
}

void opkg::LoadPackages(vector<string> *dest, vector<string> excludeRepos) {
    for (const auto& [n, pk]: packages) {
        if(!excludeRepos.empty() && find(excludeRepos.begin(), excludeRepos.end(), pk->Repo) == excludeRepos.end())
            continue;
        dest->emplace_back(pk->Package);
    }
}


string opkg::FormatPackage(const shared_ptr<package> &pk) {
    stringstream ss;

    if(!pk->Package.empty()) ss << "Package" << ": " << pk->Package << endl;
    if(!pk->Description.empty()) ss << "Description" << ": " << pk->Description << endl;
    if(!pk->Homepage.empty()) ss << "Homepage" << ": " << pk->Homepage << endl;
    if(!pk->Version.empty()) ss << "Version" << ": " << pk->Version << endl;
    if(!pk->Maintainer.empty()) ss << "Maintainer" << ": " << pk->Maintainer << endl;
    if(!pk->Architecture.empty()) ss << "Architecture" << ": " << pk->Architecture << endl;
    if(!pk->Repo.empty()) ss << "Repo" << ": " << pk->Repo << endl;

    if(pk->State == package::Installed) {
        time_t intime = pk->installTime;
        auto tm = localtime(&intime);
        ss << "Installed on " << asctime(tm) << endl;

        if (pk->autoInstalled)
            ss << "AutoInstalled: yes" << endl;

        if(pk->Essential)
            ss << "Essential: yes" << endl;

        ss << "Installed size: " << pk->Size << endl;
    }
    else if(pk->State == package::InstallError){
        ss << "Installation error! Placeholder text." << endl;
    }

    if(!pk->Depends.empty()){
        ss << "Depends: ";
        for(const auto &d: pk->Depends)
            ss << d->Package << " ";
        ss << endl;
    }
    if(!pk->Dependents.empty()){
        ss << "Depended by: ";
        for(const auto &d: pk->Dependents)
            ss << d->Package << " ";
        ss << endl;
    }
    return ss.str();
}

//TODO: this needs to move to utilities
vector<string> split(const string &s, const char delimiter)
{
    vector<string> splits;
    string _split;
    istringstream ss(s);
    while (getline(ss, _split, delimiter))
    {
        utils::ltrim(_split);
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
            for(const auto &dpk: pkg->Depends){
                dpk->Dependents.push_back(pkg);
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

void opkg::update_states() {
    for(auto const &[name,pkg] : packages){
        if(pkg->_status_str.empty()){
            pkg->State = package::NotInstalled;
            continue;
        }
        auto status = pkg->_status_str.c_str();

        char sw_str[64], sf_str[64], ss_str[64];
        int r;

        r = sscanf(status, "%63s %63s %63s", sw_str, sf_str, ss_str);
        if(r != 3){
            printf("Error parsing state for %s : %s\n", pkg->Package.c_str(), status);
            continue;
        }

        if(strcmp(sw_str, "install") == 0){
            if(strcmp(ss_str, "installed")== 0){
                pkg->State = package::Installed;
                continue;
            }
        }

        pkg->State = package::InstallError;
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
        field = stoul(f);
        return true;
    }
    catch (exception&) {
        return false;
    }
}

inline bool try_parse_long(const char *prefix, const char *line, long &field) {
    string f;
    if (!try_parse_str(prefix, line, f))
        return false;
    try {
        field = stol(f);
        return true;
    }
    catch (exception&) {
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
                        auto &fpk = it->second;
                        ptr = fpk;
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
            if(try_parse_str("Provides", line, ptr->_provides_str)){
                parsing_desc = false;
                break;
            }
            goto NOT_RECOGNIZED;
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
            if (try_parse_bool("Auto-Installed", line, ptr->autoInstalled)) {
                parsing_desc = false;
                break;
            }
            goto NOT_RECOGNIZED;
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
            goto NOT_RECOGNIZED;
        }
        case 'C': {
            if(try_parse_str("Conflicts", line, ptr->_conflicts_str)){
                parsing_desc = false;
                break;
            }
            string c;
            if (try_parse_str("CPE-ID", line, c)) {
                parsing_desc = false;
                break;
            }
            if (try_parse_str("Conffiles", line, c)) {  //we don't need conffiles, only one package has it
                parsing_desc = false;                   //this will create some errors on the following lines
                break;                                  //but that's manageable
            }
            goto NOT_RECOGNIZED;
        }
        case 'E': {
            if (try_parse_bool("Essential", line, ptr->Essential)) {
                parsing_desc = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'F': {
            if (try_parse_str("Filename", line, ptr->Filename)) {
                parsing_desc = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'H': {
            if (try_parse_str("Homepage", line, ptr->Homepage)) {
                parsing_desc = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'I': {
            if (try_parse_uint("Installed-Size", line, ptr->Size)) {
                parsing_desc = false;
                break;
            }
            if (try_parse_long("Installed-Time", line, ptr->installTime)) {
                parsing_desc = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'L': {
            if (try_parse_str("License", line, ptr->License)) {
                parsing_desc = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'M': {
            if (try_parse_str("Maintainer", line, ptr->Maintainer)) {
                parsing_desc = false;
                break;
            }
            goto NOT_RECOGNIZED;
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
            goto NOT_RECOGNIZED;
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
            if (try_parse_str("Status", line, ptr->_status_str)) {
                parsing_desc = false;
                break;
            }
            string s;
            if (try_parse_str("SourceDateEpoch", line, s)) {
                parsing_desc = false;
                break;
            }
            if (try_parse_str("SourceName", line, s)) {
                parsing_desc = false;
                break;
            }
            if (try_parse_str("Source", line, s)) {
                parsing_desc = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case 'V': {
            if (try_parse_str("Version", line, ptr->Version)) {
                parsing_desc = false;
                break;
            }
            goto NOT_RECOGNIZED;
        }
        case ' ': {
            if (parsing_desc) {
                //newlines in descriptions are prefixed with a space, apparently
                utils::trim(ptr->Description);
                ptr->Description.append("\n");
                auto ld = string(line);
                //printf("Appending line to description for package %s\n 1: %s\n 2: %s\n", ptr->Package.c_str(), ptr->Description.c_str(), ld.c_str());
                utils::trim(ld);
                ptr->Description.append(ld);
                break;
            }
        }
        NOT_RECOGNIZED:
        default: {
            auto dln = strlen(line);
            if (dln <= 2)
                return false;
            for (uint i = 0; i < dln; i++)
                if (line[i] != ' ' && line[i] != '\n' && line[i] != '\r') {
                    printf("BadChar: %d :: %2x :: %c\n", i, line[0], line[0]);
                    printf("Unhandled tag:%s\n", line);
                    return true;
                }
            printf("UNKN: ");
            for (uint j = 0; j < dln; j++) {
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
        repositories.push_back(f.path().filename());
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
        pc++;                                   //parse_line will take care of updating extant packages
    }
    statusfile.close();

    printf("parsed %d status lines\n", pc);

    pc = 0;
    auto infopath = OPKG_LIB;
    infopath += "/info";
    int fc = 0;
    for (const auto &f: fs::directory_iterator(infopath)) {
        if (f.path().extension() == ".control") {
            fc++;
            ifstream cfile;
            cfile.open(f.path(), ios::in);
            if (!cfile.is_open()) {
                printf("ERROR! Failed to open control file %s\n", f.path().c_str());
                continue;
            }
            auto pname = f.path().filename().string().substr(0, f.path().filename().string().find_last_of('.'));
            auto pit = packages.find(pname);
            if (pit == packages.end()) {
                printf("ERROR! Could not match package %s to control file %s\n", pname.c_str(), f.path().c_str());
                continue;
            }
            pk = pit->second;
            for (string line; getline(cfile, line);) {
                pc++;
                parse_line(pk, line.c_str(), false);    //no need to update extant, we know what package this is from the filename
            }
        }
    }
    printf("Processed %d control files containing %d lines\n", fc, pc);
    printf("Processed %d packages\n", packages.size());

    link_dependencies();
    update_lists();
    update_states();
}
