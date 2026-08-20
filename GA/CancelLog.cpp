#include "CancelLog.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

using namespace std;

const int CancelLogWidth = 100;

// CancelLog.cpp implements Assignment Part B Stack using a fixed-size array.
// topIndex points to the latest cancellation record, which is the stack top.

void printCancelSeparator() {
    cout << string(CancelLogWidth, '-') << endl;
}

void printCancelTitle(const string& title) {
    int leftPadding = (CancelLogWidth - static_cast<int>(title.length())) / 2;
    if (leftPadding < 0) {
        leftPadding = 0;
    }
    cout << string(leftPadding, ' ') << title << endl;
}

void printCancelField(const string& label, const string& value) {
    cout << "  " << left << setw(14) << label << right << ": " << value << endl;
}

void printCancelEntryDetails(const CancelEntry& entry) {
    printCancelField("Ticket ID", to_string(entry.ticketID));
    printCancelField("Customer", entry.customerName);
    printCancelField("Reason", entry.note);
    printCancelField("Cancelled At", entry.cancelledAt);
}

string cleanCancelFileField(const string& text) {
    // The file format uses | as a column separator.
    // Replace user-entered | characters so one record stays on one row.
    string cleanedText = text;
    for (char& ch : cleanedText) {
        if (ch == '|') {
            ch = '/';
        }
    }
    return cleanedText;
}

CancelLog::CancelLog()
    : topIndex(-1) {
}

// Push a cancellation record onto the stack.
bool CancelLog::recordCancel(const CancelEntry& entry) {
    // Push is blocked when the array stack reaches MaxCapacity.
    if (isFull()) {
        cout << "Cannot record cancellation: cancel log is full." << endl;
        return false;
    }

    topIndex++;
    entries[topIndex] = entry;
    cout << "Cancellation recorded: Ticket #" << entry.ticketID << " at " << entry.cancelledAt << "." << endl;
    return true;
}

bool CancelLog::recordCancelSilently(const CancelEntry& entry) {
    // Used by file loading so old cancellation records can be restored without extra messages.
    if (isFull()) {
        return false;
    }

    // Push operation: move topIndex up, then store the new entry at the top.
    topIndex++;
    entries[topIndex] = entry;
    return true;
}

// Pop the latest cancellation record for undo operation.
bool CancelLog::undoCancel(CancelEntry& entry) {
    // Pop is not possible when the stack is empty.
    if (isEmpty()) {
        cout << "No cancellations to undo." << endl;
        return false;
    }

    entry = entries[topIndex];
    topIndex--;
    cout << "Undoing cancellation for Ticket #" << entry.ticketID << "." << endl;
    return true;
}

// View the latest cancellation without removing it from the stack.
bool CancelLog::peekLatest(CancelEntry& entry) const {
    // Peek shows the top stack element but does not reduce topIndex.
    if (isEmpty()) {
        cout << "No cancellations available to peek." << endl;
        return false;
    }

    // Pop operation: copy the top entry out, then move topIndex down.
    entry = entries[topIndex];
    cout << endl;
    printCancelSeparator();
    printCancelTitle("Latest Cancellation");
    printCancelSeparator();
    printCancelEntryDetails(entry);
    printCancelSeparator();
    return true;
}

// Display cancellation records from newest to oldest.
void CancelLog::showAllCancels() const {
    // Stack display starts from topIndex because the latest cancellation is shown first.
    if (isEmpty()) {
        cout << "Cancellation log is empty." << endl;
        return;
    }

    cout << endl;
    printCancelSeparator();
    printCancelTitle("Cancellation Records (Most Recent First)");
    printCancelSeparator();
    for (int index = topIndex; index >= 0; --index) {
        const CancelEntry& entry = entries[index];
        cout << endl;
        printCancelTitle("Cancellation " + to_string(topIndex - index + 1));
        printCancelEntryDetails(entry);
    }
    printCancelSeparator();
}

bool CancelLog::isEmpty() const {
    // topIndex below 0 means no stack element exists.
    return topIndex < 0;
}

bool CancelLog::isFull() const {
    // The array stack is full when topIndex reaches the last valid index.
    return topIndex >= MaxCapacity - 1;
}

int CancelLog::getCount() const {
    // Because topIndex starts at -1, count is always topIndex + 1.
    return topIndex + 1;
}

// Save cancellation stack records so cancellation history is kept after restarting the program.
bool CancelLog::saveToFile(const string& fileName, bool showMessage) const {
    // Save from bottom to top so loading can rebuild the same stack order.
    ofstream outputFile(fileName);
    if (!outputFile) {
        if (showMessage) {
            cout << "Unable to open cancellation log file for saving." << endl;
        }
        return false;
    }

    outputFile << "TicketID|CustomerName|Reason|CancelledAt" << endl;
    for (int index = 0; index <= topIndex; ++index) {
        const CancelEntry& entry = entries[index];
        outputFile << entry.ticketID << "|"
                   << cleanCancelFileField(entry.customerName) << "|"
                   << cleanCancelFileField(entry.note) << "|"
                   << cleanCancelFileField(entry.cancelledAt) << endl;
    }

    if (showMessage) {
        cout << "Cancellation log saved to " << fileName << "." << endl;
    }
    return true;
}

// Load saved cancellation records back into the array stack.
bool CancelLog::loadFromFile(const string& fileName, bool showMessage) {
    // Loading clears the existing stack first, then pushes each valid row.
    ifstream inputFile(fileName);
    if (!inputFile) {
        if (showMessage) {
            cout << "Unable to open cancellation log file for loading." << endl;
        }
        return false;
    }

    clear();
    string line;
    getline(inputFile, line);
    int loadedCount = 0;
    int skippedCount = 0;
    while (getline(inputFile, line)) {
        if (line.empty()) {
            continue;
        }

        stringstream lineStream(line);
        string idText;
        string customerName;
        string reason;
        string cancelledAt;
        string extraText;
        getline(lineStream, idText, '|');
        getline(lineStream, customerName, '|');
        getline(lineStream, reason, '|');
        getline(lineStream, cancelledAt, '|');
        bool hasExtraColumn = static_cast<bool>(getline(lineStream, extraText, '|'));

        stringstream idStream(idText);
        int ticketID = 0;
        bool invalidLine = !(idStream >> ticketID)
            || ticketID <= 0
            || !isValidText(customerName)
            || !isValidText(reason)
            || !isValidText(cancelledAt)
            || hasExtraColumn
            || isFull();

        if (invalidLine) {
            skippedCount++;
            continue;
        }

        topIndex++;
        entries[topIndex] = CancelEntry{ticketID, customerName, reason, cancelledAt};
        loadedCount++;
    }

    if (showMessage) {
        cout << "Loaded " << loadedCount << " cancellation record(s) from " << fileName << "." << endl;
    }
    if (showMessage && skippedCount > 0) {
        cout << "Skipped " << skippedCount << " invalid cancellation row(s)." << endl;
    }
    return true;
}

void CancelLog::clear() {
    // Clearing a stack only needs to reset the top pointer.
    topIndex = -1;
}
