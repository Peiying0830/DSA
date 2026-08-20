#include "Ticket.h"

#include <cctype>
#include <ctime>
#include <iomanip>

using namespace std;

// Convert enum status into readable text for display and file saving.
string ticketStatusToString(TicketStatus status) {
    // Used when displaying tickets and writing status into text files.
    switch (status) {
        case TicketStatus::WAITING:
            return "WAITING";
        case TicketStatus::SERVED:
            return "SERVED";
        case TicketStatus::CANCELLED:
            return "CANCELLED";
        default:
            return "UNKNOWN";
    }
}

// Convert text from file back into enum status.
bool stringToTicketStatus(const string& text, TicketStatus& status) {
    // File loading stores status as text, so it must be converted back to enum.
    string normalizedText = toLowerText(text);
    if (normalizedText == "waiting") {
        status = TicketStatus::WAITING;
        return true;
    }
    if (normalizedText == "served") {
        status = TicketStatus::SERVED;
        return true;
    }
    if (normalizedText == "cancelled" || normalizedText == "canceled") {
        status = TicketStatus::CANCELLED;
        return true;
    }
    return false;
}

// Generate current date and time for cancellation records and reports.
string getCurrentTimestamp() {
    // chrono gets the current system time; put_time formats it for cancellation records.
    auto now = chrono::system_clock::now();
    time_t timeValue = chrono::system_clock::to_time_t(now);
    tm* localTimePtr = localtime(&timeValue);
    ostringstream output;
    if (localTimePtr != nullptr) {
        output << put_time(localTimePtr, "%Y-%m-%d %H:%M:%S");
    } else {
        output << "unknown-time";
    }
    return output.str();
}

// Check that a string is not blank or spaces only.
bool isValidText(const string& text) {
    // At least one non-space character is required.
    for (char ch : text) {
        if (!isspace(static_cast<unsigned char>(ch))) {
            return true;
        }
    }
    return false;
}

// Seat format must start with a letter followed by one or more digits.
bool isValidSeatCode(const string& seatCode) {
    // This only checks general format. Registry checks the actual A-K / 01-18 seat range.
    if (seatCode.length() < 2) {
        return false;
    }
    if (!isalpha(static_cast<unsigned char>(seatCode[0]))) {
        return false;
    }

    for (size_t index = 1; index < seatCode.length(); ++index) {
        if (!isdigit(static_cast<unsigned char>(seatCode[index]))) {
            return false;
        }
    }
    return true;
}

// Lowercase conversion supports case-insensitive comparisons.
string toLowerText(const string& text) {
    // Used for case-insensitive comparisons such as movie titles, halls, and statuses.
    string loweredText = text;
    for (char& ch : loweredText) {
        ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
    }
    return loweredText;
}
