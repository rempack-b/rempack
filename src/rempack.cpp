//
// Created by brant on 1/24/24.
//

#include <rmkit.h>
#include "rempack.h"
#include "../assets/icons/icons_embed.h"

//basically a reimplementation of ui::Button with a clickable image instead of text
class ImageButton :public ui::Widget{
public:
    ImageButton(int x, int y, int w, int h, icons::Icon icon): Widget(x,y,w,h){
        pixmap = make_shared<ui::Pixmap>(x, y, w, h, icon);
    }
    void on_mouse_enter(input::SynMotionEvent &ev) override {dirty = 1;}
    void on_mouse_leave(input::SynMotionEvent &ev) override {dirty = 1;}
    void on_mouse_down(input::SynMotionEvent &ev) override {dirty = 1;}
    void on_mouse_up(input::SynMotionEvent &ev) override {dirty = 1;}

    void render() override{
        this->undraw();
        pixmap->x = x;
        pixmap->y = y;
        pixmap->render();

        auto color = WHITE;
        bool fill = false;
        if(mouse_inside) {
            color = GRAY;
            fill = true;
        }
        if(mouse_down) {
            color = BLACK;
            fill = true;
        }
        fb->draw_rect(x, y, w, h, color, fill);
    }
private:
    shared_ptr<ui::Pixmap> pixmap;
};

class SearchBox: ui::TextInput{
public:
    SearchBox(int x, int y, int w, int h, icons::Icon icon, const string text = ""): TextInput(x, y, w, h, text){
        pixmap = make_shared<ui::Pixmap>(x, y, w, h, icon);
    }
private:
    shared_ptr<ui::Pixmap> pixmap;
};

class ConfigButton : public ImageButton{
public:
    ConfigButton(int x, int y, int w, int h):ImageButton(x,y,w,h,ICON(assets::png_menu_png)){

    }
};


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

    while(true){
        ui::MainLoop::main();
        ui::MainLoop::redraw();
        ui::MainLoop::read_input();
    }

}

//1404x1872 - 157x209mm -- 226dpi
ui::Scene buildHomeScene(int width, int height) {
    auto scene = ui::make_scene();

    auto layout = new ui::VerticalLayout(20, 20, width, height, scene);
    auto searchPane = new ui::HorizontalLayout(0, 0, width, 100, scene);
    layout->pack_start(searchPane);

    auto settingButton = new ConfigButton(0,0,60,60);
    searchPane->pack_start(settingButton);

    return scene;
}
