#ifndef WAITING_LINE_H
#define WAITING_LINE_H

#include "Ticket.h"

#include <iostream>

using namespace std;

// Assignment Part A: Queue (FIFO) implementation.
// WaitingLine implements a queue using a linked list.
// The front node is served first, following FIFO (First In, First Out).
class WaitingLine {
public:
    WaitingLine();
    ~WaitingLine();

    // Enqueue ticket at the rear of the linked-list queue.
    bool joinLine(const Ticket& ticket);
    // Enqueue without display messages, used when rebuilding from file.
    bool joinLineSilently(const Ticket& ticket);
    // Dequeue the front ticket and return it through servedTicket.
    bool serveNext(Ticket& servedTicket);
    // Display queue contents and peek front/rear nodes.
    void showLine() const;
    void showFront() const;
    void showRear() const;
    // Queue status helpers required by Part A.
    bool isEmpty() const;
    bool isFull() const;
    int getCount() const;
    // Remove/update specific waiting tickets when cancel/edit happens.
    bool removeTicketByID(int ticketID);
    bool contains(int ticketID) const;
    bool updateTicketByID(const Ticket& ticket);
    // Clear all nodes before rebuilding queue data.
    void clearLine();

private:
    // Node stores one ticket and a pointer to the next customer in line.
    struct Node {
        Ticket ticket;
        Node* next;
    };

    // head is the front of the queue, tail is the rear of the queue.
    Node* head;
    Node* tail;

    // Internal helper used by destructor and clearLine().
    void clear();
};

#endif // WAITING_LINE_H
