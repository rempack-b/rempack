//
// Created by brant on 1/24/24.
//
//#define DEBUG_FB
#include <rmkit.h>
#include <unordered_set>
#include <utility>
#include "rempack.h"
#include "../ui/widgets.h"
#include "../ui/debug_widgets.h"
#include "../ui/rempack_widgets.h"
#include "../opkg/opkg.h"
#include "../ui/list_box.h"
#include "../include/algorithm/boyer_moore.h"
#include "widget_helpers.h"

using ListItem = widgets::ListBox::ListItem;
namespace boyer = strings::boyer_moore;
ui::Scene buildHomeScene(int width, int height);

opkg pkg;
widgets::ListBox *filterPanel, *packagePanel;
widgets::PackageInfoPanel *displayBox;
widgets::MenuData *_menuData;

void Rempack::startApp() {
    //std::raise(SIGINT);   //firing a sigint here helps synchronize remote gdbserver
    //sleep(10);
    shared_ptr<framebuffer::FB> fb;
    fb = framebuffer::get();
    fb->clear_screen();
    auto scene = buildHomeScene(fb->width, fb->height);
    ui::MainLoop::set_scene(scene);

    //ui::MainLoop::main();
    ui::MainLoop::refresh();
    //ui::MainLoop::redraw();

    while(true){
        ui::MainLoop::main();
        ui::MainLoop::redraw();
        fb->waveform_mode = WAVEFORM_MODE_DU;
        ui::MainLoop::read_input();
    }

}

//this is getting a little unwieldy.
//instead of doing the work in this delegate, we should run the filter and sort
//in the background and trigger a list refresh when finished.
//at that point, it makes the most sense to simply clear the listbox and
//just write our results in directly, then we wouldn't need this delegate setup

//for now though, I'll keep shoehorning it in here

std::unordered_set<std::string> _filters;
shared_ptr<package> _selected;
widgets::FilterOptions _filterOpts;
std::string _searchQuery = "";

bool packageFilterDelegate(const shared_ptr<ListItem> &item) {
    auto pk = any_cast<shared_ptr<package>>(item->object);
    bool visible = false;
    if (!_filters.empty() && _filters.find(pk->Section) == _filters.end())
        return false;
    if(!_filterOpts.Licenses.empty() && !_filterOpts.Licenses.find(pk->License)->second)
        return false;
    if(!_filterOpts.Repos.empty() && !_filterOpts.Repos.find(pk->Repo)->second)
        return false;
    if(!((_filterOpts.Installed && pk->IsInstalled()) ||
    (_filterOpts.NotInstalled && pk->State == package::NotInstalled)))
        return false;   //yes, I feel bad
    // if(_filterOpts.Upgradable && !pk->_is_updatable)
    //    return false;
    if(!_searchQuery.empty()) {
        boyer::pattern pat;
        boyer::init_pattern(_searchQuery, pat);
        std::vector<size_t> indexes = boyer::search(pk->Package, pat);
        if (indexes.empty()) {
            if(!_filterOpts.SearchDescription)
                return false;
            indexes = boyer::search(pk->Description, pat);
            if (indexes.empty())
                return false;
        }
    }
    return true;
}
void searchQueryUpdate(string s){
    _searchQuery = std::move(s);
    packagePanel->mark_redraw();
}
bool sectionFilterDelegate(const shared_ptr<ListItem> &item){
    for(auto &[r,s]: _filterOpts.Repos){
        if(s && CONTAINS(pkg.sections_by_repo[item->label], r))
            return true;
    }
    return false;
}
void onFilterAdded(shared_ptr<ListItem> item) {
    _filters.emplace(item->label);
    packagePanel->mark_redraw();
}
void onFilterRemoved(shared_ptr<ListItem> item) {
    _filters.erase(item->label);
    packagePanel->mark_redraw();
}
void onPackageSelect(shared_ptr<ListItem> item) {
    auto pk = any_cast<shared_ptr<package>>(item->object);
    printf("Package selected: %s\n", pk->Package.c_str());
    _selected = pk;
    displayBox->set_states(pk->IsInstalled());
    displayBox->undraw();
    displayBox->set_text(opkg::FormatPackage(pk));
    displayBox->mark_redraw();
}
void onPackageDeselect(shared_ptr<ListItem> item) {
    auto pk = any_cast<shared_ptr<package>>(item->object);
    printf("Package deselected: %s\n", pk->Package.c_str());
    _selected = nullptr;
    displayBox->set_states(false);
    displayBox->undraw();
    displayBox->set_text("");
    displayBox->mark_redraw();
}
void onFiltersChanged(widgets::FilterOptions &options){
    _filterOpts = options;
    filterPanel->mark_redraw();
    packagePanel->mark_redraw();
}
void markInstall(shared_ptr<package> pkg){
    if(pkg->IsInstalled())
        return;
    if(_menuData->PendingInstall.emplace(pkg->Package).second)
        for(auto const &spk: pkg->Depends)
            markInstall(spk);
}
void onInstallClick(void*){
    //auto str = pkg.formatDependencyTree(_selected, false);
    //cout << str;
    //auto m = new widgets::ModalOverlay(20,20,1200,1400,{widgets::ModalOverlay::ModalButton::OK}, str);
    auto m = new widget_helpers::InstallDialog(500,500,600,800,vector<shared_ptr<package>>{_selected});
    //auto m = new ui::InfoDialog(50,50,200,200);
    //m->set_title("installing your mom");
    m->show();
    _menuData->PendingInstall.emplace(_selected->Package);
}
void onUninstallClick(void*){
    _menuData->PendingRemove.emplace(_selected->Package);
}
void onDownloadClick(void*){

}
void onPreviewClick(void*){

}

//1404x1872 - 157x209mm -- 226dpi
ui::Scene buildHomeScene(int width, int height) {
    int padding = 20;
    auto scene = ui::make_scene();

    //vertical stack that takes up the whole screen
    auto layout = new ui::VerticalReflow(padding, padding, width - padding*2, height - padding*2, scene);

    pkg.InitializeRepositories();
    /* Search + menus */
    //short full-width pane containing search and menus
    auto searchPane = new ui::HorizontalReflow(0, 0, layout->w, 80, scene);

    _filterOpts ={
            .Installed = true,
            .Upgradable = true,
            .NotInstalled = true,
    };
    for(auto &r : pkg.repositories){
        _filterOpts.Repos.emplace(r, r != "entware");   //hide entware by default, there's so many openwrt packages it drowns out toltec
    }
    auto filterButton = new widgets::FilterButton(0,0,60,60, _filterOpts);
    filterButton->events.updated += onFiltersChanged;
    _menuData = new widgets::MenuData;
    auto settingButton = new widgets::ConfigButton(padding*2, 0, 60, 60, _menuData);
    auto searchBox = new widgets::SearchBox(padding, 0, layout->w - 120 - padding*2, 60, widgets::RoundCornerStyle());
    searchBox->events.done += PLS_DELEGATE(searchQueryUpdate);
    searchPane->pack_start(filterButton);
    searchPane->pack_start(searchBox);
    searchPane->pack_start(settingButton);

    /* Applications */
    //full-width horizontal stack underneath the search pane. give it half the remaining height
    auto applicationPane = new ui::HorizontalReflow(0, 0, layout->w, (layout->h - searchPane->h - padding)/2, scene);
    filterPanel = new widgets::ListBox(0, 0, 300, applicationPane->h, 30);
    std::vector<std::string> sections;
    pkg.LoadSections(&sections);
    std::sort(sections.begin(), sections.end());
    for (const auto &s: sections)
        filterPanel->add(s);

    filterPanel->filterPredicate = sectionFilterDelegate;
    filterPanel->events.selected += PLS_DELEGATE(onFilterAdded);
    filterPanel->events.deselected += PLS_DELEGATE(onFilterRemoved);

    packagePanel = new widgets::ListBox(padding, 0, layout->w - filterPanel->w - padding, applicationPane->h, 30);
    std::vector<std::string> packages;
    pkg.LoadPackages(&packages);
    std::sort(packages.begin(), packages.end());
    packagePanel->multiSelect = false;
    packagePanel->filterPredicate = packageFilterDelegate;
    for (const auto &[n, pk]: pkg.packages) {
        //ListBox will trim strings internally depending on render width
        string displayName = pk->Package;
        displayName.append(" -- ").append(pk->Description);
        displayName.erase(std::remove(displayName.begin(), displayName.end(), '\n'), displayName.end());
        packagePanel->add(displayName, pk);
    }
    packagePanel->events.selected += PLS_DELEGATE(onPackageSelect);
    packagePanel->events.deselected += PLS_DELEGATE(onPackageDeselect);

    displayBox = new widgets::PackageInfoPanel(0,0,applicationPane->w,applicationPane->h, widgets::RoundCornerStyle());

    displayBox->events.install += PLS_DELEGATE(onInstallClick);
    displayBox->events.uninstall += PLS_DELEGATE(onUninstallClick);
    displayBox->events.download += PLS_DELEGATE(onDownloadClick);
    displayBox->events.preview += PLS_DELEGATE(onPreviewClick);

    layout->pack_start(searchPane);
    layout->pack_start(applicationPane);
    applicationPane->pack_start(filterPanel);
    applicationPane->pack_start(packagePanel);
    layout->pack_end(displayBox);

    layout->reflow();

    return scene;
}
