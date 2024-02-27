#pragma once

#include <unordered_set>
#include "widgets/widgets.h"
#include "buttons.h"

namespace widgets {
/*
 * Displays a list of items.
 * Provides sorting and filtering delegates to control displayed items, as well as
 * text clipping and pagination.
 * Features multiselect, uniselect, read only modes. Fires events when de/selected
 * and when items added/removed from contents.
 * Items are represented as a simple string with an attached std::any object which can store any user data.
 * Reflow is supported: list will be reformatted when the control is resized or moved.
 *
 * Presently only single-line items are supported; overflowing text will be trimmed.
 * Each item in the list must have a unique label.
 *
 * ___________________________
 * | Item 1                  |
 * | Item 2                  |
 * | Item 3 with a clipped la|
 * | [1/4] [<<] [<] [>] [>>] |
 * ---------------------------
 * [page number] [back 5/10] [back 1] [forward 1] [forward 5/10]
 * Fast nav buttons switch from 5 to 10 pages when there are more than 10 pages.
 * Keypad to enter page number would be nice
 */
    class ListBox : public widgets::RoundCornerWidget {
    public:
        struct ListItem {
            friend class ListBox;

            explicit ListItem(string label) : label(move(label)) {};          //label
            explicit ListItem(string label, any object) : label(move(label)), object(move(object)) {};
            string label;                //the text displayed in the listbox. May only be one line.
            any object;             //an optional pointer to any data you want to keep a reference to

            //all items must be unique. This sucks, but we'll implement it if someone needs it
            inline bool operator==(const ListItem &other) const {
                return this->label == other.label;
            }

            bool _selected = false;
        private:
            shared_ptr<ui::Text> _widget = nullptr;
        };

    public:
        PLS_DEFINE_SIGNAL(LISTBOX_EVENT, const shared_ptr<ListItem>);

        class LISTBOX_EVENTS {
        public:
            LISTBOX_EVENT selected;
            LISTBOX_EVENT deselected;
            LISTBOX_EVENT added;
            LISTBOX_EVENT removed;
        };

        LISTBOX_EVENTS events;

        std::function<bool(const shared_ptr<ListItem> &)> filterPredicate;
        std::function<bool(const shared_ptr<ListItem> &, shared_ptr<ListItem> &)> sortPredicate;

        bool selectable = true; //allow selecting of entries at all
        bool multiSelect = true; //allow selecting more than one entry

        int pageSize() {
            auto size = (int) floor(float(h - padding - padding) / float(itemHeight + padding));
            if (size < (int) _sortedView.size())   //if we have more items than will fit on one page,
                size--;                          //reserve at least one line of space at the bottom of the view for the nav elements
            return size;
        }

        int currentPage() const {
            return pageOffset + 1;
        }

        int maxPages() {
            return (int) ceil((float) _sortedView.size() / (float) pageSize());
        }

        //please call mark_redraw() on this widget after editing contents or selections
        vector<shared_ptr<ListItem>> contents;
        std::unordered_set<shared_ptr<ListItem>> selectedItems;

        ListBox(int x, int y, int w, int h, int itemHeight);

        ListBox(int x, int y, int w, int h, int itemHeight, const vector<string> &items) : ListBox(x, y, w, h, itemHeight) {
            for (const auto &s: items) {
                this->add(s);
            }
        }

        shared_ptr<ListItem> add(const string &label, const std::any &object = nullptr);

        bool remove(const string &label);

        void removeAt(int index);

        void on_reflow() override;

        void undraw() override;

        void render() override;

        void on_mouse_click(input::SynMotionEvent &ev) override;

    protected:
        std::vector<shared_ptr<ListItem>> _currentView;
        std::vector<shared_ptr<ListItem>> _sortedView;
        shared_ptr<ui::Text> _pageLabel;
        shared_ptr<ImageButton> _navLL, _navL, _navR, _navRR;

        void layout_buttons();

    private:
        //TODO: style sheets
        int itemHeight;
        int padding = 5;
        int pageOffset = 0;

        void updateControlStates();

        void updatePageDisplay();

        void LL_CLICK(void *v) {
            pageOffset = max(0, pageOffset - (maxPages() > 10 ? 10 : 5));
            updatePageDisplay();
            mark_redraw();
        }

        void L_CLICK(void *v) {
            pageOffset--;
            updatePageDisplay();
            mark_redraw();
        }

        void R_CLICK(void *v) {
            pageOffset++;
            updatePageDisplay();
            mark_redraw();
        }

        void RR_CLICK(void *v) {
            pageOffset = min(pageOffset + (maxPages() > 10 ? 10 : 5), maxPages() - 1);
            updatePageDisplay();
            mark_redraw();
        }

        void trim_texts() {
            for (const auto &it: contents) {
                auto wd = it->_widget;
                wd->text = utils::clip_string(it->label, wd->w, wd->h, ui::Widget::style.font_size);
                wd->mark_redraw();
            }
        }

        void refresh_list();

        void selectIndex(int index);


    };
}