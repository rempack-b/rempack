//
// Created by brant on 2/5/24.
//

#pragma once

#include <unordered_set>
#include "widgets.h"
namespace widgets {
//TODO: add pagination controls
    class ListBox : public RoundCornerWidget {
    public:
        struct ListItem {
            friend class ListBox;

            explicit ListItem(string label) : label(std::move(label)) {};          //label
            explicit ListItem(string label, std::any object) : label(std::move(label)), object(std::move(object)) {};
            string label;                                   //the text displayed in the listbox. May only be one line.
            std::any object;             //an optional pointer to any data you want to keep a reference to

            //all items must be unique. This sucks, but we'll implement it if someone needs it
            inline bool operator==(const ListItem &other) const {
                return this->label == other.label;
            }

        private:
            shared_ptr<ui::Text> _widget = nullptr;
            bool _selected = false;
        };

        PLS_DEFINE_SIGNAL(LISTBOX_EVENT, const shared_ptr<ListItem>);

        class LISTBOX_EVENTS {
        public:
            LISTBOX_EVENT selected;
            LISTBOX_EVENT deselected;
            LISTBOX_EVENT added;
            LISTBOX_EVENT removed;
        };

        LISTBOX_EVENTS events;

        std::function<bool(const shared_ptr<ListItem> &)> filterPredicate = dummy_filter;

        //TODO: style sheets
        int itemHeight;
        int padding = 5;

        int offset = 0; //start rendering the list at the nth element
        bool multiSelect = true; //allow selecting more than one entry

        int pageSize() {
            return h / (itemHeight + padding);
        }

        int currentPage() {
            return ceil(offset / pageSize());
        }

        //please call mark_redraw() on this widget after editing contents or selections
        vector<shared_ptr<ListItem>> contents;
        std::unordered_set<shared_ptr<ListItem>> selectedItems;

        ListBox(int x, int y, int w, int h, int itemHeight) : RoundCornerWidget(x, y, w, h, RoundCornerStyle()) {
            this->itemHeight = itemHeight;
        }

        //TODO: add backing index to item as we add it
        void add(string label, std::any object = nullptr) {
            auto item = make_shared<ListItem>(label, object);
            item->_widget = make_shared<ui::Text>(x, y, w, itemHeight, label);
            //TODO: style sheets
            item->_widget->style.valign = ui::Style::MIDDLE;
            item->_widget->style.justify = ui::Style::LEFT;
            contents.push_back(item);
            events.added(item);
            this->mark_redraw();
        }

        bool remove(string label) {
            //sure, you could use std::find but C++ Lambdas are an affront to all that is good in this world
            int i = 0;
            shared_ptr<ListItem> item = nullptr;
            for (; i < contents.size(); i++) {
                auto ti = contents[i];
                if (label == ti->label) {
                    item = ti;
                    break;
                }
            }
            if (item != nullptr) {
                contents.erase(contents.begin() + i);
                events.removed(item);
                mark_redraw();
                return true;
            }
            return false;
        }

        void removeAt(int index) {
            auto item = contents[index];
            auto w = item->_widget;
            contents.erase(contents.begin() + index);
            events.removed(item);
            mark_redraw();
        }

        void trim_texts() {
            for (const auto &it: contents) {
                auto wd = it->_widget;
                wd->text = utils::clip_string(it->label, wd->w, wd->h, ui::Widget::style.font_size);
                wd->mark_redraw();
            }
        }

        void on_reflow() override {
            trim_texts();
            mark_redraw();
        }

        //first, filter contents with our predicate and copy to current view
        //second, sort current view
        virtual void refresh_list() {
            int si = 0;
            for (auto &item: contents) {
                if (filterPredicate(item)) {
                    if (si >= _sortedView.size())
                        _sortedView.push_back(item);
                    else
                        _sortedView[si] = item;
                    si++;
                }
            }
            _sortedView.erase(_sortedView.begin() + si, _sortedView.end());
            std::sort(_sortedView.begin(), _sortedView.end());

            auto count = std::min((int) pageSize(), (int) _sortedView.size() - offset);
            auto cit = _currentView.begin();
            for (int i = offset; i < offset + count; i++) {
                if (cit == _currentView.end()) {
                    _currentView.push_back(_sortedView[i]);
                    cit = _currentView.end();
                    continue;
                }
                auto citem = *cit;
                if (citem == _sortedView[i]) {
                    cit++;
                    continue;
                }
                (*cit) = _sortedView[i];
                cit++;
            }
            if (cit !=
                _currentView.end()) { //there are more elements in the view than are available for the current page
                for (auto cbt = cit; cbt != _currentView.end(); cbt++)
                    (*cbt)->_widget->undraw();
                _currentView.erase(cit, _currentView.end());
            }
        }

        void undraw() override {
            fb->draw_rect(this->x, this->y, this->w, this->h, WHITE, true);
            RoundCornerWidget::undraw();
        }

        void render() override {
            undraw();
            refresh_list();
            int sx = this->x + padding;
            int sy = this->y + padding;
            for (auto item: _currentView) {
                auto wi = item->_widget;
                wi->x = sx;
                wi->y = sy;
                wi->h = itemHeight;
                wi->w = w - padding - padding;
                wi->on_reflow();
                if (item->_selected) {
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
            if (index >= _currentView.size()) {
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
                if (multiSelect)
                    selectedItems.emplace(item);
                else {
                    for (const auto &si: selectedItems) {
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
        void on_mouse_click(input::SynMotionEvent &ev) override {
            auto hgt = itemHeight + padding;
            auto sy = ev.y - this->y;
            auto shgt = sy / hgt;
            int idx = floor(shgt);
            //printf("Click at %d,%d: computed offset %d: displayed %d\n", ev.x, ev.y, idx, displayed_items());
            //std::cout<<std::endl;
            if (idx > displayed_items())
                return;
            selectIndex(idx);
            mark_redraw();
        }

    protected:
        std::vector<shared_ptr<ListItem>> _currentView;
        std::vector<shared_ptr<ListItem>> _sortedView;

        int displayed_items() {
            return min((int) pageSize(), (int) contents.size() - offset);
        }

    private:
        static bool dummy_filter(shared_ptr<ListItem> it) {
            return true;
        }
    };
}
