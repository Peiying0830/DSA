#ifndef CANCEL_LOG_H
#define CANCEL_LOG_H

#include "Ticket.h"

#include <iostream>
#include <string>

using namespace std;

// Assignment Part B: Stack (LIFO) implementation.
// CancelLog implements a stack using an array.
// The most recent cancellation is placed at the top and can be undone first.
class CancelLog {
public:
    CancelLog();

    // Push a cancellation record onto the stack.
    bool recordCancel(const CancelEntry& entry);
    // Push without printing messages, mainly used when loading saved log data.
    bool recordCancelSilently(const CancelEntry& entry);
    // Pop the latest cancellation for undo.
    bool undoCancel(CancelEntry& entry);
    // Peek latest cancellation without removing it.
    bool peekLatest(CancelEntry& entry) const;
    // Display all cancellation records from newest to oldest.
    void showAllCancels() const;
    // Stack status and count helpers.
    bool isEmpty() const;
    bool isFull() const;
    int getCount() const;
    // Text file handling for cancellation log persistence.
    bool saveToFile(const string& fileName, bool showMessage) const;
    bool loadFromFile(const string& fileName, bool showMessage);
    // Reset the stack to empty.
    void clear();

private:
    // Fixed stack capacity for cancellation records.
    static const int MaxCapacity = 50;

    // Array stack storage and top pointer.
    CancelEntry entries[MaxCapacity];
    int topIndex;
};

#endif // CANCEL_LOG_H
