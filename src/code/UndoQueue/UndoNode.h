#pragma once

#include <gtkmm-4.0/gtkmm.h>

/**
 * @class UndoNode
 * @brief Abstract class representing an action that can be undone and redone.
 *
 */
class UndoNode
{
public:
    /**
     * @brief Destructor for UndoNode.
     *
     */
    virtual ~UndoNode() = default;
    /**
     * @brief Returns the UndoNode that is the polar opposite of this one.
     *
     * If a specific event is undone, this method is used to get the event that is to be applied if the event is redone.
     *
     * @returns The opposite event.
     */
    virtual std::unique_ptr<UndoNode> getOppositeEvent() = 0;
};

/**
 * @class InsertEvent
 * @brief An UndoNode representing text insertion.
 *
 */
struct InsertEvent : public UndoNode
{
    /**
     * @brief Constructor for InsertEvent
     *
     * @param insertionPageIndex_ The (first) page where the insertion happened.
     * @param insertionOffset_ The offset at which the insertion happened.
     * @param text_ The text that was inserted.
     */
    InsertEvent(std::size_t insertionPageIndex_, int insertionOffset_, Glib::RefPtr<Gtk::TextBuffer> text_)
        : insertionPageIndex(insertionPageIndex_), insertionOffset(insertionOffset_), text(text_) {};
    /**
     * @brief Destructor for InsertEvent
     *
     */
    virtual ~InsertEvent();
    /**
     * @brief Returns the opposite event, in this override it's the EraseEvent.
     *
     * @returns EraseEvent that acts as the opposite to the object's insertion.
     */
    std::unique_ptr<UndoNode> getOppositeEvent() override;

    /**
     * @brief The (first) page where the insertion happened.
     */
    std::size_t insertionPageIndex;
    /**
     * @brief The offset at which the insertion happened.
     */
    int insertionOffset;
    /**
     * @brief The text that was inserted.
     */
    Glib::RefPtr<Gtk::TextBuffer> text;
};

/**
 * @class EraseEvent
 * @brief An UndoNode representing text erasure.
 *
 */
struct EraseEvent : public UndoNode
{
    /**
     * @brief Constructor for EraseEvent
     *
     * @param deletionPageIndex_ The (first) page where the deletion happened.
     * @param deletionOffset_ The offset at which the deletion happened.
     * @param text_ The text that was erased.
     */
    EraseEvent(std::size_t deletionPageIndex_, int deletionOffset_, Glib::RefPtr<Gtk::TextBuffer> text_)
        : deletionPageIndex(deletionPageIndex_), deletionOffset(deletionOffset_), text(text_) {};
    /**
     * @brief Destructor for EraseEvent
     *
     */
    virtual ~EraseEvent();
    /**
     * @brief Returns the opposite event, in this override it's the InsertEvent.
     *
     * @returns InsertEvent that acts as the opposite to the object's erasure.
     */
    std::unique_ptr<UndoNode> getOppositeEvent() override;

    /**
     * @brief The (first) page where the deletion happened.
     */
    std::size_t deletionPageIndex;
    /**
     * @brief The offset at which the deletion happened.
     */
    int deletionOffset;
    /**
     * @brief The text that was erased.
     *
     * Must remember all the text, because it's necessary to generate the opposite event
     */
    Glib::RefPtr<Gtk::TextBuffer> text;
};

/**
 * @class TagEvent
 * @brief An UndoNode representing the application of a tag.
 *
 */
struct TagEvent : public UndoNode
{ /**
   * @brief Constructor for TagEvent
   *
   * Currently only supports single page tag application.
   *
   * @param tag_ The tag that was applied.
   * @param tagPageIndex_ The page index at which the tag application happened.
   * @param tagStartOffset_ The offset of the start of tag application.
   * @param tagEndOffset_ The offset of the end of tag application.
   */
    TagEvent(Glib::RefPtr<Gtk::TextTag> tag_, std::size_t tagPageIndex_, int tagStartOffset_, int tagEndOffset_)
        : tag(tag_), tagPageIndex(tagPageIndex_), tagStartOffset(tagStartOffset_), tagEndOffset(tagEndOffset_) {};
    /**
     * @brief Destructor for TagEvent
     *
     */
    virtual ~TagEvent();
    /**
     * @brief Returns the opposite event, in this override it's the UntagEvent.
     *
     * @returns UntagEvent that acts as the opposite to the object's tag event.
     */
    std::unique_ptr<UndoNode> getOppositeEvent() override;
    /**
     * @brief The tag that was applied.
     *
     */
    Glib::RefPtr<Gtk::TextTag> tag;
    /**
     * @brief The page index at which the tag application happened.
     *
     */
    std::size_t tagPageIndex;
    /**
     * @brief The offset of the start of tag application.
     *
     */
    int tagStartOffset;
    /**
     * @brief The offset of the end of tag application.
     *
     */
    int tagEndOffset;
};

/**
 * @class UntagEvent
 * @brief An UndoNode representing the de-application of a tag.
 *
 */
struct UntagEvent : public UndoNode
{
    /**
     * @brief Constructor for UntagEvent
     *
     * Currently only supports single page tag de-application.
     *
     * @param tag_ The tag that was un-applied.
     * @param tagPageIndex_ The page index at which the tag de-application happened.
     * @param tagStartOffset_ The offset of the start of tag de-application.
     * @param tagEndOffset_ The offset of the end of tag de-application.
     */
    UntagEvent(Glib::RefPtr<Gtk::TextTag> tag_, std::size_t tagPageIndex_, int tagStartOffset_, int tagEndOffset_)
        : tag(tag_), tagPageIndex(tagPageIndex_), tagStartOffset(tagStartOffset_), tagEndOffset(tagEndOffset_) {};
    /**
     * @brief Destructor for UntagEvent
     *
     */
    virtual ~UntagEvent();
    /**
     * @brief Returns the opposite event, in this override it's the TagEvent.
     *
     * @returns TagEvent that acts as the opposite to the object's untag event.
     */
    std::unique_ptr<UndoNode> getOppositeEvent() override;
    /**
     * @brief The tag that was un-applied.
     *
     */
    Glib::RefPtr<Gtk::TextTag> tag;
    /**
     * @brief The page index at which the tag de-application happened.
     *
     */
    std::size_t tagPageIndex;
    /**
     * @brief The offset of the start of tag de-application.
     *
     */
    int tagStartOffset;
    /**
     * @brief The offset of the end of tag de-application.
     *
     */
    int tagEndOffset;
};

class ImageInsertEvent : public UndoNode
{
};
