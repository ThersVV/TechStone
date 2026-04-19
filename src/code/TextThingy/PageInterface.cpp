#include "TextThingy.h"

void print_tags(const BufferPtr &buffer)
{
    auto start = buffer->begin();
    auto end = buffer->end();

    while (start != end)
    {
        auto tags = start.get_tags();
        for (auto &tag : tags)
        {
            if (start.starts_tag(tag))
            {
                auto range_start = start;
                auto range_end = start;
                while (range_end != end && !range_end.ends_tag(tag))
                    ++range_end;

                std::cout << "Tag '" << tag->property_name()
                          << "' from offset " << range_start.get_offset()
                          << " to " << range_end.get_offset() << "\n";
            }
        }
        ++start;
    }
    std::cout << "\n\n";
}

void print_page_buffers(const std::vector<BufferPtr> &buffers)
{
    for (size_t i = 0; i < buffers.size(); ++i)
    {
        if (buffers[i])
        {
            std::cout << "Page " << i + 1 << ":\n"
                      << buffers[i]->get_text() << "\n\n";
            std::cout << "Tags: ";
            print_tags(buffers[i]);
        }
        else
        {
            std::cerr << "Page " << i + 1 << " is null!\n";
        }
    }
}

int getTextWidth(Glib::RefPtr<Pango::Layout> layout, const Glib::ustring &text)
{
    if (text.empty())
    {
        return 0;
    }
    // warning: this function is slow af
    layout->set_text(text);
    Pango::Rectangle ink_rect, logical_rect;
    layout->get_extents(ink_rect, logical_rect);
    return logical_rect.get_x() + logical_rect.get_width();
}

PageInterface::PageInterface(TextThingy &owner_, std::string startingText)
    : owner(owner_)
{
    userIsInserting = std::make_shared<bool>(false);
    undoQueue = std::make_shared<UndoQueue>(200);
    createDefaultTagTable();
    addPageBuffers();
    pages[0]->page->SilentInsert(pages[0]->page->get_buffer()->begin(), startingText);
}

std::size_t PageInterface::GetPageCount() const noexcept
{
    return pages.size();
}

std::string PageInterface::GetPageContents(std::size_t pageIndex) const
{
    if (pageIndex < pages.size())
    {
        return pages[pageIndex]->page->get_buffer()->get_text();
    }
    return "";
}

std::shared_ptr<ScrollingPageWrapper> PageInterface::GetLastPage() const
{
    return pages.back();
}

void PageInterface::TagSelectedText(const std::string &tagName)
{
    int activePageI = getActivePageIndex();
    if (activePageI != -1)
    {
        pages[activePageI]->page->TagSelection(tagName);
    }
}

void PageInterface::addPage()
{
    addPageBuffers();
    owner.addPage();
}

void PageInterface::addPageBuffers()
{
    const int w = owner.pageWidth;
    const int h = owner.pageHeight;

    const auto page = std::make_shared<IndexedTextView>(pages.size(), w, h, userIsInserting, defaultTagTable);
    BeforeTag tagFun = sigc::mem_fun(*this, &PageInterface::before_apply_tag);
    BeforeUntag untagFun = sigc::mem_fun(*this, &PageInterface::before_remove_tag);
    BeforeInsert insertFun = sigc::mem_fun(*this, &PageInterface::on_insert);
    BeforeKey keyFun = sigc::mem_fun(*this, &PageInterface::on_key_pressed);
    BeforeErase eraseFunB = sigc::mem_fun(*this, &PageInterface::before_erase);
    AfterErase eraseFunA = sigc::mem_fun(*this, &PageInterface::after_erase);
    AfterClicked clickedFun = sigc::mem_fun(*this, &PageInterface::on_clicked);

    page->SetHandlers(page, tagFun, untagFun, eraseFunB, eraseFunA, insertFun, keyFun, clickedFun);

    AfterOverflow overflowFun = sigc::mem_fun(*this, &PageInterface::on_overflow);
    pages.push_back(std::make_shared<ScrollingPageWrapper>(page, w, h, overflowFun));
}

void PageInterface::removePage(std::size_t index)
{
    if (index < pages.size())
    {
        // Maybe first shift buffers?
        pages.erase(pages.begin() + index);
        owner.removePage(index);
        fixIndicesFrom(index);
    }
}

void PageInterface::removeLastPage()
{
    if (!pages.empty())
    {
        pages.pop_back();
        owner.removeLastPage();
    }
}

void PageInterface::fixIndicesFrom(std::size_t index)
{
    for (std::size_t i = index; i < pages.size(); i++)
    {
        pages[i]->page->SetIndex(i);
    }
}

void PageInterface::moveRelevantTextTags(const BufferIt &textStartBufferFrom,
                                         const BufferIt &textStartBufferTo,
                                         const std::size_t textLength)
{
    // Call this after you insert text to the second buffer but before you erase it.
    // (This way you have more control about how you do these actions.)
    auto bufferTo = textStartBufferTo.get_buffer();
    auto iterLeft = textStartBufferFrom;
    auto iterRight = iterLeft;
    auto bufferToLeftIter = textStartBufferTo;
    auto bufferToRightIter = bufferToLeftIter;
    const auto endOffset = textStartBufferFrom.get_offset() + textLength;

    bool exitLoop = false;
    while (!exitLoop)
    {
        if (iterRight.get_offset() > endOffset || !iterRight.forward_to_tag_toggle())
        {
            exitLoop = true;
            iterRight.set_offset(endOffset);
        }

        bufferToRightIter.forward_chars(iterRight.get_offset() - iterLeft.get_offset());

        const auto tags = iterLeft.get_tags();
        for (const auto &tag : tags)
        {
            bufferTo->apply_tag(tag, bufferToLeftIter, bufferToRightIter);
        }

        iterLeft = iterRight;
        bufferToLeftIter = bufferToRightIter;
    }
}

void PageInterface::before_apply_tag(const TagPtr &tag, const BufferIt &start, const BufferIt &end, PagePtr pagePtr, std::shared_ptr<bool> isMachineTag)
{
    if (!*isMachineTag)
    {
        auto undoNode = std::make_shared<TagEvent>(tag, pagePtr->GetIndex(), start.get_offset(), end.get_offset());
        undoQueue->addEvent(undoNode);
    }
}

void PageInterface::before_remove_tag(const TagPtr &tag, const BufferIt &start, const BufferIt &end, PagePtr pagePtr, std::shared_ptr<bool> isMachineUntag)
{
    if (!*isMachineUntag)
    {
        auto undoNode = std::make_shared<UntagEvent>(tag, pagePtr->GetIndex(), start.get_offset(), end.get_offset());
        undoQueue->addEvent(undoNode);
    }
}

void PageInterface::on_insert(BufferIt &pos, const Glib::ustring &text, int bytes, PagePtr pagePtr, std::shared_ptr<bool> isMachineInsert)
{
    *userIsInserting = true;
    owner.setEdited();

    if (!*isMachineInsert)
    {
        auto textAsBuff = Gtk::TextBuffer::create(defaultTagTable);
        textAsBuff->set_text(text);
        auto undoNode = std::make_shared<InsertEvent>(pagePtr->GetIndex(), pos.get_offset(), textAsBuff);
        undoQueue->addEvent(undoNode);
    }
}

void PageInterface::on_overflow(std::size_t pageIndex)
{
    // Todo: handle special case when the overflow starts with \n
    BufferIt pageEndIter;
    pages[pageIndex]->page->get_iter_at_location(pageEndIter, 0, owner.pageHeight);

    const auto pagePtr = pages[pageIndex]->page;
    const auto insertionPageBuffer = pagePtr->get_buffer();

    if (pageEndIter == insertionPageBuffer->end())
    {
        // This sometimes happens, not sure why. Easy solution tho xd
        return;
    }
    if (pagePtr->GetIndex() == pages.size() - 1)
    {
        addPage();
    }

    pageEndIter.backward_char(); // ?
    std::string overflowingText = insertionPageBuffer->get_slice(pageEndIter, insertionPageBuffer->end());

    const int insertOffset = insertionPageBuffer->get_insert()->get_iter().get_offset();
    const int howMuchInsertedTextIsOverflowing = insertOffset - pageEndIter.get_offset();

    const auto nextPageBuffer = pages[pagePtr->GetIndex() + 1]->page->get_buffer();
    pages[pagePtr->GetIndex() + 1]->page->MachineInsert(nextPageBuffer->begin(), overflowingText);

    if (howMuchInsertedTextIsOverflowing > 0 && *userIsInserting)
    {
        pages[pagePtr->GetIndex() + 1]->page->grab_focus();
        nextPageBuffer->place_cursor(nextPageBuffer->get_iter_at_offset(howMuchInsertedTextIsOverflowing));
    }
    else
    {
        pagePtr->grab_focus();
        insertionPageBuffer->place_cursor(insertionPageBuffer->get_iter_at_offset(insertOffset));
    }
    moveRelevantTextTags(pageEndIter, nextPageBuffer->begin(), overflowingText.length());
    pages[pagePtr->GetIndex()]->page->EraseWithoutPaging(pageEndIter, insertionPageBuffer->end());
}

bool PageInterface::on_key_pressed(guint keyval, guint keycode, Gdk::ModifierType state, PagePtr pagePtr)
{
    const auto currentBuffer = pagePtr->get_buffer();
    const int cursorPos = currentBuffer->get_iter_at_mark(currentBuffer->get_insert()).get_offset();

    if (keyval == GDK_KEY_BackSpace && cursorPos == 0 && !currentBuffer->get_has_selection())
    {
        auto index = pagePtr->GetIndex();
        if (index > 0)
        {
            // Switch pages
            pages[index - 1]->page->grab_focus();
            const auto prevPageBuffer = pages[index - 1]->page->get_buffer();
            prevPageBuffer->place_cursor(prevPageBuffer->end());
            // Delete last page if necessary
            if (pages.back()->page->get_buffer()->get_char_count() == 0)
            {
                removeLastPage();
            }
        }
        return true;
    }

    if (keyval == GDK_KEY_Left || keyval == GDK_KEY_Down ||
        keyval == GDK_KEY_Right || keyval == GDK_KEY_Up)
    {
        return handleArrowPressLag(keyval, pagePtr);
    }

    const bool ctrlMod = (state & Gdk::ModifierType::CONTROL_MASK) != Gdk::ModifierType::NO_MODIFIER_MASK;
    if (ctrlMod && keyval == GDK_KEY_z)
    {
        return undo();
    }
    if (ctrlMod && keyval == GDK_KEY_y)
    {
        return redo();
    }

    return false;
}

bool PageInterface::handleArrowPressLag(guint keyval, const PagePtr &pagePtr)
{
    const bool isForward = keyval == GDK_KEY_Right || keyval == GDK_KEY_Down;
    const auto currentBuffer = pagePtr->get_buffer();
    const auto cursor = currentBuffer->get_iter_at_mark(currentBuffer->get_insert());

    if (isForward && cursor == currentBuffer->end())
    {
        return true;
    }
    else if (!isForward && cursor == currentBuffer->begin())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void PageInterface::before_erase(BufferIt &range_start, BufferIt &range_end, PagePtr pagePtr, std::shared_ptr<bool> isMachineErase, std::shared_ptr<bool> eraseWithoutPageAlign)
{
    if (*eraseWithoutPageAlign)
    {
        return;
    }
    owner.setEdited();
    if (pagePtr->GetIndex() != pages.size() - 1)
    {
        setFillInText(range_start, range_end, pagePtr);
    }

    if (!*isMachineErase)
    {
        auto text = Gtk::TextBuffer::create(defaultTagTable);
        text->set_text(pagePtr->get_buffer()->get_slice(range_start, range_end));

        auto const eraseTextLen = range_end.get_offset() - range_start.get_offset();
        moveRelevantTextTags(range_start, text->begin(), eraseTextLen);

        auto undoNode = std::make_shared<EraseEvent>(pagePtr->GetIndex(), range_start.get_offset(), text);
        undoQueue->addEvent(undoNode);
    }
}

void PageInterface::after_erase(BufferIt &range_start, BufferIt &range_end, PagePtr pagePtr, std::shared_ptr<bool> eraseWithoutPageAlign)
{
    if (*eraseWithoutPageAlign)
    {
        return;
    }
    const auto erasePageBuffer = pagePtr->get_buffer();
    const auto index = pagePtr->GetIndex();

    // Flow in text from the next page
    if (index != pages.size() - 1)
    {
        const int rangeStartOffset = range_start.get_offset();

        const auto erasePageEndOffset = erasePageBuffer->end().get_offset();
        pagePtr->SilentInsert(erasePageBuffer->end(), eraseFillInText);

        const auto nextPageBuffer = pages[index + 1]->page->get_buffer();
        moveRelevantTextTags(nextPageBuffer->begin(), erasePageBuffer->get_iter_at_offset(erasePageEndOffset),
                             eraseFillInText.length());

        nextPageBuffer->erase(nextPageBuffer->begin(), nextPageBuffer->get_iter_at_offset(eraseFillInText.length()));

        // This looks dumb, but range_start iterator is not valid
        const auto iterAtDeletion = erasePageBuffer->get_iter_at_offset(rangeStartOffset);
        erasePageBuffer->place_cursor(iterAtDeletion);

        // erase last page if necessary (todo: is this even possible?)
        if (pages.back()->page->get_buffer()->get_char_count() == 0)
        {
            removeLastPage();
        }
    }
    return;
}

void PageInterface::on_clicked(int n_press, double x, double y, PagePtr pagePtr)
{
    // print_page_buffers(buffersToShift);
}

void PageInterface::setFillInText(const BufferIt &range_start, const BufferIt &range_end, PagePtr &pagePtr)
{
    // todo: Optimise this, i can usually guess that if I delete 100 chars, 1 wont be enough (not always!!!)
    const auto buffer = pagePtr->get_buffer();
    const auto index = pagePtr->GetIndex();
    BufferPtr backupBuffer = pages[index + 1]->page->get_buffer();

    auto layout = Pango::Layout::create(pagePtr->get_pango_context());

    // For some reason this measurement is not precise, so we compensate a bit
    const int erase_text_width = getTextWidth(layout, buffer->get_text(range_start, range_end, false)) * 105 / 100;

    Glib::ustring replacement_text = "";
    int width = getTextWidth(layout, replacement_text);
    for (auto leftIter = backupBuffer->begin(); leftIter != backupBuffer->end() && (width < erase_text_width);)
    {
        auto rightIter = leftIter;
        bool spaceFound = rightIter.forward_find_char([](gunichar c)
                                                      { return g_unichar_isspace(c); });
        if (!spaceFound)
        {
            replacement_text += backupBuffer->get_text(leftIter, backupBuffer->end(), false);
            break;
        }
        replacement_text += backupBuffer->get_text(leftIter, rightIter, false);
        width = getTextWidth(layout, replacement_text);

        leftIter = rightIter;
    }
    eraseFillInText = replacement_text;
}

void PageInterface::createDefaultTagTable()
{
    defaultTagTable = Gtk::TextTagTable::create();

    const auto redTag = Gtk::TextTag::create("red");
    redTag->property_foreground() = "red";
    defaultTagTable->add(redTag);

    const auto boldTag = Gtk::TextTag::create("bold");
    boldTag->property_weight() = Pango::Weight::BOLD;
    defaultTagTable->add(boldTag);

    const auto italicsTag = Gtk::TextTag::create("italics");
    italicsTag->property_style() = Pango::Style::ITALIC;
    defaultTagTable->add(italicsTag);

    const auto underlinedTag = Gtk::TextTag::create("underlined");
    underlinedTag->property_underline() = Pango::Underline::SINGLE;
    defaultTagTable->add(underlinedTag);
}

int PageInterface::getActivePageIndex()
{
    int activePageI = -1;
    for (std::size_t i = 0; i < pages.size(); i++)
    {
        if (pages[i]->page->has_focus() || pages[i]->page->is_focus())
        {
            activePageI = i;
            break;
        }
    }
    return activePageI;
}

void PageInterface::multipageErase(PageOffset start, PageOffset end)
{
    if (end.pageIndex > pages.size())
    {
        throw std::runtime_error("Cannot erase from imaginary pages!");
    }
    auto endPageBuff = pages[end.pageIndex]->page->get_buffer();
    auto startPageBuff = pages[start.pageIndex]->page->get_buffer();
    if (end.pageIndex == start.pageIndex)
    {
        pages[end.pageIndex]->page->MachineErase(startPageBuff->get_iter_at_offset(start.offset),
                                                 endPageBuff->get_iter_at_offset(end.offset));
        return;
    }
    while (end.pageIndex - start.pageIndex > 1)
    {
        removePage(start.pageIndex + 1);
        // removePage fixes indices
        end.pageIndex--;
    }
    pages[end.pageIndex]->page->MachineErase(endPageBuff->begin(), endPageBuff->get_iter_at_offset(end.offset));
    pages[start.pageIndex]->page->MachineErase(startPageBuff->get_iter_at_offset(start.offset), startPageBuff->end());
}

bool PageInterface::undo()
{
    if (undoQueue->undoSize() < 1)
    {
        return true;
    }
    auto lastEdit = undoQueue->backUndoEvent();
    if (undoEvent(lastEdit))
    {
        undoQueue->popUndoEvent();
        return true;
    }
    return false;
}

bool PageInterface::redo()
{
    if (undoQueue->redoSize() < 1)
    {
        return true;
    }
    auto lastUndo = undoQueue->backRedoEvent();
    if (undoEvent(lastUndo->getOppositeEvent()))
    {
        undoQueue->popRedoEvent();
        return true;
    }
    return false;
}

void PageInterface::undoInsert(const std::shared_ptr<InsertEvent> &insertPtr)
{
    const auto pageIndex = insertPtr->insertionPageIndex;
    const PageOffset start(pageIndex, insertPtr->insertionOffset);
    const auto remainingAtInsertPage = pages[pageIndex]->page->get_buffer()->get_char_count() - insertPtr->insertionOffset;
    auto overflowingLen = insertPtr->text->get_char_count() - remainingAtInsertPage;
    auto endPageIndex = pageIndex;
    while (endPageIndex < pages.size() - 1 && overflowingLen > 0)
    {
        endPageIndex++;
        overflowingLen -= pages[endPageIndex]->page->get_buffer()->get_char_count();
    }
    overflowingLen += pages[endPageIndex]->page->get_buffer()->get_char_count();
    const PageOffset end(endPageIndex, overflowingLen);
    multipageErase(start, end);
}

void PageInterface::undoErase(const std::shared_ptr<EraseEvent> &erasePtr)
{
    auto insertPage = pages[erasePtr->deletionPageIndex]->page;
    auto iter = insertPage->get_buffer()->get_iter_at_offset(erasePtr->deletionOffset);
    insertPage->MachineInsert(iter, erasePtr->text->get_text());
    // again because it was disvalidated
    iter = insertPage->get_buffer()->get_iter_at_offset(erasePtr->deletionOffset);

    auto const temp = *insertPage->GetFlags().isMachineTag;
    *insertPage->GetFlags().isMachineTag = true;
    moveRelevantTextTags(erasePtr->text->begin(), iter, erasePtr->text->get_char_count());
    *insertPage->GetFlags().isMachineTag = temp;
}

void PageInterface::undoTag(const std::shared_ptr<TagEvent> &tagPtr)
{
    auto relevantPage = pages[tagPtr->tagPageIndex]->page;
    auto const startIter = relevantPage->get_buffer()->get_iter_at_offset(tagPtr->tagStartOffset);
    auto const endIter = relevantPage->get_buffer()->get_iter_at_offset(tagPtr->tagEndOffset);
    relevantPage->MachineUntag(tagPtr->tag, startIter, endIter);
}

void PageInterface::undoUntag(const std::shared_ptr<UntagEvent> &untagPtr)
{
    auto relevantPage = pages[untagPtr->tagPageIndex]->page;
    auto const startIter = relevantPage->get_buffer()->get_iter_at_offset(untagPtr->tagStartOffset);
    auto const endIter = relevantPage->get_buffer()->get_iter_at_offset(untagPtr->tagEndOffset);
    relevantPage->MachineTag(untagPtr->tag, startIter, endIter);
}

bool PageInterface::undoEvent(std::shared_ptr<UndoNode> event)
{
    // returns true if matched (and handled).

    if (auto insertPtr = std::dynamic_pointer_cast<InsertEvent>(event))
    {
        undoInsert(insertPtr);
        return true;
    }
    else if (auto erasePtr = std::dynamic_pointer_cast<EraseEvent>(event))
    {
        undoErase(erasePtr);
        return true;
    }
    else if (auto tagPtr = std::dynamic_pointer_cast<TagEvent>(event))
    {
        undoTag(tagPtr);
        return true;
    }
    else if (auto untagPtr = std::dynamic_pointer_cast<UntagEvent>(event))
    {
        undoUntag(untagPtr);
        return true;
    }
    return false;
}