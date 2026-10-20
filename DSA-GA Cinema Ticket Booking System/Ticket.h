#ifndef TICKET_H
#define TICKET_H

#include <chrono>
#include <sstream>
#include <string>

using namespace std;

// Represents the current state of a ticket inside the booking system.
enum class TicketStatus { WAITING, SERVED, CANCELLED };

// Stores one complete ticket record.
// This is the main data item stored inside the ticket registry array and waiting-line queue.
struct Ticket {
    int ticketID;
    string customerName;
    string movieTitle;
    string genre;
    string classification;
    string language;
    string hallName;
    string showTime;
    string duration;
    string seatCode;
    string ticketType;
    double ticketPrice;
    TicketStatus status;
};

// Stores one cancellation record.
// CancelEntry objects are pushed into CancelLog, which works like a stack.
struct CancelEntry {
    int ticketID;
    string customerName;
    string note;
    string cancelledAt;
};

// Convert TicketStatus enum value to a human-readable string.
string ticketStatusToString(TicketStatus status);

// Convert stored text back to a TicketStatus value.
bool stringToTicketStatus(const string& text, TicketStatus& status);

// Return the current timestamp as a formatted string.
string getCurrentTimestamp();

// Validate that a name or title is not empty and does not contain only spaces.
bool isValidText(const string& text);

// Validate seat code format such as A1, A2, or B10.
bool isValidSeatCode(const string& seatCode);

// Return lowercase text for case-insensitive comparison.
string toLowerText(const string& text);

#endif // TICKET_H
