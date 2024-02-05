//
// Created by brant on 1/24/24.
//
//#define DEBUG_FB
#include <rmkit.h>
#include <unordered_set>
#include "rempack.h"
#include "../ui/widgets.h"
#include "../ui/debug_widgets.h"
#include "../ui/rempack_widgets.h"
#include "../opkg/opkg.h"

using ListItem = widgets::ListBox::ListItem;
ui::Scene buildHomeScene(int width, int height);

opkg pkg;
widgets::ListBox *filterPanel, *packagePanel;
widgets::PackageInfoPanel *displayBox;

void Rempack::startApp() {
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
//todo: this should be more complex than a string vector
//probably some type of structure that contains a sliver of logic based on its type
//i.e. repoFilterStruct(string includeRepoName) or sectionFilterStruct(string includeSections)
//for now this is simply sections because that's the only filter I've implemented yet
std::unordered_set<std::string> _filters;
shared_ptr<package> _selected;
bool packageFilterDelegate(const shared_ptr<ListItem> &item) {
    auto pk = any_cast<shared_ptr<package>>(item->object);
    if (pk->Repo == "entware")
        return false;
    if (_filters.empty() || _filters.find(pk->Section) != _filters.end())
        return true;
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
    displayBox->undraw();
    displayBox->set_text(opkg::dumpPackage(pk));
    displayBox->mark_redraw();
}
void onPackageDeselect(shared_ptr<ListItem> item) {
    auto pk = any_cast<shared_ptr<package>>(item->object);
    printf("Package deselected: %s\n", pk->Package.c_str());
    displayBox->undraw();
    displayBox->set_text("");
    displayBox->mark_redraw();
}

//1404x1872 - 157x209mm -- 226dpi
ui::Scene buildHomeScene(int width, int height) {
    int padding = 20;
    auto scene = ui::make_scene();

    //vertical stack that takes up the whole screen
    auto layout = new ui::VerticalReflow(padding, padding, width - padding*2, height - padding*2, scene);

    /* Search + menus */
    //short full-width pane containing search and menus
    auto searchPane = new ui::HorizontalReflow(0, 0, layout->w, 80, scene);
    auto filterButton = new widgets::FilterButton(0,0,60,60);
    auto settingButton = new widgets::ConfigButton(padding*2, 0, 60, 60);
    auto searchBox = new widgets::SearchBox(padding, 0, layout->w - 120 - padding*2, 60, widgets::RoundCornerStyle());
    searchPane->pack_start(filterButton);
    searchPane->pack_start(searchBox);
    searchPane->pack_start(settingButton);

    /* Applications */
    //full-width horizontal stack underneath the search pane. give it half the remaining height
    auto applicationPane = new ui::HorizontalReflow(0, 0, layout->w, (layout->h - searchPane->h - padding)/2, scene);
    filterPanel = new widgets::ListBox(0, 0, 200, applicationPane->h, 30);
    std::vector<std::string> sections;
    pkg.InitializeRepositories();
    pkg.LoadSections(&sections);
    std::sort(sections.begin(), sections.end());
    for (const auto &s: sections)
        filterPanel->add(s);

    filterPanel->events.selected += PLS_DELEGATE(onFilterAdded);
    filterPanel->events.deselected += PLS_DELEGATE(onFilterRemoved);

    packagePanel = new widgets::ListBox(padding, 0, layout->w - filterPanel->w - padding, applicationPane->h, 30);
    std::vector<std::string> packages;
    pkg.LoadPackages(&packages);
    std::sort(packages.begin(), packages.end());
    packagePanel->multiSelect = false;
    packagePanel->filterPredicate = packageFilterDelegate;
    for (const auto &[n, pk]: pkg.packages) {
        //todo: properly width-limit this. for now it just clips
        //(no it doesn't, we're just drawing off the edge of the screen)
        string displayName = pk->Package;
        displayName.append(" -- ").append(pk->Description);
        packagePanel->add(displayName, pk);
    }
    packagePanel->events.selected += PLS_DELEGATE(onPackageSelect);
    packagePanel->events.deselected += PLS_DELEGATE(onPackageDeselect);

    displayBox = new widgets::PackageInfoPanel(0,0,applicationPane->w,applicationPane->h, widgets::RoundCornerStyle());

    layout->pack_start(searchPane);
    layout->pack_start(applicationPane);
    applicationPane->pack_start(filterPanel);
    applicationPane->pack_start(packagePanel);
    layout->pack_end(displayBox);

    layout->reflow();

    return scene;
}
