#include "UndoNode.h"

InsertEvent::~InsertEvent() {}
EraseEvent::~EraseEvent() {}
TagEvent::~TagEvent() {}
UntagEvent::~UntagEvent() {}

std::unique_ptr<UndoNode> InsertEvent::getOppositeEvent()
{
    return std::make_unique<EraseEvent>(insertionPageIndex, insertionOffset, text);
}

std::unique_ptr<UndoNode> EraseEvent::getOppositeEvent()
{
    return std::make_unique<InsertEvent>(deletionPageIndex, deletionOffset, text);
}

std::unique_ptr<UndoNode> TagEvent::getOppositeEvent()
{
    return std::make_unique<UntagEvent>(tag, tagPageIndex, tagStartOffset, tagEndOffset);
}

std::unique_ptr<UndoNode> UntagEvent::getOppositeEvent()
{
    return std::make_unique<TagEvent>(tag, tagPageIndex, tagStartOffset, tagEndOffset);
}
