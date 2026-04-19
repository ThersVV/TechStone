#pragma once
#include "IndexedTextView.h"
#include <gtkmm-4.0/gtkmm.h>

class PageInterface;

using AfterOverflow = sigc::slot<void(std::size_t)>;
/**
 * @class ScrollingPageWrapper
 * @brief A wrapper around IndexedTextView.
 *
 * This wrapper adds the scrolling capability to the IndexedTextView. If there are multiple pages,
 * the change in the maximum scroll height is the event that indicates a page in overflowing.
 *
 * Inherits from Gtk::ScrolledWindow.
 */
class ScrollingPageWrapper : public Gtk::ScrolledWindow
{
public:
    /**
     * @brief Constructor for ScrollingPageWrapper.
     *
     * @param _page Pointer to the page this will wrap
     * @param width The width of the underlaying page
     * @param height The height of the underlaying page
     * @param _overflowHandler The handler to be called if the page overflows (currently unused, since it's buggy)
     */
    ScrollingPageWrapper(Glib::RefPtr<IndexedTextView> _page, int width, int height, AfterOverflow _overflowHandler);

    Glib::RefPtr<IndexedTextView> page;

private:
    void on_scroll();
    int setHeight;
    int setWidth;
    sigc::slot<void(std::size_t)> overflowHandler;
};