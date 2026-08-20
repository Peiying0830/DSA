#include "WaitingLine.h"

#include <iomanip>
#include <sstream>

using namespace std;

const int WaitingLineWidth = 100;

// WaitingLine.cpp implements Assignment Part A Queue using a linked list.
// head represents the front of the queue, and tail represents the rear.

void printWaitingLineSeparator() {
    cout << string(WaitingLineWidth, '-') << endl;
}

void printWaitingLineTitle(const string& title) {
    int leftPadding = (WaitingLineWidth - static_cast<int>(title.length())) / 2;
    if (leftPadding < 0) {
        leftPadding = 0;
    }
    cout << string(leftPadding, ' ') << title << endl;
}

void printWaitingLineField(const string& label, const string& value) {
    cout << "  " << left << setw(12) << label << right << ": " << value << endl;
}

void printWaitingTicketDetails(const Ticket& ticket) {
    printWaitingLineField("Ticket ID", to_string(ticket.ticketID));
    printWaitingLineField("Status", ticketStatusToString(ticket.status));
    printWaitingLineField("Customer", ticket.customerName);
    printWaitingLineField("Movie", ticket.movieTitle);
    printWaitingLineField("Showtime", ticket.hallName + " | " + ticket.showTime + " | " + ticket.duration);

    stringstream seatStream;
    seatStream << ticket.seatCode << " | " << ticket.ticketType
               << " | RM" << fixed << setprecision(2) << ticket.ticketPrice;
    printWaitingLineField("Seat", seatStream.str());
}

WaitingLine::WaitingLine()
    : head(nullptr), tail(nullptr) {
}

WaitingLine::~WaitingLine() {
    clear();
}

// Add a ticket to the rear of the waiting line.
bool WaitingLine::joinLine(const Ticket& ticket) {
    // Duplicate ticket IDs should not appear twice in the waiting queue.
    if (contains(ticket.ticketID)) {
        cout << "Cannot join line: Ticket #" << ticket.ticketID << " is already in the waiting line." << endl;
        return false;
    }

    Node* newNode = new Node{ticket, nullptr};
    if (isEmpty()) {
        head = tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }

    cout << "Ticket #" << ticket.ticketID << " added to waiting line." << endl;
    return true;
}

// Add a ticket while rebuilding the queue from saved records, without printing a message.
bool WaitingLine::joinLineSilently(const Ticket& ticket) {
    // Silent enqueue is used when rebuilding the queue from saved ticket records.
    if (contains(ticket.ticketID)) {
        return false;
    }

    // Create a new linked-list node for the ticket.
    Node* newNode = new Node{ticket, nullptr};
    if (isEmpty()) {
        // If the queue is empty, the new node is both front and rear.
        head = tail = newNode;
    } else {
        // Otherwise, link the current rear node to the new node.
        tail->next = newNode;
        tail = newNode;
    }
    return true;
}

// Remove the front ticket from the waiting line.
bool WaitingLine::serveNext(Ticket& servedTicket) {
    // Cannot dequeue if the queue has no nodes.
    if (isEmpty()) {
        cout << "No customers to serve: the waiting line is empty." << endl;
        return false;
    }

    // Dequeue operation:
    // save the front ticket, move head to the next node, then delete the old front node.
    Node* current = head;
    servedTicket = current->ticket;
    head = head->next;
    if (head == nullptr) {
        // If the removed node was the only node, the rear pointer must also be cleared.
        tail = nullptr;
    }
    delete current;

    cout << "Serving Ticket #" << servedTicket.ticketID << ", customer " << servedTicket.customerName << "." << endl;
    return true;
}

// Display every customer currently in the queue from front to rear.
void WaitingLine::showLine() const {
    // Traversal starts at head and follows next pointers until nullptr.
    if (isEmpty()) {
        cout << "Waiting line is currently empty." << endl;
        return;
    }

    cout << endl;
    printWaitingLineSeparator();
    printWaitingLineTitle("Current Waiting Line (Front to Rear)");
    printWaitingLineSeparator();
    Node* current = head;
    int position = 1;
    while (current != nullptr) {
        const Ticket& ticket = current->ticket;
        printWaitingLineTitle("Queue Position " + to_string(position));
        printWaitingTicketDetails(ticket);
        current = current->next;
        position++;
    }
    printWaitingLineSeparator();
}

// Peek at the front customer without removing them.
void WaitingLine::showFront() const {
    if (isEmpty()) {
        cout << "The waiting line is empty. No front customer." << endl;
        return;
    }

    const Ticket& ticket = head->ticket;
    cout << endl;
    printWaitingLineSeparator();
    printWaitingLineTitle("Front of Line");
    printWaitingLineSeparator();
    printWaitingTicketDetails(ticket);
    printWaitingLineSeparator();
}

// Peek at the rear customer without removing them.
void WaitingLine::showRear() const {
    if (isEmpty()) {
        cout << "The waiting line is empty. No rear customer." << endl;
        return;
    }

    const Ticket& ticket = tail->ticket;
    cout << endl;
    printWaitingLineSeparator();
    printWaitingLineTitle("Rear of Line");
    printWaitingLineSeparator();
    printWaitingTicketDetails(ticket);
    printWaitingLineSeparator();
}

bool WaitingLine::isEmpty() const {
    // A linked-list queue is empty when head does not point to any node.
    return head == nullptr;
}

bool WaitingLine::isFull() const {
    // Linked list queue has no fixed capacity, so it is never full.
    return false;
}

// Count how many customers are currently waiting in the linked-list queue.
int WaitingLine::getCount() const {
    int count = 0;
    Node* current = head;
    while (current != nullptr) {
        count++;
        current = current->next;
    }
    return count;
}

// Remove a specific waiting ticket when it is cancelled.
bool WaitingLine::removeTicketByID(int ticketID) {
    // This removes a waiting ticket when the customer cancels before being served.
    if (isEmpty()) {
        return false;
    }

    Node* current = head;
    Node* previous = nullptr;
    while (current != nullptr) {
        if (current->ticket.ticketID == ticketID) {
            if (previous == nullptr) {
                head = current->next;
            } else {
                previous->next = current->next;
            }

            if (current == tail) {
                tail = previous;
            }

            delete current;
            return true;
        }
        previous = current;
        current = current->next;
    }
    return false;
}

// Check if a ticket already exists in the queue.
bool WaitingLine::contains(int ticketID) const {
    Node* current = head;
    while (current != nullptr) {
        if (current->ticket.ticketID == ticketID) {
            return true;
        }
        current = current->next;
    }
    return false;
}

// Keep queue data consistent after staff edits a waiting ticket.
bool WaitingLine::updateTicketByID(const Ticket& ticket) {
    Node* current = head;
    while (current != nullptr) {
        if (current->ticket.ticketID == ticket.ticketID) {
            current->ticket = ticket;
            return true;
        }
        current = current->next;
    }
    return false;
}

// Public wrapper for clearing the queue when ticket data is loaded from file.
void WaitingLine::clearLine() {
    clear();
}

void WaitingLine::clear() {
    while (head != nullptr) {
        Node* current = head;
        head = head->next;
        delete current;
    }
    tail = nullptr;
}
