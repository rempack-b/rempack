//
// Created by brant on 1/24/24.
//
//#define DEBUG_FB
#include <rmkit.h>
#include "rempack.h"
#include "../assets/icons/icons_embed.h"
#include "../ui/widgets.h"
#include "../ui/utils.h"
#define DEBUG_DRAW

ui::Scene buildHomeScene(int width, int height);

void Rempack::startApp() {
    shared_ptr<framebuffer::FB> fb;
    fb = framebuffer::get();
    fb->clear_screen();

    //auto scene = ui::make_scene();
    auto scene = buildHomeScene(fb->width, fb->height);
    ui::MainLoop::set_scene(scene);
    //auto t = new ui::Text(0,0,200,50, "hellorld");
    //scene->add(t);

    //ui::MainLoop::main();
    ui::MainLoop::refresh();
    //ui::MainLoop::redraw();

    while(true){
        ui::MainLoop::main();
        //fb->waveform_mode = WAVEFORM_MODE_GC16;
        ui::MainLoop::redraw();
        fb->waveform_mode = WAVEFORM_MODE_DU;
        ui::MainLoop::read_input();
    }

}

//1404x1872 - 157x209mm -- 226dpi
ui::Scene buildHomeScene(int width, int height) {
    auto scene = ui::make_scene();

    auto layout = new ui::VerticalReflow(20, 20, width, height, scene);
    auto searchPane = new ui::HorizontalReflow(0, 0, width, 100, scene);
    layout->pack_start(searchPane);

    auto settingButton = new widgets::ConfigButton(0,0,60,60);
    searchPane->pack_start(settingButton);
    auto searchBox = new widgets::SearchBox(0,0,1000,60, widgets::RoundCornerStyle());
    searchPane->pack_center(searchBox);

    auto applicationPane = new ui::HorizontalReflow(0,0,width - 40, height - 400, scene);
    auto groupPane = new ui::VerticalReflow(0,0,500,800,scene);
    applicationPane->pack_start(groupPane);
    auto editor = new widgets::RoundCornerEditor(300,300,800,800,searchBox);
    applicationPane->pack_start(editor);

    return scene;
}
