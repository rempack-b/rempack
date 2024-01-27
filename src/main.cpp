#include <iostream>
#include <rmkit.h>

#include "rempack.h"
int main()
{
    auto fb = framebuffer::get();

    fb->clear_screen();
    auto scene = ui::make_scene();

    ui::MainLoop::set_scene(scene);
    auto t = new ui::Text(0,0,200,50, "hellorld");
    scene->add(t);

    while(true){
        ui::MainLoop::main();
        ui::MainLoop::redraw();
        ui::MainLoop::read_input();
    }

    return 0;
}
