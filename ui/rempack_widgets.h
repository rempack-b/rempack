//
// Created by brant on 2/5/24.
//

#pragma once

#include "widgets.h"

#include <utility>

namespace widgets{

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
            RoundedTextInput::on_reflow();
        }
    private:
        shared_ptr<ui::Pixmap> pixmap;
    };

    class ConfigButton : public RoundImageButton {
    public:
        RoundCornerStyle style;

        ConfigButton(int x, int y, int w, int h, RoundCornerStyle style = RoundCornerStyle()) : RoundImageButton(x, y, w, h, ICON(assets::png_menu_png), style) {

        }

    };

    class FilterButton:public RoundImageButton{
    public:
        RoundCornerStyle style;

        FilterButton(int x, int y, int w, int h, RoundCornerStyle style = RoundCornerStyle()) : RoundImageButton(x, y, w, h, ICON(assets::png_filter_png), style) {

        }

    };

    class PackageInfoPanel: public RoundCornerWidget{
    public:
        PackageInfoPanel(int x, int y, int w, int h, RoundCornerStyle style) : RoundCornerWidget(x,y,w,h,style){
            _text = make_shared<ui::MultiText>(x,y,w,h,"");
            children.push_back(_text);
        }

        void on_reflow() override{
            _text->set_coords(x,y,w,h);
            _text->mark_redraw();
        }

        void set_text(string text){
            _text->undraw();
            _text->text = std::move(text);
            _text->mark_redraw();
            this->mark_redraw();
        }
    private:
        shared_ptr<ui::MultiText> _text;
    };
}