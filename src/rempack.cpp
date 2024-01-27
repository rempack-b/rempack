//
// Created by brant on 1/24/24.
//

#include "rempack.h"


ui::Scene rempack::buildHomeScene(int width, int height) {
    auto scene = ui::make_scene();

    auto layout = ui::VerticalLayout(0, 0, width, height, scene);
    auto searchPane = ui::HorizontalLayout(0, 0, width, 100, scene);


    return scene;
}
