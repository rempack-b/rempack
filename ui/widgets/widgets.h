//
// Created by brant on 1/26/24.
//

#pragma once

#include <rmkit.h>
#include <list>
#include <utility>
#include "icons/icons_embed.h"
#include "utils.h"
#include <functional>
#include <any>
#include "text_helpers.h"
#include "graphic_utils.h"

namespace widgets {

    //TODO: style sheets
    struct RoundCornerStyle {
    public:
        int cornerRadius;
        int borderThickness;
        float startColor;
        float endColor;
        bool gradient;
        //shrink the border by this many pixels. Generally, set this equal to cornerRadius
        int inset;
        float expA;
        float expB;

        RoundCornerStyle() {
            cornerRadius = 10;
            borderThickness = 10;
            startColor = 0;
            endColor = 1;
            gradient = true;
            inset = 8;
            expA = -9.f;
            expB = 1.f;
        }
    };


    class RoundCornerWidget : public ui::Widget {
    public:
        RoundCornerWidget(int x, int y, int w, int h, RoundCornerStyle style) : ui::Widget(x, y, w, h) {
            this->style = style;
        };
        RoundCornerStyle style;
        uint16_t undraw_color = WHITE;

        //TODO: this still isn't quite right
        void undraw() override {
            return;
            //top
            fb->draw_rect(x + style.inset - style.cornerRadius - style.borderThickness,
                          y + style.inset - style.cornerRadius - style.borderThickness,
                          w - style.inset + style.cornerRadius + style.borderThickness,
                          style.borderThickness,
                          undraw_color, true);
            //bottom
            fb->draw_rect(x + style.inset - style.cornerRadius - style.borderThickness,
                          y + h - style.inset + style.cornerRadius,
                          w - style.inset + style.cornerRadius + style.borderThickness,
                          style.borderThickness,
                          undraw_color, true);
            //left
            fb->draw_rect(x + style.inset - style.cornerRadius - style.borderThickness,
                          y + style.inset - style.cornerRadius - style.borderThickness,
                          style.borderThickness,
                          h - style.inset + style.cornerRadius + style.borderThickness,
                          undraw_color, true);
            //right
            fb->draw_rect(x + w - style.inset + style.cornerRadius,
                          y + style.inset - style.cornerRadius - style.borderThickness,
                          style.borderThickness,
                          h - style.inset + style.cornerRadius + style.borderThickness,
                          undraw_color, true);
        }

        void render_border() override {
            drawRoundedBox(x, y, w, h, style.cornerRadius, fb, style.borderThickness,
                           style.startColor, style.inset, style.gradient, style.endColor,
                           style.expA, style.expB);
        }

        void render_inside_fill(float gray = 1.f) {
            //draw a rounded box to fill the awkward space between the border and inner content
            drawRoundedBox(x, y, w, h, style.cornerRadius, fb, style.cornerRadius,
                           gray, style.inset + (style.cornerRadius), false, 1, 1, 1, 2.f); //extra junk here because C++ doesn't support
            //named parameters and I need to change the alpha
            //draw a rectangle to cover the rest of the inner area
            fb->draw_rect(x + style.inset, y + style.inset,
                          w - style.inset - style.inset, h - style.inset - style.inset,
                          color::from_float(gray), true);
        }
    };

    //same as ui::TextInput except it draws fancy rounded corners
    class RoundedTextInput : public ui::TextInput {
    public:
        RoundCornerStyle style;
        shared_ptr<RoundCornerWidget> border;

        RoundedTextInput(int x, int y, int w, int h, RoundCornerStyle style, string text = "") : ui::TextInput(x, y, w, h, std::move(text)) {
            //TODO: style sheets
            ui::TextInput::style.valign = ui::Style::MIDDLE;
            ui::TextInput::style.justify = ui::Style::LEFT;
            this->style = style;
            border = make_shared<RoundCornerWidget>(x, y, w, h, style);
            children.push_back(border);
        }

        void undraw() override {
            auto sx = x - style.cornerRadius - style.borderThickness + style.inset;
            auto sy = y - style.cornerRadius - style.borderThickness + style.inset;
            auto dx = sx + w + style.cornerRadius + style.borderThickness + style.inset;
            auto dy = sy + h + style.cornerRadius + style.borderThickness + style.inset;
            fb->draw_rect(sx, sy, dx, dy, WHITE, true);
        }

        void render() override {
            fb->waveform_mode = WAVEFORM_MODE_GC16;
            ui::TextInput::render();
        }

        void on_reflow() override {
            border->set_coords(x, y, w, h);
            border->mark_redraw();
        }

        void render_border() override {
            border->style = style;
            //stop parent class from rendering its border
        }
    };

    class LabeledRangeInput : public ui::Widget {
    public:
        enum LabelPosition {
            LEFT, TOP
        };

        LabeledRangeInput(int x, int y, int w, int h, string text = "", LabelPosition pos = LEFT, int padding = 5)
                : ui::Widget(x, y, w, h) {
            if (!text.empty()) {
                label = make_shared<ui::Text>(x, y, w, (h / 2) - padding, text);
                children.push_back(label);
                y += h / 2;
                h = h / 2 - padding;
            }
            range = make_shared<ui::RangeInput>(x, y, w, h);

            children.push_back(range);
            w = range->x + range->w - x;
            h = range->y + range->h - y;
        }

        void mark_redraw() override {
            range->mark_redraw();
            if (label != nullptr)
                label->mark_redraw();
        }


        shared_ptr<ui::RangeInput> range;
    private:
        shared_ptr<ui::Text> label = nullptr;
    };

}
