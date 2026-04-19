#pragma once

#include <gtkmm-4.0/gtkmm.h>

class IndexedTextView;

using BeforeTag = sigc::slot<void(const Glib::RefPtr<Gtk::TextTag> &, const Gtk::TextBuffer::iterator &, const Gtk::TextBuffer::iterator &, std::shared_ptr<IndexedTextView>, std::shared_ptr<bool>)>;
using BeforeUntag = sigc::slot<void(const Glib::RefPtr<Gtk::TextTag> &, const Gtk::TextBuffer::iterator &, const Gtk::TextBuffer::iterator &, std::shared_ptr<IndexedTextView>, std::shared_ptr<bool>)>;
using BeforeInsert = sigc::slot<void(Gtk::TextBuffer::iterator &, const Glib::ustring &, int, std::shared_ptr<IndexedTextView>, std::shared_ptr<bool>)>;
using BeforeErase = sigc::slot<void(Gtk::TextBuffer::iterator &, Gtk::TextBuffer::iterator &, std::shared_ptr<IndexedTextView>, std::shared_ptr<bool>, std::shared_ptr<bool>)>;
using AfterErase = sigc::slot<void(Gtk::TextBuffer::iterator &, Gtk::TextBuffer::iterator &, std::shared_ptr<IndexedTextView>, std::shared_ptr<bool>)>;
using AfterClicked = sigc::slot<void(int, double, double, std::shared_ptr<IndexedTextView>)>;
using BeforeKey = sigc::slot<bool(guint keyval, guint keycode, Gdk::ModifierType state, std::shared_ptr<IndexedTextView>)>;

/**
 * @class IndexedTextView
 * @brief Utility wrapper around Gtk::TextView with an index.
 *
 * This class adds some useful functions for Gtk::TextView as well as an index, so it can be used as a page.
 */
class IndexedTextView : public Gtk::TextView
{
public:
    /**
     * @class PageEditFlags
     * @brief Flags that change the behavious of event handlers
     *
     * Holds multiple boolean flags that can be used to modify the behavior of event handlers in IndexedTextView,
     * namely the insert, erase and tag events.
     */
    struct PageEditFlags
    {
        /**
         * @brief Constructor for PageEditFlags.
         */
        PageEditFlags() : eraseWithoutPageAlign(std::make_shared<bool>(false)),
                          isMachineErase(std::make_shared<bool>(false)),
                          isMachineInsert(std::make_shared<bool>(false)),
                          isMachineTag(std::make_shared<bool>(false)),
                          isMachineUntag(std::make_shared<bool>(false)) {}

        /**
         * @brief Constructor for PageEditFlags.
         * @param eraseWithoutPageAlign Flag indicating that upon erase, the alignement of pages should not be recalculated.
         * @param isMachineErase Flag indicating it was not the user that called the erase event. Useful for undo event handling.
         * @param isMachineInsert Flag indicating it was not the user that called the insert event. Useful for undo event handling.
         * @param isMachineTag Flag indicating it was not the user that called the tag event. Useful for undo event handling.
         * @param isMachineUntag Flag indicating it was not the user that called the untag event. Useful for undo event handling.
         */
        PageEditFlags(bool eraseWithoutPageAlign, bool isMachineErase,
                      bool isMachineInsert, bool isMachineTag,
                      bool isMachineUntag)
            : eraseWithoutPageAlign(std::make_shared<bool>(eraseWithoutPageAlign)),
              isMachineErase(std::make_shared<bool>(isMachineErase)),
              isMachineInsert(std::make_shared<bool>(isMachineInsert)),
              isMachineTag(std::make_shared<bool>(isMachineTag)),
              isMachineUntag(std::make_shared<bool>(isMachineUntag)) {}

        std::shared_ptr<bool> eraseWithoutPageAlign;
        std::shared_ptr<bool> isMachineErase;
        std::shared_ptr<bool> isMachineInsert;
        std::shared_ptr<bool> isMachineTag;
        std::shared_ptr<bool> isMachineUntag;
    };
    /**
     * @brief Constructor for IndexedTextView.
     * @param width The width of the TextView.
     * @param height The height of the TextView.
     * @param _userInsertionFlag Flag to run the insert event without running any handlers.
     * @param tagTable Gtk::TextTagTable for the TextView.
     */
    IndexedTextView(std::size_t i, int width, int height, std::shared_ptr<bool> _userInsertionFlag, Glib::RefPtr<Gtk::TextTagTable> tagTable);

    /**
     * @brief Sets the event handlers for the TextView events.
     * @param thisPage A shared_ptr to this object, is to be passed to the handlers.
     * @param slotBTag The handler to be run before the tag event.
     * @param slotBUntag The handler to be run before the untag event.
     * @param slotBErase The handler to be run before the erase event.
     * @param slotAErase The handler to be run after the erase event.
     * @param slotBInsert The handler to be run before the insert event.
     * @param slotBKey The handler to be run before the keypress event.
     * @param slotAClick The handler to be run after the click event.
     */
    void SetHandlers(std::shared_ptr<IndexedTextView> thisPage,
                     BeforeTag &slotBTag, BeforeUntag &slotBUntag,
                     BeforeErase &slotBErase, AfterErase &slotAErase,
                     BeforeInsert &slotBInsert, BeforeKey &slotBKey, AfterClicked &slotAClick);
    /**
     * @brief Runs the erase event with the `eraseWithoutPageAlign` flag on.
     *
     * Should be used for example if you want to handle the alignment yourself.
     *
     * @param range_start Gtk iterator pointing to the start of the erasure.
     * @param range_end Gtk iterator pointing to the end of the erasure.
     */
    void EraseWithoutPaging(const Gtk::TextBuffer::iterator &range_start, const Gtk::TextBuffer::iterator &range_end);
    /**
     * @brief Runs the erase event with the `isMachineErase` flag on.
     *
     * Should be used for example if you don't want the event to be undoable.
     *
     * @param range_start Gtk iterator pointing to the start of the erasure.
     * @param range_end Gtk iterator pointing to the end of the erasure.
     */
    void MachineErase(const Gtk::TextBuffer::iterator &range_start, const Gtk::TextBuffer::iterator &range_end);
    /**
     * @brief Runs the erase event with the `IsMachineInsert` flag on.
     *
     * Should be used for example if you don't want the event to be undoable.
     *
     * @param pos Gtk iterator pointing to insertion position.
     * @param range_end Text to be inserted.
     */
    void MachineInsert(const Gtk::TextBuffer::iterator &pos, const Glib::ustring &text);
    /**
     * @brief Runs the erase event with the `IsMachineTag` flag on.
     *
     * Should be used for example if you don't want the event to be undoable.
     *
     * @param tag Tag to be applied.
     * @param range_start Gtk iterator pointing to the start of the text to be tagged.
     * @param range_end Gtk iterator pointing to the end of the text to be tagged.
     */
    void MachineTag(const Glib::RefPtr<Gtk::TextTag> &tag, const Gtk::TextBuffer::iterator &range_start, const Gtk::TextBuffer::iterator &range_end);
    /**
     * @brief Runs the erase event with the `IsMachineUntag` flag on.
     *
     * Should be used for example if you don't want the event to be undoable.
     *
     * @param tag Tag to be removed.
     * @param range_start Gtk iterator pointing to the start of the text to be untagged.
     * @param range_end Gtk iterator pointing to the end of the text to be untagged.
     */
    void MachineUntag(const Glib::RefPtr<Gtk::TextTag> &tag, const Gtk::TextBuffer::iterator &range_start, const Gtk::TextBuffer::iterator &range_end);
    /**
     * @brief Runs the insert event Without triggering any handlers set
     *
     * Should be used with extreme care. Currently unused, but intended to be used in page overflow handler,
     * to be sure it runs only once.
     *
     * @param pos Gtk iterator pointing to insertion position.
     * @param range_end Text to be inserted.
     */
    void SilentInsert(const Gtk::TextBuffer::iterator &pos, const Glib::ustring &text);

    /**
     * @brief Checks if the range specified has the tag applied on the whole range
     *
     * @param startIt Gtk iterator pointing to the start of the text range.
     * @param endIt Gtk iterator pointing to the end of the text range.
     * @param tag Tag to be checked for.
     * @param skipSpaces Flag specifying if spaces count. For example, if you are checking for "red text" tag, you might want to consider the range red even when the spaces are "white".
     *
     * @returns true if the whole range is tagged.
     */
    bool IsRangeFullyTagged(const Gtk::TextIter &startIt, const Gtk::TextIter &endIt,
                            const Glib::RefPtr<Gtk::TextTag> &tag, bool skipSpaces = false);
    /**
     * @brief Gets a tag from the tag table by its name
     *
     * @param name The name of the looked for tag.
     *
     * @returns Tag with the specified name.
     */
    Glib::RefPtr<Gtk::TextTag> GetTagByName(const Glib::ustring &name);
    /**
     * @brief Tags the currently selected text, if any.
     *
     * @param tagName The name of the tag to apply.
     */
    void TagSelection(const Glib::ustring &tagName);
    /**
     * @brief Gets the object's index.
     *
     * @returns The object's index.
     */
    std::size_t GetIndex() const noexcept;
    /**
     * @brief Sets the object's index.
     * @param newIndex New index value.
     */
    void SetIndex(std::size_t newIndex) noexcept;
    /**
     * @brief Returns the flags object.
     *
     * All flags are pointers, so changing it's value will change the behaviour of events.
     * Use with care.
     *
     * @returns The flags object.
     */
    PageEditFlags GetFlags() const noexcept
    {
        return flags;
    }

private:
    bool tagChangesSpaces(const Glib::RefPtr<Gtk::TextTag> &tag);
    PageEditFlags flags;
    std::shared_ptr<bool> userInsertionFlag;
    sigc::connection on_insert_signal;
    std::size_t index;
};