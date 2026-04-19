#pragma once

#include "UndoNode.h"
#include <deque>
#include <gtkmm-4.0/gtkmm.h>
#include <memory>
#include <queue>
#include <stack>

/**
 * @class UndoQueue
 * @brief A structure keeping track of undo and redo events.
 */
class UndoQueue
{
public:
    /**
     * @brief Default constructor for UndoQueue.
     *
     * Initializes the structure with a default maximum length of 100.
     */
    UndoQueue() : maxLength(100) {}
    /**
     * @brief Constructor for UndoQueue
     *
     * @param historyLength The maximum number of undo events to keep in the queue.
     */
    UndoQueue(std::size_t historyLength) : maxLength(historyLength) {}

    /**
     * @brief Adds an event to the undo queue.
     *
     * If the queue is full, the oldest event is removed.
     *
     * @param node The event to be added.
     */
    void addEvent(std::shared_ptr<UndoNode> &&node);
    /**
     * @brief Returns the number of undo events tracked.
     *
     * @returns The size of the undo queue.
     */
    std::size_t undoSize();
    /**
     * @brief Returns the number of redo events tracked.
     *
     * @returns The size of the redo stack.
     */
    std::size_t redoSize();
    /**
     * @brief Removes the last undo event and pushes it to the redo stack.
     *
     * @throws std::runtime_error if there are no undo events to pop.
     */
    void popUndoEvent();
    /**
     * @brief Removes the last redo event and pushes it back to the undo queue.
     *
     * @throws std::runtime_error if there are no redo events to pop.
     */
    void popRedoEvent();
    /**
     * @brief Returns the last undo event without removing it.
     *
     * @returns The last undo event.
     */
    std::shared_ptr<UndoNode> backUndoEvent();
    /**
     * @brief Returns the last redo event without removing it.
     *
     * @returns The last redo event.
     */
    std::shared_ptr<UndoNode> backRedoEvent();

private:
    std::size_t maxLength;
    std::deque<std::shared_ptr<UndoNode>> undoQueue;
    std::stack<std::shared_ptr<UndoNode>> redoStack;
};