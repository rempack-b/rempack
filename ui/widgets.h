//
// Created by brant on 1/26/24.
//

#pragma once

#include <rmkit.h>
#include <list>
#include <utility>
#include "../assets/icons/icons_embed.h"
#include "utils.h"
#include <functional>
#include <any>
#include "text_helpers.h"

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
            fb->waveform_mode = WAVEFORM_MODE_GC16;
            drawRoundedBox(x, y, w, h, style.cornerRadius, fb, style.borderThickness,
                           style.startColor, style.inset, style.gradient, style.endColor,
                           style.expA, style.expB);
        }
    };

    class RoundImageButton : public ImageButton{
    public:

        shared_ptr<RoundCornerWidget> border;
        RoundImageButton(int x, int y, int w, int h, icons::Icon icon, RoundCornerStyle style): ImageButton(x,y,w,h,icon){
            border = make_shared<RoundCornerWidget>(x,y,w,h,style);
            children.push_back(border);
        }
        void on_reflow()override{
            border->set_coords(x,y,w,h);
            border->mark_redraw();
            ImageButton::on_reflow();
        }
    };

    //same as ui::TextInput except it draws fancy rounded corners
class RoundedTextInput: public ui::TextInput{
public:
RoundCornerStyle style;
shared_ptr<RoundCornerWidget> border;
    RoundedTextInput(int x, int y, int w, int h, RoundCornerStyle style, string text = ""): ui::TextInput(x,y,w,h,std::move(text)){
        //TODO: this is so bad
        //TODO: style sheets
        ui::TextInput::style.valign = ui::Style::MIDDLE;
        ui::TextInput::style.justify = ui::Style::LEFT;
        this->style = style;
        border = make_shared<RoundCornerWidget>(x,y,w,h,style);
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

    void on_reflow()override{
        border->set_coords(x,y,w,h);
        border->mark_redraw();
    }

    void render_border() override{
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
            explicit ListItem(string label, std::any object):label(std::move(label)),object(std::move(object)){};
            string label;                                   //the text displayed in the listbox. May only be one line.
            std::any object;             //an optional pointer to any data you want to keep a reference to

            //all items must be unique. This sucks, but we'll implement it if someone needs it
            inline bool operator==(const ListItem& other)const{
                return this->label == other.label;
            }
        private:
            shared_ptr<ui::Text> _widget = nullptr;
            bool _selected = false;
        };

    PLS_DEFINE_SIGNAL(LISTBOX_EVENT, const shared_ptr<ListItem>);
    class LISTBOX_EVENTS{
    public:
        LISTBOX_EVENT selected;
        LISTBOX_EVENT deselected;
        LISTBOX_EVENT added;
        LISTBOX_EVENT removed;
    };

    LISTBOX_EVENTS events;

    std::function<bool(const shared_ptr<ListItem>&)> filterPredicate = dummy_filter;

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
        std::unordered_set<shared_ptr<ListItem>> selectedItems;

        ListBox(int x, int y, int w, int h, int itemHeight) : RoundCornerWidget(x,y,w,h,RoundCornerStyle()){
            this->itemHeight = itemHeight;
        }

        //TODO: add backing index to item as we add it
        void add(string label, std::any object = nullptr){
            auto item = make_shared<ListItem>(label, object);
            item->_widget = make_shared<ui::Text>(x,y,w,itemHeight,label);
            //TODO: style sheets
            item->_widget->style.valign = ui::Style::MIDDLE;
            item->_widget->style.justify = ui::Style::LEFT;
            contents.push_back(item);
            events.added(item);
            this->mark_redraw();
        }

        bool remove(string label){
            //sure, you could use std::find but C++ Lambdas are an affront to all that is good in this world
            int i = 0;
            shared_ptr<ListItem> item = nullptr;
            for(; i <contents.size(); i++)
            {
                auto ti = contents[i];
                if(label == ti->label) {
                    item = ti;
                    break;
                }
            }
            if(item != nullptr){
                contents.erase(contents.begin() + i);
                events.removed(item);
                mark_redraw();
                return true;
            }
            return false;
        }

        void removeAt(int index){
            auto item = contents[index];
            auto w = item->_widget;
            contents.erase(contents.begin() + index);
            events.removed(item);
            mark_redraw();
        }

        void trim_texts(){
            for(const auto &it : contents){
                auto wd = it->_widget;
                wd->text = utils::clip_string(it->label, wd->w, wd->h, ui::Widget::style.font_size);
                wd->mark_redraw();
            }
        }

        void on_reflow() override{
            trim_texts();
            mark_redraw();
        }

        //first, filter contents with our predicate and copy to current view
        //second, sort current view
        virtual void refresh_list(){
            int si = 0;
            for(auto & item : contents){
                if(filterPredicate(item)) {
                    if(si >= _sortedView.size())
                        _sortedView.push_back(item);
                    else
                        _sortedView[si] = item;
                    si++;
                }
            }
            _sortedView.erase(_sortedView.begin() + si, _sortedView.end());
            std::sort(_sortedView.begin(), _sortedView.end());

            auto count = std::min((int)pageSize(), (int)_sortedView.size() - offset);
            auto cit = _currentView.begin();
            for(int i = offset; i < offset + count; i++){
                if(cit == _currentView.end()){
                    _currentView.push_back(_sortedView[i]);
                    cit = _currentView.end();
                    continue;
                }
                auto citem = *cit;
                if(citem == _sortedView[i]) {
                    cit++;
                    continue;
                }
                (*cit) = _sortedView[i];
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
            fb->waveform_mode = WAVEFORM_MODE_GC16;
        }

        void selectIndex(int index) {
            if(index >= _currentView.size()) {
                fprintf(stderr, "selectIndex out of bounds: idx[%d]\n", index);
                return;
            }
            auto item = _currentView[index];
            if (item->_selected) {
                selectedItems.erase(item);
                item->_selected = false;
                events.deselected(item);
            } else {
                item->_selected = true;
                if(multiSelect)
                    selectedItems.emplace(item);
                else{
                    for(const auto& si : selectedItems){
                        si->_selected = false;
                        events.deselected(si);
                    }
                    selectedItems.clear();
                    selectedItems.emplace(item);
                }
                events.selected(item);
            }
        }

        //check the Y position relative to top of widget, divide by itemHeight
        void on_mouse_click(input::SynMotionEvent &ev) override{
            auto hgt = itemHeight + padding;
            auto sy = ev.y - this->y;
            auto shgt = sy/hgt;
            int idx = floor(shgt);
            //printf("Click at %d,%d: computed offset %d: displayed %d\n", ev.x, ev.y, idx, displayed_items());
            //std::cout<<std::endl;
            if(idx > displayed_items())
                return;
            selectIndex(idx);
            mark_redraw();
        }

    protected:
        std::vector<shared_ptr<ListItem>> _currentView;
        std::vector<shared_ptr<ListItem>> _sortedView;
        int displayed_items(){
            return min((int)pageSize(), (int)contents.size() - offset);
        }
    private:
        static bool dummy_filter(shared_ptr<ListItem> it){
            return true;
        }
    };
}
