//
// Created by brant on 1/26/24.
//

#pragma once

#include <rmkit.h>

#include <utility>

#include "utils.h"
namespace widgets{
    /*
 * ╭----╮
 * |    |
 * ╰----╯
 *
 * Draws all four rounded corners as described by the center and radius of the top-left corner,
 * and the offset to the bottom-right corner
 *
 * Optionally, you can specify a color for the border, its thickness, and a gradient
 * Thickness expands outward from the centerpoint, and the gradient is a simple linear function
 */

    inline void drawRoundedCorners(int x0, int y0, int ox, int oy, int radius, framebuffer::FB* fb, float grayfColor = 0, uint stroke = 1, bool gradient = false, float grayfendColor = 1) {
        int x = 0;
        int y = radius;
        int d = -(radius >> 1);
        int w = stroke;
        int h = stroke;
        auto color = color::from_float(grayfColor);

        if(!gradient){
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
            }}
        else{
            auto dc = abs(grayfendColor - grayfColor)/(float)stroke;
            for(uint si = 0; si <= stroke; si++){
                drawRoundedCorners(x0,y0,ox,oy,radius+si,fb,grayfColor + utils::sigmoid(dc * si, -20, 8));
            }
        }
    }

    inline void drawRoundedBox(int x0, int y0, int w, int h, int radius, framebuffer::FB* fb, int stroke = 1, float grayfColor = 0, int shrink = 0, bool gradient = false, float grayfendColor = 1){
        int sx = x0 + shrink;
        int sy = y0 + shrink;
        int dx = w - (2 * shrink);
        int dy = h - (2 * shrink);

        fb->draw_rect(sx - radius - stroke, sy - radius - stroke,
                      sx + dx + radius + stroke, sy + dy + radius + stroke, WHITE, true);

        if(!gradient){
            auto color = color::from_float(grayfColor);
            drawRoundedCorners(sx,sy,dx,dy, radius, fb, grayfColor, stroke);
            fb->_draw_rect_fast(sx - stroke - radius, sy, stroke, dy, color);
            fb->_draw_rect_fast(sx + dx + radius, sy, stroke, dy, color);
            fb->_draw_rect_fast(sx, sy - stroke - radius, dx, stroke, color);
            fb->_draw_rect_fast(sx, sy + dy + radius, dx, stroke, color);
        }
        else{
            drawRoundedCorners(sx,sy,dx,dy,radius,fb,grayfColor,stroke,gradient,grayfendColor);
            float dc = abs(grayfendColor - grayfColor)/(float)stroke;
            for(int i = 0; i <= stroke; i++){
                auto color = color::from_float(utils::sigmoid(grayfColor + (dc * i), -20, 8));
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

    inline void moveChildren(const shared_ptr<ui::Widget> &widget, int lastx, int lasty){
        int x = widget->x - lastx;
        int y = widget->y - lasty;
        for(const auto& c: widget->children){
            auto lx = c->x;
            auto ly = c->y;
            c->x += x;
            c->y += y;
            moveChildren(c,lx,ly);
        }
    }

//basically a reimplementation of ui::Button with a clickable image instead of text
    class ImageButton :public ui::Widget{
    public:
        ImageButton(int x, int y, int w, int h, icons::Icon icon): Widget(x,y,w,h){
            pixmap = make_shared<ui::Pixmap>(x, y, w, h, icon);
        }
        void on_mouse_enter(input::SynMotionEvent &ev) override {dirty = 1;}
        void on_mouse_leave(input::SynMotionEvent &ev) override {dirty = 1;}
        void on_mouse_down(input::SynMotionEvent &ev) override {dirty = 1;}
        void on_mouse_click(input::SynMotionEvent &ev) override {dirty = 1;}

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

#ifdef DEBUG_DRAW
            fb->draw_rect(x,y,w,h,BLACK,false);
#endif
        }
    private:
        shared_ptr<ui::Pixmap> pixmap;
    };

    struct RoundCornerStyle{
    public:
        int cornerRadius;
        int borderThickness;
        float startColor;
        float endColor;
        bool gradient;
        //move the center point of the corner radius inwards
        int inset;
        int expA;
        int expB;

        RoundCornerStyle(){
            cornerRadius = 10;
            borderThickness = 8;
            startColor = 0;
            endColor = 1;
            gradient = true;
            inset = 1;
            expA = -20;
            expB = 6;
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
        auto dx = sx + w - style.cornerRadius - style.borderThickness + style.inset;
        auto dy = sy + h - style.cornerRadius - style.borderThickness + style.inset;
        fb->draw_rect(sx, sy, dx, dy, WHITE, true);
    }
    void render_border() override {
        fb->waveform_mode = WAVEFORM_MODE_GC16;
        drawRoundedBox(x, y, w, h, style.cornerRadius, fb, style.borderThickness,
                       style.startColor, style.inset, style.gradient, style.endColor);
    }
};

    class SearchBox: public RoundedTextInput{
    public:
        SearchBox(int x, int y, int w, int h, const string text = ""): RoundedTextInput(x, y, w, h, RoundCornerStyle(), text){
            pixmap = make_shared<ui::Pixmap>(x + w - h, y, h, h, ICON(assets::png_search_png));
        }

        void render() override{
            pixmap->undraw();
            pixmap->x=x+w-h;
            pixmap->y=y;
            pixmap->render();
#ifdef DEBUG_DRAW
            fb->draw_rect(pixmap->x,pixmap->y,pixmap->w,pixmap->h,BLACK,false);
#endif
            RoundedTextInput::render();
        }
    private:
        shared_ptr<ui::Pixmap> pixmap;
    };

    class ConfigButton : public ImageButton{
    public:
        ConfigButton(int x, int y, int w, int h):ImageButton(x,y,w,h,ICON(assets::png_menu_png)){

        }
    };

class LabeledRangeInput: public ui::Widget{
public:
    enum LabelPosition{LEFT,TOP};
    LabeledRangeInput(int x, int y, int w, int h, string text = "", LabelPosition pos = LEFT, int padding = 5): ui::Widget(x,y,w,h){
        if(!text.empty()) {
            label = make_shared<ui::Text>(x, y, w, (h / 2) - padding, text);
            children.push_back(label);
            x += h/2;
            h = h/2 - padding;
        }
        range = make_shared<ui::RangeInput>(x,y,w,h);
        children.push_back(range);
    }
    shared_ptr<ui::RangeInput> range;
private:
    shared_ptr<ui::Text> label = nullptr;
};

class RoundCornerEditor: public ui::Widget{
public:
    RoundCornerEditor(int x, int y, int w, int h, RoundCornerStyle style, RoundedTextInput *target):ui::Widget(x,y,w,h){
        this->target = target;
        this->style = style;
        int padding = 5;
        int lineHeight = 100;
        inputA = make_shared<LabeledRangeInput>(x, y, w, lineHeight, "Exp");
        y+=lineHeight+padding;
        inputA->range->set_range(-50,10);
        inputA->range->set_value(-12);
        inputA->range->events.change += PLS_DELEGATE(this->updateSliderA);
        children.push_back(inputA);
        inputB = make_shared<LabeledRangeInput>(x, y, w, lineHeight, "Coef");
        y+=lineHeight+padding;
        inputB->range->set_range(0,20);
        inputB->range->set_value(6);
        inputB->range->events.change += PLS_DELEGATE(this->updateSliderB);
        children.push_back(inputB);
        inputT = make_shared<LabeledRangeInput>(x, y, w, lineHeight, "THICC");
        y+=lineHeight+padding;
        inputT->range->set_range(0,50);
        inputT->range->set_value(6);
        inputT->range->events.change += PLS_DELEGATE(this->updateSliderT);
        children.push_back(inputT);
    }
private:
    RoundedTextInput *target;
    RoundCornerStyle style;
    shared_ptr<LabeledRangeInput> inputA, inputB, inputT, inputS, inputEnd;
    shared_ptr<ui::ToggleButton> gradientSw;
    void updateSliderA(float p){        target->style.expA = inputA->range->get_value();  target->mark_redraw();  }
    void updateSliderB(float p){        target->style.expB = inputB->range->get_value();    target->mark_redraw(); }
    void updateSliderT(float p){        target->style.borderThickness = inputT->range->get_value();    target->mark_redraw(); }
    void updateSliderS(float p){        target->style.startColor = p;    target->mark_redraw(); }
    void updateSliderE(float p){        target->style.endColor = p;    target->mark_redraw(); }
};
}
