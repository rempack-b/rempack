//
// Created by brant on 2/27/24.
//

#ifndef REMPACK_BUTTONS_H
#define REMPACK_BUTTONS_H

#include "widgets.h"

class EventButton : public ui::Button {
public:
    EventButton(int x, int y, int w, int h, string text = "") : Button(x, y, w, h, text) {}

    PLS_DEFINE_SIGNAL(BUTTON_EVENT, void*);

    class BUTTON_EVENTS {
    public:
        BUTTON_EVENT clicked;
    };

    BUTTON_EVENTS events;

    void on_mouse_click(input::SynMotionEvent &ev) override {
        ev.stop_propagation();
        if (!enabled)
            return;
        events.clicked();
        mark_redraw();
    }

    void disable() {
        enabled = false;
        mark_redraw();
    }

    void enable() {
        enabled = true;
        mark_redraw();
    }

    void render() override {
        Button::render();
        if (!enabled) {
            fb->draw_rect(x, y, w, h, color::GRAY_12, true);
        }
        //fb->draw_rect(x,y,w,h,BLACK,false);
    }

protected:
    bool enabled = true;
};

//basically a reimplementation of ui::Button with a clickable image instead of text
class ImageButton : public EventButton {
public:
    ImageButton(int x, int y, int w, int h, icons::Icon icon) : EventButton(x, y, w, h, "") {
        pixmap = make_shared<ui::Pixmap>(x, y, w, h, icon);
        pixmap->alpha = WHITE;
        children.push_back(pixmap);
    }


    void render() override {
        EventButton::render();
        //fb->waveform_mode = WAVEFORM_MODE_GC16;
        pixmap->render();
    }

    void on_reflow() override {
        //pixmap->undraw();
        auto dw = min(w, h);
        auto dx = x + (w / 2) - (dw / 2);
        pixmap->set_coords(dx, y, dw, dw);
        //pixmap->icon.width  = dw;
        //pixmap->icon.height = dw;
        //util::resize_image(pixmap->icon.image, dw, dw, 20);
        pixmap->on_reflow();
        pixmap->mark_redraw();
    }

    void on_mouse_click(input::SynMotionEvent &ev) override {
        ev.stop_propagation();
        if (!enabled)
            return;
        EventButton::on_mouse_click(ev);
    }


private:
    shared_ptr<ui::Pixmap> pixmap;
};

class RoundImageButton : public ImageButton {
public:

    shared_ptr<widgets::RoundCornerWidget> border;

    RoundImageButton(int x, int y, int w, int h, icons::Icon icon, widgets::RoundCornerStyle style) : ImageButton(x, y, w, h, icon) {
        border = make_shared<widgets::RoundCornerWidget>(x, y, w, h, style);
        children.push_back(border);
    }

    void on_reflow() override {
        border->set_coords(x, y, w, h);
        border->mark_redraw();
        ImageButton::on_reflow();
    }
};

#endif //REMPACK_BUTTONS_H
