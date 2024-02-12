//
// Created by brant on 2/5/24.
//

#pragma once

#include "widgets.h"
#include "list_box.h"

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

    struct FilterOptions{
        bool Installed;
        bool Upgradable;
        bool NotInstalled;
        std::map<std::string, bool> Repos;
        std::map<std::string, bool> Licenses;
    };

    class FilterOverlay: RoundCornerWidget{
    public:
        ui::Scene scene;
        FilterOptions *options;
        FilterOverlay(int x, int y, int w, int h, FilterOptions *currentOptions): RoundCornerWidget(x,y,w,h,RoundCornerStyle()) {
            options = currentOptions;
            scene = make_overlay();
        }

        void show() {
            //this->scene->pinned = true;
            ui::MainLoop::show_overlay(this->scene);
        }

        void render() override{
            render_inside_fill();
        }

        void mark_redraw() override{
            for(const auto &c: children)
                c->mark_redraw();
            this->dirty=1;
        }

        PLS_DEFINE_SIGNAL(OFILTER_EVENT, FilterOptions*);

        class OFILTER_EVENTS {
        public:
            OFILTER_EVENT updated;
        };

        OFILTER_EVENTS events;

    private:
        void upate_event(){
            events.updated(options);
            mark_redraw();
        }
        /*
         * [ ] Installed
         * [ ] Upgradable
         *
         * __________________
         * | Available      |
         * | Repository list|
         * ------------------
         * __________________
         * | License type   |
         * ------------------
         *
         */
        shared_ptr<ListBox> _repoList;
        shared_ptr<ListBox> _licenseList;
        ui::Scene make_overlay() {
            int padding = 20;
            auto s = ui::make_scene();
            s->add(this);
            mark_redraw();
            //auto v = new ui::VerticalLayout(x, y, 500, 800, s);
            auto dw = 500 - padding - padding;
            auto dh = 800 - padding - padding;
            auto dx = x + padding;
            auto dy = y + padding;
            auto iTog = make_shared<ui::ToggleButton>(dx, dy, dw, 50, "Installed");
            iTog->toggled = options->Installed;
            iTog->style.justify = ui::Style::JUSTIFY::LEFT;
            iTog->events.toggled += [this](bool s){options->Installed = s; upate_event();};
            children.push_back(iTog);
            dy += padding + iTog->h;
            auto uTog = make_shared<ui::ToggleButton>(dx, dy, dw, 50, "Upgradable");
            uTog->toggled = options->Upgradable;
            uTog->style.justify = ui::Style::JUSTIFY::LEFT;
            uTog->events.toggled += [this](bool s){options->Upgradable = s; upate_event();};
            children.push_back(uTog);
            dy += padding + uTog->h;
            auto unTog = make_shared<ui::ToggleButton>(dx, dy, dw, 50, "Not Installed");
            unTog->toggled = options->NotInstalled;
            unTog->style.justify = ui::Style::JUSTIFY::LEFT;
            unTog->events.toggled += [this](bool s){options->NotInstalled = s; upate_event();};
            children.push_back(unTog);
            dy += padding + unTog->h;
            if(!options->Repos.empty()) {
                //TODO: set height of the list based on number of entries
                _repoList = make_shared<ListBox>(dx, dy, dw, 200, 25);
                for(auto &[r, set]: options->Repos){
                    auto item = _repoList->add(r);
                    if(set) {
                        item->_selected = true;
                        _repoList->selectedItems.emplace(item);
                    }
                }
                _repoList->mark_redraw();
                _repoList->events.selected += [this](shared_ptr<ListBox::ListItem> li){ options->Repos[li->label] = true; upate_event(); };
                _repoList->events.deselected += [this](shared_ptr<ListBox::ListItem> li){ options->Repos[li->label] = false; upate_event(); };
                children.push_back(_repoList);
                dy += padding + _repoList->h;
            }
            if(!options->Licenses.empty()) {
                _licenseList = make_shared<ListBox>(dx, dy, dw, 200, 25);
                for(auto &[l, set] : options->Licenses) {
                    auto item = _licenseList->add(l);
                    if(set) {
                        item->_selected = true;
                        _licenseList->selectedItems.emplace(item);
                    }
                }
                children.push_back(_licenseList);
                _licenseList->mark_redraw();
                _licenseList->events.selected += [this](shared_ptr<ListBox::ListItem> li){ options->Licenses[li->label] = true; upate_event(); };
                _licenseList->events.deselected += [this](shared_ptr<ListBox::ListItem> li){ options->Licenses[li->label] = false; upate_event(); };
            }
            return s;
        }
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
        FilterOptions *options;
        FilterButton(int x, int y, int w, int h, FilterOptions defaultOptions, RoundCornerStyle style = RoundCornerStyle()) : RoundImageButton(x, y, w, h, ICON(assets::png_filter_png), style) {
            options = new FilterOptions(std::move(defaultOptions));
        }
        ~FilterButton() override{
            delete options;
        }
        PLS_DEFINE_SIGNAL(FILTER_EVENT, FilterOptions*);

        class FILTER_EVENTS: public BUTTON_EVENTS {
        public:
            FILTER_EVENT updated;
        };

        FILTER_EVENTS events;

        void on_overlay_hidden(ui::InnerScene::DialogVisible v){
            events.updated(options);
        }

        void on_mouse_click(input::SynMotionEvent &ev) override{
            ev.stop_propagation();

            auto ov = new FilterOverlay(x+w,y+h,500,800,options);
            ov->scene->on_hide += PLS_DELEGATE(on_overlay_hidden);
            ov->events.updated += [this, &ov](FilterOptions* o){events.updated(options);};
            ov->show();
            ui::MainLoop::refresh();
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