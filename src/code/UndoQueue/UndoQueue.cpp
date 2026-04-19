#include "UndoQueue.h"

std::size_t UndoQueue::undoSize()
{
    return undoQueue.size();
}

std::size_t UndoQueue::redoSize()
{
    return redoStack.size();
}

void UndoQueue::addEvent(std::shared_ptr<UndoNode> &&node)
{
    if (undoQueue.size() == maxLength)
    {
        undoQueue.pop_front();
    }
    undoQueue.push_back(std::move(node));
    redoStack = std::stack<std::shared_ptr<UndoNode>>();
}

void UndoQueue::popUndoEvent()
{
    if (undoQueue.size() == 0)
    {
        throw std::runtime_error("Not enough items in undo queue for a pop!");
    }
    redoStack.push(undoQueue.back());
    undoQueue.pop_back();
}

void UndoQueue::popRedoEvent()
{
    if (redoStack.size() == 0)
    {
        throw std::runtime_error("Not enough items in redo stack for a pop!");
    }
    undoQueue.push_back(redoStack.top());
    redoStack.pop();
}

std::shared_ptr<UndoNode> UndoQueue::backUndoEvent()
{
    return undoQueue.back();
}

std::shared_ptr<UndoNode> UndoQueue::backRedoEvent()
{
    return redoStack.top();
}