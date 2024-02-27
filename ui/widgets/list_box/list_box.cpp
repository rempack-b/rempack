//
// Created by brant on 2/5/24.
//

#include "list_box.h"
#include "buttons.h"
#include <unordered_set>
using namespace widgets;
     ListBox::ListBox(int x, int y, int w, int h, int itemHeight) : RoundCornerWidget(x, y, w, h, RoundCornerStyle()) {
         this->itemHeight = itemHeight;
         _pageLabel = make_shared<ui::Text>(0, 0, w, itemHeight, "");

         _navLL = make_shared<ImageButton>(0, 0, itemHeight, itemHeight, ICON(assets::png_fast_arrow_left_png));
         _navL = make_shared<ImageButton>(0, 0, itemHeight, itemHeight, ICON(assets::png_nav_arrow_left_png));
         _navR = make_shared<ImageButton>(0, 0, itemHeight, itemHeight, ICON(assets::png_nav_arrow_right_png));
         _navRR = make_shared<ImageButton>(0, 0, itemHeight, itemHeight, ICON(assets::png_fast_arrow_right_png));
         _navLL->hide();
         _navL->hide();
         _navR->hide();
         _navRR->hide();
         _pageLabel->hide();
         children.push_back(_navLL);
         children.push_back(_navL);
         children.push_back(_navR);
         children.push_back(_navRR);
         children.push_back(_pageLabel);

         _navLL->events.clicked += PLS_DELEGATE(LL_CLICK);
         _navL->events.clicked += PLS_DELEGATE(L_CLICK);
         _navR->events.clicked += PLS_DELEGATE(R_CLICK);
         _navRR->events.clicked += PLS_DELEGATE(RR_CLICK);
         layout_buttons();
     }

     shared_ptr<ListBox::ListItem> widgets::ListBox::add(const string &label, const any &object) {
         auto item = make_shared<ListItem>(label, object);
         item->_widget = make_shared<ui::Text>(x, y, w, itemHeight, label);
         //TODO: style sheets
         item->_widget->style.valign = ui::Style::MIDDLE;
         item->_widget->style.justify = ui::Style::LEFT;
         contents.push_back(item);
         events.added(item);
         this->mark_redraw();
         return item;
     }

     bool widgets::ListBox::remove(const string &label) {
         //sure, you could use std::find but C++ Lambdas are an affront to all that is good in this world
         int i = 0;
         shared_ptr<ListItem> item = nullptr;
         for (; i < (int) contents.size(); i++) {
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

void widgets::ListBox::on_reflow() {
    layout_buttons();
    trim_texts();
    mark_redraw();
}

void widgets::ListBox::removeAt(int index) {
    auto item = contents[index];
    auto w = item->_widget;
    contents.erase(contents.begin() + index);
    events.removed(item);
    mark_redraw();
}

void widgets::ListBox::undraw() {
    //fb->draw_rect(this->x, this->y, this->w, this->h, WHITE, true);
    RoundCornerWidget::render_inside_fill();
    RoundCornerWidget::undraw();
}

void widgets::ListBox::render() {
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

//check the Y position relative to top of widget, divide by itemHeight
void widgets::ListBox::on_mouse_click(input::SynMotionEvent &ev) {
    ev.stop_propagation();
    if (!selectable)
        return;
    auto hgt = itemHeight + padding;
    auto sy = ev.y - this->y;
    auto shgt = sy / hgt;
    int idx = floor(shgt);
    //printf("Click at %d,%d: computed offset %d: displayed %d\n", ev.x, ev.y, idx, displayed_items());
    if (idx >= (int) _currentView.size())
        return;
    selectIndex(idx);
    mark_redraw();
}

void widgets::ListBox::layout_buttons() {
    int bx = x + padding;
    int by = y + h - itemHeight - padding;
    stringstream lss;
    lss << "[ " << maxPages() << "/" << maxPages() << " ]";
    auto [lbx, lby] = utils::measure_string(lss.str(), ui::Widget::style.font_size);
    auto lbw = lbx;// + padding;
    _pageLabel->set_coords(bx, by, lbw, itemHeight);
    _pageLabel->style.valign = ui::Style::VALIGN::BOTTOM;
    bx += lbw + padding;
    //this width calculation is wrong for very narrow windows
    //I can't be bothered to fix it right now
    auto buttonWidth = ((w - lbw - padding - padding) / 5);
    if (maxPages() <= 2)
        buttonWidth *= 2;
    buttonWidth = min(buttonWidth, 200);
    buttonWidth = max(buttonWidth, itemHeight);
    if (maxPages() > 2) {
        _navLL->set_coords(bx, by, buttonWidth, itemHeight);
        bx += buttonWidth + padding;
    }
    _navL->set_coords(bx, by, buttonWidth, itemHeight);
    bx += buttonWidth + padding;
    _navR->set_coords(bx, by, buttonWidth, itemHeight);
    bx += buttonWidth + padding;
    if (maxPages() > 2)
        _navRR->set_coords(bx, by, buttonWidth, itemHeight);
    _pageLabel->on_reflow();
    _navLL->on_reflow();
    _navL->on_reflow();
    _navR->on_reflow();
    _navRR->on_reflow();
    _pageLabel->mark_redraw();
    _navLL->mark_redraw();
    _navL->mark_redraw();
    _navR->mark_redraw();
    _navRR->mark_redraw();
}

void widgets::ListBox::updateControlStates() {
    if (maxPages() == 1) {
        _navLL->hide();
        _navL->hide();
        _navR->hide();
        _navRR->hide();
        _pageLabel->hide();
        return;
    }
    if (maxPages() > 2) {
        _navLL->show();
        _navRR->show();
    } else {
        _navLL->hide();
        _navRR->hide();
    }
    _navL->show();
    _navR->show();
    _pageLabel->show();
    if (currentPage() == 1) {
        _navLL->disable();
        _navL->disable();
        _navR->enable();
        _navRR->enable();
    } else if (currentPage() == maxPages()) {
        _navLL->enable();
        _navL->enable();
        _navR->disable();
        _navRR->disable();
    } else {
        _navLL->enable();
        _navL->enable();
        _navR->enable();
        _navRR->enable();
    }
    layout_buttons();
}

void widgets::ListBox::updatePageDisplay() {
    stringstream ss;
    ss << "[ " << currentPage() << '/' << maxPages() << " ]";
    _pageLabel->text = ss.str();
    _pageLabel->mark_redraw();
    updateControlStates();
}

//first, filter contents with our predicate and copy to current view
//second, sort current view
void widgets::ListBox::refresh_list() {
    _sortedView.clear();
    for (auto &item: contents) {
        if (!filterPredicate || filterPredicate(item)) {
            _sortedView.push_back(item);
        }
    }

    if (sortPredicate)
        std::sort(_sortedView.begin(), _sortedView.end(), sortPredicate);
    else
        std::sort(_sortedView.begin(), _sortedView.end());

    auto offset = pageOffset * pageSize();
    auto count = std::min((int) pageSize(), (int) _sortedView.size() - offset);
    _currentView.clear();
    for (int i = offset; i < offset + count; i++) {
        _currentView.push_back(_sortedView[i]);
    }

    pageOffset = min(maxPages() - 1, pageOffset);

    updatePageDisplay();
}

void widgets::ListBox::selectIndex(int index) {
    if (index >= (int) _currentView.size()) {
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
                events.deselected(si);              // it is important to fire deselect events
            }                                       // |
            selectedItems.clear();                  // |
            selectedItems.emplace(item);            // |
        }                                           // V
        events.selected(item);                      // before firing select event
    }
}
