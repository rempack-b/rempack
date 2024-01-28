//
// Created by brant on 1/26/24.
//

#pragma once

#include <rmkit.h>

#include <utility>

#include "utils.h"

namespace widgets {
    /*
     * ╭----╮
     * |    |
     * ╰----╯
     *
     * Draws all four rounded corners as described by the center and radius of the top-left corner,
     * and the offset to the bottom-right corner
     *
     * Optionally, you can specify a color for the border, its thickness, and a gradient
     * Thickness expands outward from the centerpoint, and the gradient is a sigmoid function (utils::sigmoid)
     *
     * colors are specified as a float in the range (0,1) where 0 is black and 1 is white (see color::from_float)
     *
     * This is an adaptation of the circle outline algorithm at framebuffer::draw_circle_outline
     * The gradient is accomplished by drawing multiple 1px arcs with diminishing color in 1px steps away from center
     *
     * We only actually compute one eighth of the circle and mirror it to all the other octants
     */

    inline void drawRoundedCorners(int x0, int y0, int ox, int oy, int radius, framebuffer::FB *fb,
                                   float grayfColor = 0, uint stroke = 1, bool gradient = false,
                                   float grayfendColor = 1,
                                   float expA = -20.f, float coefB = 8) {
        int x = 0;
        int y = radius;
        int d = -(radius >> 1);
        int w = stroke;
        int h = stroke;
        auto color = color::from_float(grayfColor);

        //since we use _draw_rect_fast, we need to manually mark the area dirty
        fb->update_dirty(fb->dirty_area, x0 - radius - stroke, y0 - radius - stroke);
        fb->update_dirty(fb->dirty_area, x0 + ox + radius + stroke, y0 + oy + radius + stroke);
        fb->waveform_mode = WAVEFORM_MODE_GC16;

        if (!gradient) {
            while (x <= y) {
                //q4
                fb->_draw_rect_fast(-x + x0 - stroke, -y + y0 - stroke, w, h, color);
                fb->_draw_rect_fast(-y + x0 - stroke, -x + y0 - stroke, w, h, color);
                //q1
                fb->_draw_rect_fast(x + x0 + ox, -y + y0 - stroke, w, h, color);
                fb->_draw_rect_fast(y + x0 + ox, -x + y0 - stroke, w, h, color);
                //q2
                fb->_draw_rect_fast(x + x0 + ox, y + y0 + oy, w, h, color);
                fb->_draw_rect_fast(y + x0 + ox, x + y0 + oy, w, h, color);
                //q3
                fb->_draw_rect_fast(-x + x0 - stroke, y + y0 + oy, w, h, color);
                fb->_draw_rect_fast(-y + x0 - stroke, x + y0 + oy, w, h, color);

                if (d <= 0) {
                    x++;
                    d += x;
                } else {
                    y--;
                    d -= y;
                }
            }
        } else {
            auto dc = abs(grayfendColor - grayfColor) / (float) stroke;
            for (uint si = 0; si <= stroke; si++) {
                drawRoundedCorners(x0, y0, ox, oy, radius + si, fb, utils::sigmoid(grayfColor + dc * si, expA, coefB));
            }
        }
    }

    //draws a box with rounded corners with some style options
    //calls into drawRoundedCorners first, then draws the lines connecting the arcs
    //gradient is based on a sigmoid function (see utils::sigmoid)
    //colors are specified as a float in the range (0,1) where 0 is black and 1 is white (see color::from_float)
    inline void drawRoundedBox(int x0, int y0, int w, int h, int radius, framebuffer::FB *fb,
                               int stroke = 1, float grayfColor = 0, int shrink = 0, bool gradient = false,
                               float grayfendColor = 1,
                               float expA = -20.f, float coefB = 8) {
        int sx = x0 + shrink;
        int sy = y0 + shrink;
        int dx = w - (2 * shrink);
        int dy = h - (2 * shrink);

        if (!gradient) {
            auto color = color::from_float(grayfColor);
            drawRoundedCorners(sx, sy, dx, dy, radius, fb, grayfColor, stroke);
            fb->_draw_rect_fast(sx - stroke - radius, sy, stroke, dy, color);
            fb->_draw_rect_fast(sx + dx + radius, sy, stroke, dy, color);
            fb->_draw_rect_fast(sx, sy - stroke - radius, dx, stroke, color);
            fb->_draw_rect_fast(sx, sy + dy + radius, dx, stroke, color);
        } else {
            drawRoundedCorners(sx, sy, dx, dy, radius, fb, grayfColor, stroke, gradient, grayfendColor, expA, coefB);
            float dc = abs(grayfendColor - grayfColor) / (float) stroke;
            for (int i = 0; i <= stroke; i++) {
                auto color = color::from_float(utils::sigmoid(grayfColor + (dc * i), expA, coefB));
                //left
                fb->_draw_rect_fast(sx - i - radius - 1, sy, 1, dy, color);
                //right
                fb->_draw_rect_fast(sx + dx + radius + i, sy, 1, dy, color);
                //top
                fb->_draw_rect_fast(sx, sy - i - radius - 1, dx, 1, color);
                //bottom
                fb->_draw_rect_fast(sx, sy + dy + i + radius, dx, 1, color);
            }
        }
    }


//basically a reimplementation of ui::Button with a clickable image instead of text
    class ImageButton : public ui::Button {
    public:
        ImageButton(int x, int y, int w, int h, icons::Icon icon) : Button(x, y, w, h, "") {
            pixmap = make_shared<ui::Pixmap>(x, y, w, h, icon);
            children.push_back(pixmap);
        }

    void on_reflow() override{
            pixmap->set_coords(x,y,w,h);
            pixmap->mark_redraw();
        }

    private:
        shared_ptr<ui::Pixmap> pixmap;
    };

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
            inset = 0;
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

        void undraw() override {
            //top
            fb->draw_rect(x + style.inset - style.cornerRadius - style.borderThickness,
                          y + style.inset - style.cornerRadius - style.borderThickness,
                          x + w - style.inset + style.cornerRadius + style.borderThickness,
                          style.borderThickness,
                          WHITE, true);
            //bottom
            fb->draw_rect(x + style.inset - style.cornerRadius - style.borderThickness,
                          y + h - style.inset + style.cornerRadius,
                          x + w - style.inset + style.cornerRadius + style.borderThickness,
                          style.borderThickness,
                          WHITE, true);
            //left
            fb->draw_rect(x + style.inset - style.cornerRadius - style.borderThickness,
                          y + style.inset - style.cornerRadius - style.borderThickness,
                          style.borderThickness,
                          y + h - style.inset + style.cornerRadius + style.borderThickness,
                          WHITE, true);
            //right
            fb->draw_rect(x + w - style.inset + style.cornerRadius,
                          y + style.inset - style.cornerRadius - style.borderThickness,
                          style.borderThickness,
                          y + h - style.inset + style.cornerRadius + style.borderThickness,
                          WHITE, true);
        }

        void render_border() override {
            fb->waveform_mode = WAVEFORM_MODE_GC16;
            drawRoundedBox(x, y, w, h, style.cornerRadius, fb, style.borderThickness,
                           style.startColor, style.inset, style.gradient, style.endColor,
                           style.expA, style.expB);
        }
    };

    //same as ui::TextInput except it draws fancy rounded corners
class RoundedTextInput: public ui::TextInput{
public:
RoundCornerStyle style;
    RoundedTextInput(int x, int y, int w, int h, RoundCornerStyle style, string text = ""): ui::TextInput(x,y,w,h,std::move(text)){
        this->style = style;
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
        drawRoundedBox(x, y, w, h, style.cornerRadius, fb, style.borderThickness,
                       style.startColor, style.inset, style.gradient, style.endColor,
                       style.expA, style.expB);
    }
};

    class SearchBox : public RoundedTextInput {
    public:
        SearchBox(int x, int y, int w, int h, RoundCornerStyle style, const string text = "") : RoundedTextInput(x, y, w, h, style, text) {
            pixmap = make_shared<ui::Pixmap>(x + w - h, y, h, h, ICON(assets::png_search_png));
            children.push_back(pixmap);
        }

        void on_reflow() override{
            pixmap->set_coords(x + w - h, y, h, h);
            pixmap->mark_redraw();
        }
    private:
        shared_ptr<ui::Pixmap> pixmap;
    };

    class ConfigButton : public ImageButton {
    public:
        RoundCornerStyle style;

        ConfigButton(int x, int y, int w, int h) : ImageButton(x, y, w, h, ICON(assets::png_menu_png)) {
            style = RoundCornerStyle();
        }

        void render() override {
            fb->waveform_mode = WAVEFORM_MODE_GC16;
            ImageButton::render();
            drawRoundedBox(x, y, w, h, style.cornerRadius, fb, style.borderThickness,
                           style.startColor, style.inset, style.gradient, style.endColor,
                           style.expA, style.expB);
        }
    };

    class GradientGizmo : public ui::Widget {
    public:
        GradientGizmo(int x, int y, int w, int h, float expA, float coefB, bool border = true) : Widget(x, y, w, h) {
            this->expA = expA;
            this->coefB = coefB;
            drawBorder = border;
        }

        float expA;
        float coefB;
        bool drawBorder;

        void render_border() override {
            if (drawBorder)
                fb->draw_rect(x - 1, y - 1, w + 2, h + 2, BLACK, false);
        }

        void undraw() override {
            fb->draw_rect(x, y, w, h, WHITE, true);
        }

        void render() override {
            float dc = 1.f / w;
            for (int i = 0; i <= w; i++) {
                fb->_draw_rect_fast(x + i, y, 1, h, color::from_float(utils::sigmoid(dc * i, expA, coefB)));
            }
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

        void mark_redraw() override{
            range->mark_redraw();
            if(label != nullptr)
                label->mark_redraw();
        }


        shared_ptr<ui::RangeInput> range;
    private:
        shared_ptr<ui::Text> label = nullptr;
    };

    class RoundCornerEditor : public ui::Widget {
    public:
        RoundCornerEditor(int x, int y, int w, int h, RoundedTextInput *target) : ui::Widget(x,y,w,h) {
            this->target = target;
            this->style= target->style;
            int padding = 20;
            //add a few gradient previews
            auto _x = x;
            int gh = 200;
            auto g1 = make_shared<GradientGizmo>(x, y, 5, gh, style.expA, style.expB, true);
            x += g1->w + padding;
            auto g2 = make_shared<GradientGizmo>(x, y, 10, gh, style.expA, style.expB, true);
            x += g2->w + padding;
            auto g3 = make_shared<GradientGizmo>(x, y, 50, gh, style.expA, style.expB, true);
            x += g3->w + padding;
            auto g4 = make_shared<GradientGizmo>(x, y, 100, gh, style.expA, style.expB, true);
            x += g4->w + padding;
            auto g5 = make_shared<GradientGizmo>(x, y, 500, gh, style.expA, style.expB, true);
            x = _x;
            y += g1->h + padding;
            gradients.push_back(g1);
            gradients.push_back(g2);
            gradients.push_back(g3);
            gradients.push_back(g4);
            gradients.push_back(g5);
            children.push_back(g1);
            children.push_back(g2);
            children.push_back(g3);
            children.push_back(g4);
            children.push_back(g5);

            int lineHeight = 100;
            inputA = make_shared<LabeledRangeInput>(x, y, w, lineHeight, "Exp");
            y += lineHeight + padding;
            inputA->range->set_range(-50, 0);
            inputA->range->set_value(style.expA);
            inputA->range->events.done += PLS_DELEGATE(this->updateSliderA);
            children.push_back(inputA);
            inputB = make_shared<LabeledRangeInput>(x, y, w, lineHeight, "Coef");
            y += lineHeight + padding;
            inputB->range->set_range(0, 20);
            inputB->range->set_value(style.expB);
            inputB->range->events.done += PLS_DELEGATE(this->updateSliderB);
            children.push_back(inputB);
            inputT = make_shared<LabeledRangeInput>(x, y, w, lineHeight, "THICC");
            y += lineHeight + padding;
            inputT->range->set_range(0, 50);
            inputT->range->set_value(style.borderThickness);
            inputT->range->events.done += PLS_DELEGATE(this->updateSliderT);
            children.push_back(inputT);
        }

        void update_values() {
            for (auto &g: gradients) {
                g->expA = target->style.expA;
                g->coefB = target->style.expB;
                g->mark_redraw();
            }
        }

    private:
        RoundedTextInput *target;
        RoundCornerStyle style;
        std::vector<shared_ptr<GradientGizmo>> gradients;
        shared_ptr<LabeledRangeInput> inputA, inputB, inputT, inputS, inputEnd;
        shared_ptr<ui::ToggleButton> gradientSw;

        void updateSliderA(float p) {
            target->style.expA = inputA->range->get_value();
            update_values();
            target->mark_redraw();
        }

        void updateSliderB(float p) {
            target->style.expB = inputB->range->get_value();
            update_values();
            target->mark_redraw();
        }

        void updateSliderT(float p) {
            target->style.borderThickness = inputT->range->get_value();
            update_values();
            target->mark_redraw();
        }

        void updateSliderS(float p) {
            target->style.startColor = p;
            inputS->mark_redraw();
            update_values();
        }

        void updateSliderE(float p) {
            target->style.endColor = p;
            inputEnd->mark_redraw();
            update_values();
        }
    };

    class ListBox: RoundCornerWidget{
    public:
        int itemHeight;
        int padding = 10;
        struct ListItem{
            string label;
            void* object;
        private:
            shared_ptr<ui::Button> _button = nullptr;
        };
        //please call mark_redraw() on this widget after editing contents
        vector<ListItem> contents;
        ListBox(int x, int y, int w, int h, int itemHeight) : RoundCornerWidget(x,y,w,h,RoundCornerStyle()){
            this->itemHeight = itemHeight;
        }

        void add(ListItem item){
            contents.push_back(item);
            this->mark_redraw();
        }

        //void before_render() override{}

        void render() override{

        }
    };

    class ColorTestWidget :ui::Widget{
    public:
        void render() override{
            undraw();
            //for(float fi = 0.f; fi <=1.f; fi += 0.)

        }
    };
}
