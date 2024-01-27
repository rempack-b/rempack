#include <iostream>
//#include "rmkit/artifacts/rm/rmkit.h"

//#define REMARKABLE
#define FONT_EMBED_H "font_embed.h"
#define RMKIT_IMPLEMENTATION

#include "rmkit.h"

int main()
{
    auto fb = framebuffer::get();
    //auto dims = fb->get_display_size();
    //auto width = std::get<0>(dims);
    //auto height = std::get<1>(dims);

    fb->clear_screen();

    auto scene = ui::make_scene();
    ui::MainLoop::set_scene(scene);
    auto t = new ui::Text(0,0,200,50, "hellorld");
    //v->pack_start(t);
    scene->add(t);

    while(true){
        ui::MainLoop::main();
        ui::MainLoop::redraw();
        ui::MainLoop::read_input();
    }

    return 0;
}
