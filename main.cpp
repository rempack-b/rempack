#include <iostream>
#include "rmkit/artifacts/rm/rmkit.h"

int main()
{
    auto fb = framebuffer::get();
    auto dims = fb->get_display_size();
    auto width = std::get<0>(dims);
    auto height = std::get<1>(dims);

    fb->clear_screen();

    auto scene = ui::make_scene();
    ui::MainLoop::set_scene(scene);
    auto v = new ui::VerticalLayout(0,0,width,height,scene);
    auto t = new ui::Text(0,0,100,50, "hellorld");
    v->pack_start(t);
    //scene->add(v);

    while(true){
        ui::MainLoop::main();
        ui::MainLoop::redraw();
        ui::MainLoop::read_input();
    }

    return 0;
}
