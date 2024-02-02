//
// Created by brant on 1/26/24.
//

#pragma once

#include <rmkit.h>

#include <utility>
#include <list>

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
        //TODO: this is so bad
        //TODO: style sheets
        ui::TextInput::style.valign = ui::Style::MIDDLE;
        ui::TextInput::style.justify = ui::Style::LEFT;
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
            //TODO: style sheets
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



//TODO: add pagination controls
class ListBox: public RoundCornerWidget{
    public:
        struct ListItem{
            friend class ListBox;
            explicit ListItem(string label):label(std::move(label)){};          //label
            string label;                                   //the text displayed in the listbox. May only be one line.
            shared_ptr<void> object = nullptr;              //an optional pointer to any data you want to keep a reference to

            //all items must be unique. This sucks, but we'll implement it if someone needs it
            inline bool operator==(const ListItem& other)const{
                return this->label == other.label;
            }
        private:
            shared_ptr<ui::Text> _widget = nullptr;
            bool _selected = false;
        };

    //TODO: style sheets
        int itemHeight;
        int padding = 5;

        int offset = 0; //start rendering the list at the nth element
        bool multiSelect = true; //allow selecting more than one entry

        int pageSize(){
            return h / (itemHeight + padding);
        }
        int currentPage(){
            return ceil(offset/pageSize());
        }

        //please call mark_redraw() on this widget after editing contents or selections
        vector<shared_ptr<ListItem>> contents;
        std::list<shared_ptr<ListItem>> selectedItems;


        ListBox(int x, int y, int w, int h, int itemHeight) : RoundCornerWidget(x,y,w,h,RoundCornerStyle()){
            this->itemHeight = itemHeight;
        }

        void add(ListItem item){
            item._widget = make_shared<ui::Text>(x,y,w,itemHeight,item.label);
            //TODO: style sheets
            item._widget->style.valign = ui::Style::MIDDLE;
            contents.push_back(make_shared<ListItem>(item));
            this->mark_redraw();
        }

        void removeAt(int index){
            auto item = contents[index];
            auto w = item->_widget;
            contents.erase(contents.begin() + index);
            mark_redraw();
        }

        void on_reflow() override{
            mark_redraw();
        }

        void refresh_list(){
            auto count = std::min((int)pageSize(), (int)contents.size() - offset);
            auto cit = _currentView.begin();
            for(int i = offset; i < offset + count; i++){
                if(cit == _currentView.end()){
                    _currentView.push_back(contents[i]);
                    cit = _currentView.end();
                    continue;
                }
                auto citem = *cit;
                if(citem == contents[i]) {
                    cit++;
                    continue;
                }
                (*cit) = contents[i];
                cit++;
            }
            if(cit != _currentView.end()){ //there are more elements in the view than are available for the current page
                for(auto cbt = cit; cbt != _currentView.end(); cbt++)
                    (*cbt)->_widget->undraw();
                _currentView.erase(cit, _currentView.end());
            }
        }

        void undraw() override{
            fb->draw_rect(this->x,this->y,this->w,this->h,WHITE,true);
            RoundCornerWidget::undraw();
        }

        void render() override{
            undraw();
            refresh_list();
            int sx = this->x + padding;
            int sy = this->y + padding;
            for(auto item : _currentView) {
                auto wi = item->_widget;
                wi->x = sx;
                wi->y = sy;
                wi->h = itemHeight;
                wi->w = w - padding - padding;
                wi->on_reflow();
                if(item->_selected){
                    //TODO: style sheets
                    //item is selected, draw an effect
                    //I can't be bothered to make this configurable right now
                    fb->draw_rect(wi->x, wi->y, wi->w, wi->h, color::GRAY_9, true);
                }
                wi->render();
                sy += itemHeight + padding;
            }
        }

        void selectIndex(int index){
            if((offset + index) >= contents.size() || (offset + index) < 0){
                fprintf(stderr, "selectIndex out of bounds: idx[%d]\n", index);
                return;
            }
            auto item = contents[offset + index];
            if(item->_selected){
                selectedItems.remove_if([item](auto i){return item==i;});
                item->_selected = false;
            }
            else {
                item->_selected = true;
                selectedItems.push_back(item);
            }
        }

        //check the Y position relative to top of widget, divide by itemHeight
        void on_mouse_click(input::SynMotionEvent &ev) override{
            std::cout<<"CLICK"<<std::endl;
            auto hgt = itemHeight + padding;
            auto sy = ev.y - this->y;
            auto shgt = sy/hgt;
            int idx = floor(shgt);
            //printf("Click at %d,%d: computed offset %d: displayed %d\n", ev.x, ev.y, idx, displayed_items());
            std::cout<<std::endl;
            if(idx > displayed_items())
                return;
            selectIndex(idx);
            mark_redraw();
        }

    private:
        std::list<shared_ptr<ListItem>> _currentView;
        std::list<shared_ptr<ListItem>> _lastView;
        int displayed_items(){
            return std::min((int)pageSize(), (int)contents.size() - offset);
        }
    };
}
