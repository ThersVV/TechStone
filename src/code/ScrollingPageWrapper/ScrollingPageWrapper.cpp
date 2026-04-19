#include "ScrollingPageWrapper.h"

ScrollingPageWrapper::ScrollingPageWrapper(Glib::RefPtr<IndexedTextView> _page, int width, int height, AfterOverflow _overflowHandler)
    : page(_page), setHeight(height), setWidth(width), overflowHandler(_overflowHandler)
{
    set_size_request(width, height);
    set_expand(false);
    set_child(*page);
    this->get_vscrollbar()->set_visible(false);
    this->get_vscrollbar()->get_adjustment()->signal_value_changed().connect(sigc::mem_fun(*this, &ScrollingPageWrapper::on_scroll), true);
    this->get_vscrollbar()->get_adjustment()->signal_changed().connect(sigc::mem_fun(*this, &ScrollingPageWrapper::on_scroll), true);
}

void ScrollingPageWrapper::on_scroll()
{
    auto adjustmentV = this->get_vscrollbar()->get_adjustment();
    auto adjustmentH = this->get_hscrollbar()->get_adjustment();
    // We check the horizontal adjustment to filter out bogus values that are set
    // on adjustment before actually rendering
    if (adjustmentV->get_upper() > setHeight && adjustmentH->get_upper() == setWidth)
    {
        /* Glib::signal_idle().connect_once([this]()
                                         { overflowHandler(page->index); }, G_PRIORITY_LOW); */
    }
}