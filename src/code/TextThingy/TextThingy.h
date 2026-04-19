#pragma once
#include "IndexedTextView.h"
#include "ScrollingPageWrapper.h"
#include "UndoQueue.h"

#include <gtkmm-4.0/gtkmm.h>
#include <iostream>

class TextThingy;

using TagPtr = Glib::RefPtr<Gtk::TextTag>;
using BufferPtr = Glib::RefPtr<Gtk::TextBuffer>;
using BufferIt = Gtk::TextBuffer::iterator;
using PagePtr = std::shared_ptr<IndexedTextView>;

/**
 * @class PageInterface
 * @brief Class responsible for the text behaviour (across pages if turned on)
 *
 * Reacts to all text-related events, like insertion, erasure, etc.
 * Must be given a reference to TextThingy, since it's basically the frontend to this backend. It calls some
 * important functions, such as "AddPage" if needed. Implementation without this necessity is in the works.
 *
 */
class PageInterface
{
public:
    /**
     * @class PageOffset
     * @brief Used for handling offsets in text of a concrete page.
     *
     * You can think of it as a structure pointing to a specific place in the document
     */
    struct PageOffset
    {
        /**
         * @brief The index of a page
         */
        std::size_t pageIndex;
        /**
         * @brief Offset from the beginning **of the page**
         */
        std::size_t offset;
        /**
         * @brief The default contructor for PageOffset
         */
        PageOffset() : pageIndex(0), offset(0) {}
        /**
         * @brief The contructor for PageOffset
         *
         * @param _pageIndex The index of a page
         * @param _offset Offset from the beginning **of the page**
         */
        PageOffset(std::size_t _pageIndex, std::size_t _offset) : pageIndex(_pageIndex), offset(_offset) {}
    };
    /**
     * @brief The contructor for PageInterface
     *
     * @param parent_ The TextThingy this must be a member of
     * @param startingText The text that the pages should start with
     */
    PageInterface(TextThingy &parent_, std::string startingText);
    /**
     * @brief Returns the number of pages
     * @returns The number of pages
     */
    std::size_t GetPageCount() const noexcept;
    /**
     * @brief Returns the contents of a specific page
     *
     * @param pageIndex The index of the page to return the contents of.
     * @returns the contents of a specified page
     */
    std::string GetPageContents(std::size_t pageIndex) const;
    /**
     * @brief Returns the last page.
     *
     * @returns The last page.
     */
    std::shared_ptr<ScrollingPageWrapper> GetLastPage() const;
    /**
     * @brief Tags the selected text, if any.
     * @param tagName Name of the tag to be applied
     */
    void TagSelectedText(const std::string &tagName);

private:
    void addPage();
    void addPageBuffers();
    void removePage(std::size_t index);
    void removeLastPage();
    void fixIndicesFrom(std::size_t index);
    void moveRelevantTextTags(const BufferIt &textStartBufferFrom,
                              const BufferIt &textStartBufferTo,
                              std::size_t textLength);

    void before_apply_tag(const TagPtr &tag, const BufferIt &start, const BufferIt &end, PagePtr pagePtr, std::shared_ptr<bool> isMachineTag);
    void before_remove_tag(const TagPtr &tag, const BufferIt &start, const BufferIt &end, PagePtr pagePtr, std::shared_ptr<bool> isMachineUntag);
    void on_insert(BufferIt &pos, const Glib::ustring &text, int bytes, PagePtr pagePtr, std::shared_ptr<bool> isMachineInsert);
    void on_overflow(std::size_t pageIndex);
    bool on_key_pressed(guint keyval, guint keycode, Gdk::ModifierType state, PagePtr pagePtr);
    bool handleArrowPressLag(guint keyval, const PagePtr &pagePtr);
    void before_erase(BufferIt &range_start, BufferIt &range_end, PagePtr pagePtr, std::shared_ptr<bool> isMachineErase, std::shared_ptr<bool> eraseWithoutPageAlign);
    void after_erase(BufferIt &range_start, BufferIt &range_end, PagePtr pagePtr, std::shared_ptr<bool> eraseWithoutPageAlign);
    void on_clicked(int n_press, double x, double y, PagePtr pagePtr);

    void setFillInText(const BufferIt &range_start, const BufferIt &range_end, PagePtr &pagePtr);

    void createDefaultTagTable();
    int getActivePageIndex();
    void multipageErase(PageOffset start, PageOffset end);
    bool undo();
    bool redo();
    void undoInsert(const std::shared_ptr<InsertEvent> &);
    void undoErase(const std::shared_ptr<EraseEvent> &);
    void undoTag(const std::shared_ptr<TagEvent> &);
    void undoUntag(const std::shared_ptr<UntagEvent> &);
    bool undoEvent(std::shared_ptr<UndoNode>);

    std::vector<std::shared_ptr<ScrollingPageWrapper>> pages;
    Glib::RefPtr<Gtk::TextTagTable> defaultTagTable;
    TextThingy &owner;

    std::string eraseFillInText;
    std::shared_ptr<bool> userIsInserting;
    std::shared_ptr<UndoQueue> undoQueue;
};

/**
 * @class TextThingy
 * @brief Holds and presents the pages of the document
 *
 */
class TextThingy : public Gtk::ScrolledWindow
{
public:
    /**
     * @brief The default contructor for PageInterface
     *
     * Defaults starting text to empty string
     */
    TextThingy();
    /**
     * @brief The contructor for PageInterface
     *
     * @param startingText The text that the pages should start with
     */
    TextThingy(std::string startingText);
    /**
     * @brief The width of every page.
     */
    const static int pageWidth = 500;
    /**
     * @brief The height of every page, calculated so it's A4 format.
     */
    const static int pageHeight = pageWidth * 1415 / 1000;
    /**
     * @brief The height of empty spaces after every page.
     */
    const static int pageDeviderHeight = pageHeight * 20 / 100;

    /**
     * @brief Method to be called on every save.
     */
    void onSave();
    /**
     * @brief Returns if the pages have been in any way edited.
     * @returns True if pages have been edited.
     */
    bool getEdited();
    /**
     * @brief Sets the indicator that the pages have been edited (for getEdited).
     */
    void setEdited();
    /**
     * @brief Adds an empty page.
     */
    void addPage();
    /**
     * @brief Removes last page.
     */
    void removeLastPage();
    /**
     * @brief Removes the index-th page.
     * @param index The index of the page to be deleted.
     */
    void removePage(std::size_t index);
    /**
     * @brief Returns all of the contained text
     * Currently slightly incorrect, should return GLib::ustring or ideally Gtk::TextView
     * @returns All contained text
     */
    std::string getAllText();
    /**
     * @brief The underlaying page interface, responsible for handling all of the operation on page contents
     */
    std::shared_ptr<PageInterface> pageInterface;

private:
    struct Coordinates
    {
        double x;
        double y;
        Coordinates() : x(0), y(0) {}
        Coordinates(double _x, double _y) : x(_x), y(_y) {}
    };
    struct CoordinatesStatic
    {
        TextThingy *self;
        Coordinates coords;
        CoordinatesStatic(double _x, double _y, TextThingy *self_) : self(self_)
        {
            coords = Coordinates(_x, _y);
        }
    };
    void on_scroll();
    std::size_t vScrollToPage(Glib::RefPtr<Gtk::Adjustment> adjustment);

    void addFixedOverlay();
    void initPages();
    void putImageToPage(int pageNum, double x, double y);
    static void staticTryImportImage(gpointer data, gpointer user_data);
    void tryImportImage(gpointer data, gpointer user_data);

    void setupTextArea();
    static void printFile(gpointer data, gpointer user_data);
    void dropHandler();
    bool handleFileDrop(const Glib::ValueBase &value, double x, double y);

    Gtk::Fixed overlay;
    Gtk::Box pages;
    bool edited;
};
