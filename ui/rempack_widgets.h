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

    //buttons: (re)install, uninstall, download, (preview image)
    class PackageInfoPanel: public RoundCornerWidget{
    public:
        int padding = 5;
        int controlHeight = 40;
        int controlWidth = 200;
        PackageInfoPanel(int x, int y, int w, int h, RoundCornerStyle style) : RoundCornerWidget(x,y,w,h,style){
            _text = make_shared<ui::MultiText>(x,y,w,h,"");
            _text->set_coords(x+padding,y+padding,w-(2*padding),h-(2*padding) - controlHeight);
            children.push_back(_text);
            _installBtn = make_shared<EventButton>(x,y,200, controlHeight,"Install");
            _removeBtn = make_shared<EventButton>(x,y,200, controlHeight,"Uninstall");
            _downloadBtn = make_shared<EventButton>(x,y,200, controlHeight,"Download");
            _previewBtn = make_shared<EventButton>(x,y,200, controlHeight,"Preview");
            children.push_back(_installBtn);
            children.push_back(_removeBtn);
            children.push_back(_downloadBtn);
            children.push_back(_previewBtn);
            layout_buttons();
        }

        void on_reflow() override{
            _text->set_coords(x+padding,y+padding,w-(2*padding),h-(2*padding) - controlHeight);
            _text->mark_redraw();
            layout_buttons();
        }

        void set_text(string text){
            _text->undraw();
            _text->text = std::move(text);
            _text->mark_redraw();
            this->mark_redraw();
        }
    private:
        shared_ptr<ui::MultiText> _text;
        shared_ptr<EventButton> _installBtn, _removeBtn, _downloadBtn, _previewBtn;

        void layout_buttons(){
            auto dx = x + padding;
            auto dy = y + h - padding - controlHeight;
            _installBtn->set_coords(dx,dy,controlWidth,controlHeight);
            dx += controlWidth + padding;
            _removeBtn->set_coords(dx,dy,controlWidth,controlHeight);
            dx += controlWidth + padding;
            _downloadBtn->set_coords(dx,dy,controlWidth,controlHeight);
            dx += controlWidth + padding;
            _previewBtn->set_coords(dx,dy,controlWidth,controlHeight);

            _installBtn->on_reflow();
            _removeBtn->on_reflow();
            _downloadBtn->on_reflow();
            _previewBtn->on_reflow();
            _installBtn->mark_redraw();
            _removeBtn->mark_redraw();
            _downloadBtn->mark_redraw();
            _previewBtn->mark_redraw();
        }
    };
}