#include "IndexedTextView.h"
#include <iostream>

IndexedTextView::IndexedTextView(std::size_t i, int width, int height, std::shared_ptr<bool> _userInsertionFlag, Glib::RefPtr<Gtk::TextTagTable> tagTable)
    : index(i), userInsertionFlag(_userInsertionFlag)
{
    set_wrap_mode(Gtk::WrapMode::WORD_CHAR);
    set_size_request(width, height);
    set_expand(false);
    auto buffer = Gtk::TextBuffer::create(tagTable);
    set_buffer(buffer);
}

void IndexedTextView::SetHandlers(std::shared_ptr<IndexedTextView> thisPage,
                                  BeforeTag &slotBTag, BeforeUntag &slotBUntag,
                                  BeforeErase &slotBErase, AfterErase &slotAErase,
                                  BeforeInsert &slotBInsert, BeforeKey &slotBKey, AfterClicked &slotAClick)
{
    on_insert_signal = get_buffer()->signal_insert().connect(sigc::bind(slotBInsert, thisPage, flags.isMachineInsert), false);

    auto mouseController = Gtk::GestureClick::create();
    mouseController->signal_pressed().connect(sigc::bind(slotAClick, thisPage), true);
    add_controller(mouseController);

    auto keyController = Gtk::EventControllerKey::create();
    keyController->signal_key_pressed().connect(sigc::bind(slotBKey, thisPage), false);
    add_controller(keyController);

    get_buffer()->signal_erase().connect(sigc::bind(slotBErase, thisPage, flags.isMachineErase, flags.eraseWithoutPageAlign), false);
    get_buffer()->signal_erase().connect(sigc::bind(slotAErase, thisPage, flags.eraseWithoutPageAlign), true);

    get_buffer()->signal_apply_tag().connect(sigc::bind(slotBTag, thisPage, flags.isMachineTag), false);
    get_buffer()->signal_remove_tag().connect(sigc::bind(slotBUntag, thisPage, flags.isMachineUntag), false);
}

void IndexedTextView::SilentInsert(const Gtk::TextBuffer::iterator &pos, const Glib::ustring &text)
{
    *userInsertionFlag = false;
    on_insert_signal.block();
    get_buffer()->insert(pos, text);
    on_insert_signal.unblock();
}

void IndexedTextView::EraseWithoutPaging(const Gtk::TextBuffer::iterator &range_start, const Gtk::TextBuffer::iterator &range_end)
{
    const bool eraseWithoutPageAlignTemp = *flags.eraseWithoutPageAlign;
    *flags.eraseWithoutPageAlign = true;
    MachineErase(range_start, range_end);
    *flags.eraseWithoutPageAlign = eraseWithoutPageAlignTemp;
}

void IndexedTextView::MachineErase(const Gtk::TextBuffer::iterator &range_start, const Gtk::TextBuffer::iterator &range_end)
{
    const bool machineEraseTemp = *flags.isMachineErase;
    *flags.isMachineErase = true;
    get_buffer()->erase(range_start, range_end);
    *flags.isMachineErase = machineEraseTemp;
}

void IndexedTextView::MachineInsert(const Gtk::TextBuffer::iterator &pos, const Glib::ustring &text)
{
    const bool machineInsertTemp = *flags.isMachineInsert;
    *flags.isMachineInsert = true;
    get_buffer()->insert(pos, text);
    *flags.isMachineInsert = machineInsertTemp;
}

void IndexedTextView::MachineTag(const Glib::RefPtr<Gtk::TextTag> &tag, const Gtk::TextBuffer::iterator &range_start, const Gtk::TextBuffer::iterator &range_end)
{
    const bool machineTagTemp = *flags.isMachineTag;
    *flags.isMachineTag = true;
    get_buffer()->apply_tag(tag, range_start, range_end);
    *flags.isMachineTag = machineTagTemp;
}

void IndexedTextView::MachineUntag(const Glib::RefPtr<Gtk::TextTag> &tag, const Gtk::TextBuffer::iterator &range_start, const Gtk::TextBuffer::iterator &range_end)
{
    const bool machineUntagTemp = *flags.isMachineUntag;
    *flags.isMachineUntag = true;
    get_buffer()->remove_tag(tag, range_start, range_end);
    *flags.isMachineUntag = machineUntagTemp;
}

Glib::RefPtr<Gtk::TextTag> IndexedTextView::GetTagByName(const Glib::ustring &name)
{
    return get_buffer()->get_tag_table()->lookup(name);
}

void IndexedTextView::TagSelection(const Glib::ustring &tagName)
{
    Gtk::TextBuffer::iterator selectionStart, selectionEnd;
    if (get_buffer()->get_selection_bounds(selectionStart, selectionEnd))
    {
        auto tag = GetTagByName(tagName);

        if (IsRangeFullyTagged(selectionStart, selectionEnd, tag, !tagChangesSpaces(tag)))
        {
            get_buffer()->remove_tag(tag, selectionStart, selectionEnd);
        }
        else
        {
            get_buffer()->apply_tag(tag, selectionStart, selectionEnd);
        }
    }
}

bool IndexedTextView::IsRangeFullyTagged(const Gtk::TextIter &startIt, const Gtk::TextIter &endIt,
                                         const Glib::RefPtr<Gtk::TextTag> &tag, bool skipSpaces /* = false */)
{
    // Use skipSpaces for example when tagged space is identical to an untagged one

    auto iter = startIt;
    while (iter != endIt)
    {
        if (!iter.starts_tag(tag) && !iter.has_tag(tag) && !(skipSpaces && g_unichar_isspace(iter.get_char())))
        {
            return false;
        }

        iter.forward_char();
    }
    return true;
}

bool IndexedTextView::tagChangesSpaces(const Glib::RefPtr<Gtk::TextTag> &tag)
{
    if (tag->property_name() == "red")
    {
        return false;
    }
    return true;
}

std::size_t IndexedTextView::GetIndex() const noexcept
{
    return index;
}

void IndexedTextView::SetIndex(std::size_t newIndex) noexcept
{
    index = newIndex;
}
