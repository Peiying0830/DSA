#include "Registry.h"

#include <cctype>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

using namespace std;

const int DisplayWidth = 100;
const int SeatMapWidth = 100;

// Print a centered display line for booking-related messages.
void printDisplayCenteredLine(const string& text) {
    int leftPadding = (DisplayWidth - static_cast<int>(text.length())) / 2;
    if (leftPadding < 0) {
        leftPadding = 0;
    }
    cout << string(leftPadding, ' ') << text << endl;
}

void printSeatMapCenteredLine(const string& text) {
    int leftPadding = (SeatMapWidth - static_cast<int>(text.length())) / 2;
    if (leftPadding < 0) {
        leftPadding = 0;
    }
    cout << string(leftPadding, ' ') << text << endl;
}

// Print one aligned field inside ticket detail and receipt blocks.
void printDisplayField(const string& label, const string& value) {
    cout << left << setw(18) << label << right << ": " << value << endl;
}

// Print one compact row for sorted ticket record tables.
void printSortedTicketRow(const Ticket& ticket) {
    string customerName = ticket.customerName.substr(0, 17);
    string movieTitle = ticket.movieTitle.substr(0, 27);
    cout << left << setw(8) << ticket.ticketID
         << setw(18) << customerName
         << setw(28) << movieTitle
         << setw(10) << ticket.hallName
         << setw(10) << ticket.seatCode
         << setw(16) << ticket.ticketType.substr(0, 15)
         << setw(12) << ticketStatusToString(ticket.status) << endl;
}

void printSortedTicketTable(const string& title, const Ticket sortedTickets[], int ticketCount, int comparisons, int shifts) {
    cout << endl;
    cout << string(DisplayWidth, '-') << endl;
    printDisplayCenteredLine(title);
    cout << string(DisplayWidth, '-') << endl;
    cout << left << setw(8) << "ID"
         << setw(18) << "Customer"
         << setw(28) << "Movie"
         << setw(10) << "Hall"
         << setw(10) << "Seat"
         << setw(16) << "Type"
         << setw(12) << "Status" << endl;
    cout << string(DisplayWidth, '-') << endl;
    for (int index = 0; index < ticketCount; ++index) {
        printSortedTicketRow(sortedTickets[index]);
    }
    cout << string(DisplayWidth, '-') << endl;
    cout << "Insertion sort comparisons: " << comparisons << endl;
    cout << "Insertion sort shifts: " << shifts << endl;
    cout << right;
}

// Convert valid duration input such as "2h 15m" or "95m" into a standard format.
bool normalizeDurationText(const string& input, string& normalizedDuration) {
    string compactText;
    for (char ch : input) {
        if (!isspace(static_cast<unsigned char>(ch))) {
            compactText += static_cast<char>(tolower(static_cast<unsigned char>(ch)));
        }
    }

    if (compactText.empty()) {
        return false;
    }

    int hours = 0;
    int minutes = 0;
    size_t hourPosition = compactText.find('h');
    bool hasHourPart = hourPosition != string::npos;

    if (hasHourPart) {
        string hourText = compactText.substr(0, hourPosition);
        string minutePart = compactText.substr(hourPosition + 1);
        if (hourText.empty()) {
            return false;
        }
        for (char ch : hourText) {
            if (!isdigit(static_cast<unsigned char>(ch))) {
                return false;
            }
        }

        stringstream hourStream(hourText);
        hourStream >> hours;

        if (!minutePart.empty()) {
            if (minutePart.back() != 'm') {
                return false;
            }
            string minuteText = minutePart.substr(0, minutePart.length() - 1);
            if (minuteText.empty()) {
                return false;
            }
            for (char ch : minuteText) {
                if (!isdigit(static_cast<unsigned char>(ch))) {
                    return false;
                }
            }
            stringstream minuteStream(minuteText);
            minuteStream >> minutes;
        }
    } else {
        if (compactText.back() != 'm') {
            return false;
        }
        string minuteText = compactText.substr(0, compactText.length() - 1);
        if (minuteText.empty()) {
            return false;
        }
        for (char ch : minuteText) {
            if (!isdigit(static_cast<unsigned char>(ch))) {
                return false;
            }
        }
        stringstream minuteStream(minuteText);
        minuteStream >> minutes;
    }

    if (minutes < 0 || (hasHourPart && minutes > 59)) {
        return false;
    }

    int totalMinutes = hours * 60 + minutes;
    if (totalMinutes < 30 || totalMinutes > 300) {
        return false;
    }

    hours = totalMinutes / 60;
    minutes = totalMinutes % 60;
    normalizedDuration = to_string(hours) + "h ";
    if (minutes < 10) {
        normalizedDuration += "0";
    }
    normalizedDuration += to_string(minutes) + "m";
    return true;
}

// Edit text must contain at least one letter or number, not only punctuation.
bool hasLetterOrNumber(const string& text) {
    for (char ch : text) {
        if (isalnum(static_cast<unsigned char>(ch))) {
            return true;
        }
    }
    return false;
}

Registry::Registry()
    : tickets(), ticketCount(0), waitingLine(), cancelLog(), movieCatalog() {
    loadMovieCatalogFromFile("movies_autosave.txt", false);
    loadTicketsFromFile("tickets_autosave.txt", false, false);
    cancelLog.loadFromFile("cancellations_autosave.txt", false);
}

// Check duplicate ticket ID in the ticket registry array.
bool Registry::ticketIDExists(int ticketID) const {
    for (int index = 0; index < ticketCount; ++index) {
        const Ticket& ticket = tickets[index];
        if (ticket.ticketID == ticketID) {
            return true;
        }
    }
    return false;
}

// Check whether a seat is already taken in the same hall and showtime.
// Cancelled tickets are ignored because their seats become available again.
bool Registry::seatCodeTaken(const string& hallName, const string& showTime, const string& seatCode) const {
    for (int index = 0; index < ticketCount; ++index) {
        const Ticket& ticket = tickets[index];
        if (toLowerText(ticket.hallName) == toLowerText(hallName)
            && toLowerText(ticket.showTime) == toLowerText(showTime)
            && toLowerText(ticket.seatCode) == toLowerText(seatCode)
            && ticket.status != TicketStatus::CANCELLED) {
            return true;
        }
    }
    return false;
}

// Used during edit operation to check seat duplication against other tickets only.
bool Registry::seatCodeTakenByOtherTicket(const string& hallName, const string& showTime, const string& seatCode, int currentTicketID) const {
    for (int index = 0; index < ticketCount; ++index) {
        const Ticket& ticket = tickets[index];
        if (ticket.ticketID != currentTicketID
            && toLowerText(ticket.hallName) == toLowerText(hallName)
            && toLowerText(ticket.showTime) == toLowerText(showTime)
            && toLowerText(ticket.seatCode) == toLowerText(seatCode)
            && ticket.status != TicketStatus::CANCELLED) {
            return true;
        }
    }
    return false;
}

// Seat map uses rows A-K and seat numbers 01-18, similar to a cinema seating layout.
bool Registry::seatCodeInsideSeatMap(const string& seatCode) const {
    if (!isValidSeatCode(seatCode)) {
        return false;
    }

    char rowLetter = static_cast<char>(toupper(static_cast<unsigned char>(seatCode[0])));
    if (rowLetter < 'A' || rowLetter > 'K') {
        return false;
    }

    int seatNumber = 0;
    stringstream seatStream(seatCode.substr(1));
    seatStream >> seatNumber;
    return seatNumber >= 1 && seatNumber <= 18;
}

// Row A seats 01-16 are treated as twin seats in the screen layout.
bool Registry::isDoubleSeat(const string& seatCode) const {
    if (!seatCodeInsideSeatMap(seatCode)) {
        return false;
    }

    char rowLetter = static_cast<char>(toupper(static_cast<unsigned char>(seatCode[0])));
    int seatNumber = 0;
    stringstream seatStream(seatCode.substr(1));
    seatStream >> seatNumber;
    return rowLetter == 'A' && seatNumber >= 1 && seatNumber <= 16;
}

// Row A seats 17-18 are wheelchair/OKU seats in the screen layout.
bool Registry::isWheelchairSeat(const string& seatCode) const {
    if (!seatCodeInsideSeatMap(seatCode)) {
        return false;
    }

    char rowLetter = static_cast<char>(toupper(static_cast<unsigned char>(seatCode[0])));
    int seatNumber = 0;
    stringstream seatStream(seatCode.substr(1));
    seatStream >> seatNumber;
    return rowLetter == 'A' && seatNumber >= 17 && seatNumber <= 18;
}

// Prevent deleting a screening if active tickets are still linked to it.
bool Registry::screeningHasActiveTickets(const string& movieTitle, const string& hallName, const string& showTime) const {
    for (int index = 0; index < ticketCount; ++index) {
        const Ticket& ticket = tickets[index];
        if (toLowerText(ticket.movieTitle) == toLowerText(movieTitle)
            && toLowerText(ticket.hallName) == toLowerText(hallName)
            && toLowerText(ticket.showTime) == toLowerText(showTime)
            && ticket.status != TicketStatus::CANCELLED) {
            return true;
        }
    }
    return false;
}

// Return a modifiable ticket pointer for update operations.
Ticket* Registry::findTicketByID(int ticketID) {
    for (int index = 0; index < ticketCount; ++index) {
        Ticket& ticket = tickets[index];
        if (ticket.ticketID == ticketID) {
            return &ticket;
        }
    }
    return nullptr;
}

// Return read-only ticket pointer for search/display operations.
const Ticket* Registry::findTicketByID(int ticketID) const {
    for (int index = 0; index < ticketCount; ++index) {
        const Ticket& ticket = tickets[index];
        if (ticket.ticketID == ticketID) {
            return &ticket;
        }
    }
    return nullptr;
}

// Wrapper for linear search algorithm.
int Registry::linearSearchIndex(int ticketID, int& comparisons) const {
    return Finder::linearFind(tickets, ticketCount, ticketID, comparisons);
}

// Wrapper for binary search algorithm.
int Registry::binarySearchIndex(int ticketID, int& comparisons) const {
    return Finder::binaryFind(tickets, ticketCount, ticketID, comparisons);
}

// Copy ticket records into another array and sort by ticket ID using insertion sort.
void Registry::getTicketsSortedByID(Ticket sortedTickets[], int& comparisons, int& shifts) const {
    for (int index = 0; index < ticketCount; ++index) {
        sortedTickets[index] = tickets[index];
    }
    comparisons = 0;
    shifts = 0;

    for (int i = 1; i < ticketCount; ++i) {
        Ticket key = sortedTickets[i];
        int j = i;

        while (j > 0) {
            comparisons++;
            if (sortedTickets[j - 1].ticketID <= key.ticketID) {
                break;
            }
            sortedTickets[j] = sortedTickets[j - 1];
            shifts++;
            --j;
        }
        sortedTickets[j] = key;
    }
}

// Copy ticket records and sort by movie title using insertion sort.
void Registry::getTicketsSortedByMovie(Ticket sortedTickets[], int& comparisons, int& shifts) const {
    for (int index = 0; index < ticketCount; ++index) {
        sortedTickets[index] = tickets[index];
    }
    comparisons = 0;
    shifts = 0;

    for (int i = 1; i < ticketCount; ++i) {
        Ticket key = sortedTickets[i];
        string keyMovie = toLowerText(key.movieTitle);
        int j = i;

        while (j > 0) {
            comparisons++;
            string previousMovie = toLowerText(sortedTickets[j - 1].movieTitle);
            bool alreadyInOrder = previousMovie < keyMovie
                || (previousMovie == keyMovie && sortedTickets[j - 1].ticketID <= key.ticketID);
            if (alreadyInOrder) {
                break;
            }
            sortedTickets[j] = sortedTickets[j - 1];
            shifts++;
            --j;
        }
        sortedTickets[j] = key;
    }
}

// Copy ticket records and sort by hall, then showtime, then ticket ID using insertion sort.
void Registry::getTicketsSortedByHall(Ticket sortedTickets[], int& comparisons, int& shifts) const {
    for (int index = 0; index < ticketCount; ++index) {
        sortedTickets[index] = tickets[index];
    }
    comparisons = 0;
    shifts = 0;

    for (int i = 1; i < ticketCount; ++i) {
        Ticket key = sortedTickets[i];
        string keyHall = toLowerText(key.hallName);
        int j = i;

        while (j > 0) {
            comparisons++;
            string previousHall = toLowerText(sortedTickets[j - 1].hallName);
            bool alreadyInOrder = previousHall < keyHall
                || (previousHall == keyHall && sortedTickets[j - 1].showTime < key.showTime)
                || (previousHall == keyHall && sortedTickets[j - 1].showTime == key.showTime
                    && sortedTickets[j - 1].ticketID <= key.ticketID);
            if (alreadyInOrder) {
                break;
            }
            sortedTickets[j] = sortedTickets[j - 1];
            shifts++;
            --j;
        }
        sortedTickets[j] = key;
    }
}

// Copy ticket records and sort by ticket type, then ticket ID using insertion sort.
void Registry::getTicketsSortedByType(Ticket sortedTickets[], int& comparisons, int& shifts) const {
    for (int index = 0; index < ticketCount; ++index) {
        sortedTickets[index] = tickets[index];
    }
    comparisons = 0;
    shifts = 0;

    for (int i = 1; i < ticketCount; ++i) {
        Ticket key = sortedTickets[i];
        string keyType = toLowerText(key.ticketType);
        int j = i;

        while (j > 0) {
            comparisons++;
            string previousType = toLowerText(sortedTickets[j - 1].ticketType);
            bool alreadyInOrder = previousType < keyType
                || (previousType == keyType && sortedTickets[j - 1].ticketID <= key.ticketID);
            if (alreadyInOrder) {
                break;
            }
            sortedTickets[j] = sortedTickets[j - 1];
            shifts++;
            --j;
        }
        sortedTickets[j] = key;
    }
}

// Copy ticket records and sort by status, then ticket ID using insertion sort.
void Registry::getTicketsSortedByStatus(Ticket sortedTickets[], int& comparisons, int& shifts) const {
    for (int index = 0; index < ticketCount; ++index) {
        sortedTickets[index] = tickets[index];
    }
    comparisons = 0;
    shifts = 0;

    for (int i = 1; i < ticketCount; ++i) {
        Ticket key = sortedTickets[i];
        string keyStatus = toLowerText(ticketStatusToString(key.status));
        int j = i;

        while (j > 0) {
            comparisons++;
            string previousStatus = toLowerText(ticketStatusToString(sortedTickets[j - 1].status));
            bool alreadyInOrder = previousStatus < keyStatus
                || (previousStatus == keyStatus && sortedTickets[j - 1].ticketID <= key.ticketID);
            if (alreadyInOrder) {
                break;
            }
            sortedTickets[j] = sortedTickets[j - 1];
            shifts++;
            --j;
        }
        sortedTickets[j] = key;
    }
}

// Display detailed information for one ticket.
void Registry::displayTicketDetails(const Ticket& ticket) const {
    cout << string(DisplayWidth, '=') << endl;
    printDisplayCenteredLine("Ticket Details");
    cout << string(DisplayWidth, '=') << endl;
    printDisplayField("Ticket ID", to_string(ticket.ticketID));
    printDisplayField("Customer Name", ticket.customerName);
    printDisplayField("Movie Title", ticket.movieTitle);
    printDisplayField("Genre", ticket.genre);
    printDisplayField("Classification", ticket.classification);
    printDisplayField("Language", ticket.language);
    printDisplayField("Duration", ticket.duration);
    printDisplayField("Hall", ticket.hallName);
    printDisplayField("Showtime", ticket.showTime);
    printDisplayField("Seat Code", ticket.seatCode);
    printDisplayField("Ticket Type", ticket.ticketType);
    cout << fixed << setprecision(2);
    stringstream priceStream;
    priceStream << fixed << setprecision(2) << "RM" << ticket.ticketPrice;
    printDisplayField("Ticket Price", priceStream.str());
    printDisplayField("Status", ticketStatusToString(ticket.status));
    cout << string(DisplayWidth, '=') << endl;
}

// Numeric validation for ticket IDs.
int Registry::getValidatedTicketID(const string& prompt) const {
    while (true) {
        cout << prompt;
        int ticketID;
        if (!(cin >> ticketID)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a numeric ticket ID." << endl;
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (ticketID < 0) {
            cout << "Invalid input. Ticket ID must be a positive number, or 0 to cancel." << endl;
            continue;
        }
        return ticketID;
    }
}

// Auto-generate the next unique ticket ID for customer bookings.
int Registry::generateTicketID() const {
    int ticketID = 1001;
    while (ticketIDExists(ticketID)) {
        ticketID++;
    }
    return ticketID;
}

// Text validation used by names, reasons, file names, movie titles, halls, and showtimes.
string Registry::getValidatedText(const string& prompt, const string& fieldName) const {
    while (true) {
        string value;
        cout << prompt;
        getline(cin, value);
        if (isValidText(value)) {
            return value;
        }
        cout << fieldName << " cannot be empty." << endl;
    }
}

string Registry::getValidatedClassification() const {
    while (true) {
        string classification = getValidatedText("Enter classification (U/P12/P13/P16/P18): ", "Classification");
        string normalizedClassification = toLowerText(classification);
        if (normalizedClassification == "u") {
            return "U";
        }
        if (normalizedClassification == "p12") {
            return "P12";
        }
        if (normalizedClassification == "p13") {
            return "P13";
        }
        if (normalizedClassification == "p16") {
            return "P16";
        }
        if (normalizedClassification == "p18") {
            return "P18";
        }
        cout << "Invalid classification. Please enter U, P12, P13, P16, or P18." << endl;
    }
}

string Registry::getValidatedHallName() const {
    while (true) {
        string hallName = getValidatedText("Enter hall name (Hall 1-6): ", "Hall name");
        string normalizedText = toLowerText(hallName);
        if (normalizedText.rfind("hall", 0) == 0) {
            normalizedText = normalizedText.substr(4);
        }

        string numberText;
        for (char ch : normalizedText) {
            if (!isspace(static_cast<unsigned char>(ch))) {
                numberText += ch;
            }
        }

        int hallNumber = 0;
        stringstream hallStream(numberText);
        if ((hallStream >> hallNumber) && hallNumber >= 1 && hallNumber <= 6) {
            return "Hall " + to_string(hallNumber);
        }
        cout << "Invalid hall. Please enter Hall 1 to Hall 6 only." << endl;
    }
}

string Registry::getValidatedDuration() const {
    while (true) {
        string duration = getValidatedText("Enter duration (example 2h 15m or 135m): ", "Duration");
        string normalizedDuration;
        if (normalizeDurationText(duration, normalizedDuration)) {
            return normalizedDuration;
        }
        cout << "Invalid duration. Please enter 30-300 minutes, for example 2h 15m or 135m." << endl;
    }
}

string Registry::getValidatedShowTime() const {
    while (true) {
        string showTime = getValidatedText("Enter showtime for today (24-hour, example 14:15): ", "Showtime");
        string compactText;
        for (char ch : showTime) {
            if (!isspace(static_cast<unsigned char>(ch))) {
                compactText += ch;
            }
        }

        size_t colonPosition = compactText.find(':');
        if (colonPosition == string::npos || colonPosition == 0 || colonPosition > 2
            || colonPosition + 3 != compactText.length()) {
            cout << "Invalid showtime. Please use 24-hour format like 10:00 or 20:30." << endl;
            continue;
        }

        string hourText = compactText.substr(0, colonPosition);
        string minuteText = compactText.substr(colonPosition + 1, 2);
        if (minuteText.length() != 2) {
            cout << "Invalid showtime. Please use 24-hour format like 10:00 or 20:30." << endl;
            continue;
        }

        bool validDigits = true;
        for (char ch : hourText + minuteText) {
            if (!isdigit(static_cast<unsigned char>(ch))) {
                validDigits = false;
            }
        }
        if (!validDigits) {
            cout << "Invalid showtime. Please use numeric hour and minute." << endl;
            continue;
        }

        int hour = 0;
        int minute = 0;
        stringstream hourStream(hourText);
        stringstream minuteStream(minuteText);
        hourStream >> hour;
        minuteStream >> minute;
        if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
            cout << "Invalid showtime. Hour must be 00-23 and minute must be 00-59." << endl;
            continue;
        }

        string normalizedShowTime;
        if (hour < 10) {
            normalizedShowTime += "0";
        }
        normalizedShowTime += to_string(hour) + ":";
        if (minute < 10) {
            normalizedShowTime += "0";
        }
        normalizedShowTime += to_string(minute);
        return normalizedShowTime;
    }
}

string Registry::getOptionalClassification(const string& currentClassification) const {
    while (true) {
        string classification = getOptionalText("New classification (U/P12/P13/P16/P18): ");
        if (!isValidText(classification)) {
            return currentClassification;
        }

        string normalizedClassification = toLowerText(classification);
        if (normalizedClassification == "u") {
            return "U";
        }
        if (normalizedClassification == "p12") {
            return "P12";
        }
        if (normalizedClassification == "p13") {
            return "P13";
        }
        if (normalizedClassification == "p16") {
            return "P16";
        }
        if (normalizedClassification == "p18") {
            return "P18";
        }
        cout << "Invalid classification. Please enter U, P12, P13, P16, P18, or press Enter to keep current value." << endl;
    }
}

string Registry::getOptionalHallName(const string& currentHallName) const {
    while (true) {
        string hallName = getOptionalText("New hall name (Hall 1-6): ");
        if (!isValidText(hallName)) {
            return currentHallName;
        }

        string normalizedText = toLowerText(hallName);
        if (normalizedText.rfind("hall", 0) == 0) {
            normalizedText = normalizedText.substr(4);
        }

        string numberText;
        for (char ch : normalizedText) {
            if (!isspace(static_cast<unsigned char>(ch))) {
                numberText += ch;
            }
        }

        int hallNumber = 0;
        stringstream hallStream(numberText);
        if ((hallStream >> hallNumber) && hallNumber >= 1 && hallNumber <= 6) {
            return "Hall " + to_string(hallNumber);
        }
        cout << "Invalid hall. Please enter Hall 1 to Hall 6, or press Enter to keep current value." << endl;
    }
}

string Registry::getOptionalDuration(const string& currentDuration) const {
    while (true) {
        string duration = getOptionalText("New duration (example 2h 15m or 135m): ");
        if (!isValidText(duration)) {
            return currentDuration;
        }

        string normalizedDuration;
        if (normalizeDurationText(duration, normalizedDuration)) {
            return normalizedDuration;
        }
        cout << "Invalid duration. Please enter 30-300 minutes, for example 2h 15m or 135m, or press Enter to keep current value." << endl;
    }
}

string Registry::getOptionalShowTime(const string& currentShowTime) const {
    while (true) {
        string showTime = getOptionalText("New showtime (24-hour, example 14:15): ");
        if (!isValidText(showTime)) {
            return currentShowTime;
        }

        string compactText;
        for (char ch : showTime) {
            if (!isspace(static_cast<unsigned char>(ch))) {
                compactText += ch;
            }
        }

        size_t colonPosition = compactText.find(':');
        if (colonPosition == string::npos || colonPosition == 0 || colonPosition > 2
            || colonPosition + 3 != compactText.length()) {
            cout << "Invalid showtime. Please use 24-hour format like 10:00 or 20:30, or press Enter to keep current value." << endl;
            continue;
        }

        string hourText = compactText.substr(0, colonPosition);
        string minuteText = compactText.substr(colonPosition + 1, 2);
        if (minuteText.length() != 2) {
            cout << "Invalid showtime. Please use 24-hour format like 10:00 or 20:30, or press Enter to keep current value." << endl;
            continue;
        }

        bool validDigits = true;
        for (char ch : hourText + minuteText) {
            if (!isdigit(static_cast<unsigned char>(ch))) {
                validDigits = false;
            }
        }
        if (!validDigits) {
            cout << "Invalid showtime. Please use numeric hour and minute, or press Enter to keep current value." << endl;
            continue;
        }

        int hour = 0;
        int minute = 0;
        stringstream hourStream(hourText);
        stringstream minuteStream(minuteText);
        hourStream >> hour;
        minuteStream >> minute;
        if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
            cout << "Invalid showtime. Hour must be 00-23 and minute must be 00-59, or press Enter to keep current value." << endl;
            continue;
        }

        string normalizedShowTime;
        if (hour < 10) {
            normalizedShowTime += "0";
        }
        normalizedShowTime += to_string(hour) + ":";
        if (minute < 10) {
            normalizedShowTime += "0";
        }
        normalizedShowTime += to_string(minute);
        return normalizedShowTime;
    }
}

// Customer flow: choose a movie first, then choose one of that movie's showtimes.
int Registry::getScreeningNumberFromSelection() const {
    while (true) {
        movieCatalog.showTodaysMovies();
        if (movieCatalog.getMovieCount() == 0) {
            return 0;
        }
        cout << endl << "Select movie number, or 0 to return: ";
        int movieNumber;
        if (!(cin >> movieNumber)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a movie number." << endl;
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (movieNumber == 0) {
            return 0;
        }
        if (!movieCatalog.isValidUniqueMovieNumber(movieNumber)) {
            cout << "Invalid movie number. Please select from the movie list." << endl;
            continue;
        }

        string movieTitle = movieCatalog.getUniqueMovieTitle(movieNumber);
        while (true) {
            movieCatalog.showShowtimesForMovie(movieTitle);
            cout << endl << "Select showtime number, or 0 to return to movie list: ";
            int showtimeNumber;
            if (!(cin >> showtimeNumber)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please enter a showtime number." << endl;
                continue;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (showtimeNumber == 0) {
                break;
            }

            int screeningNumber = movieCatalog.getScreeningNumberForMovieShowtime(movieTitle, showtimeNumber);
            if (screeningNumber == 0) {
                cout << "Invalid showtime number. Please select from the showtime list." << endl;
                continue;
            }
            return screeningNumber;
        }
    }
}

// Used during edit: user can choose a new screening or keep the current screening.
int Registry::getOptionalMovieNumberFromSelection() const {
    while (true) {
        movieCatalog.showMovieSummary();
        if (movieCatalog.getMovieCount() == 0) {
            return 0;
        }

        cout << endl << "Enter new movie number, or 0 to keep current screening: ";
        int movieNumber;
        if (!(cin >> movieNumber)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a movie number." << endl;
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (movieNumber == 0) {
            return 0;
        }
        if (!movieCatalog.isValidUniqueMovieNumber(movieNumber)) {
            cout << "Invalid movie number. Please select from the movie list." << endl;
            continue;
        }

        string movieTitle = movieCatalog.getUniqueMovieTitle(movieNumber);
        while (true) {
            movieCatalog.showShowtimesForMovie(movieTitle);
            cout << endl << "Select new showtime number, or 0 to keep current screening: ";
            int showtimeNumber;
            if (!(cin >> showtimeNumber)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please enter a showtime number." << endl;
                continue;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (showtimeNumber == 0) {
                return 0;
            }

            int screeningNumber = movieCatalog.getScreeningNumberForMovieShowtime(movieTitle, showtimeNumber);
            if (screeningNumber == 0) {
                cout << "Invalid showtime number. Please select from the showtime list." << endl;
                continue;
            }
            return screeningNumber;
        }
    }
}

// Validate seat format such as A01 or K18.
string Registry::getValidatedSeatCode() const {
    while (true) {
        string seatCode = getValidatedText("Enter seat code (example A01, K18): ", "Seat code");
        if (isValidSeatCode(seatCode) && seatCodeInsideSeatMap(seatCode)) {
            char rowLetter = static_cast<char>(toupper(static_cast<unsigned char>(seatCode[0])));
            int seatNumber = 0;
            stringstream seatStream(seatCode.substr(1));
            seatStream >> seatNumber;
            string normalizedSeatCode;
            normalizedSeatCode += rowLetter;
            if (seatNumber < 10) {
                normalizedSeatCode += "0";
            }
            normalizedSeatCode += to_string(seatNumber);
            return normalizedSeatCode;
        }
        cout << "Invalid seat. Please choose a seat from A01 to K18." << endl;
    }
}

// Ticket type selection also sets ticket price for revenue calculation.
void Registry::getTicketTypeFromSelection(string& ticketType, double& ticketPrice) const {
    while (true) {
        cout << "\nTicket Types" << endl;
        cout << "1. Adult - RM18.00" << endl;
        cout << "2. Student - RM12.00" << endl;
        cout << "3. Child - RM8.00" << endl;
        cout << endl << "Select ticket type: ";

        int option;
        if (!(cin >> option)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a ticket type number." << endl;
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (option == 1) {
            ticketType = "Adult";
            ticketPrice = 18.00;
            return;
        }
        if (option == 2) {
            ticketType = "Student";
            ticketPrice = 12.00;
            return;
        }
        if (option == 3) {
            ticketType = "Child";
            ticketPrice = 8.00;
            return;
        }
        cout << "Invalid ticket type. Please select 1, 2, or 3." << endl;
    }
}

// Optional input for edit operation; empty input means keep current value.
string Registry::getOptionalValidatedText(const string& prompt, const string& currentValue, const string& fieldName) const {
    while (true) {
        string value = getOptionalText(prompt);
        if (!isValidText(value)) {
            return currentValue;
        }
        if (hasLetterOrNumber(value)) {
            return value;
        }
        cout << "Invalid " << fieldName << ". Please enter letters/numbers, or press Enter to keep current value." << endl;
    }
}

string Registry::getOptionalText(const string& prompt) const {
    string value;
    cout << prompt;
    getline(cin, value);
    return value;
}

// Case-insensitive Y/N validation for simple confirmation prompts.
bool Registry::getYesNoAnswer(const string& prompt) const {
    while (true) {
        string answer;
        cout << prompt;
        getline(cin, answer);
        string normalizedAnswer = toLowerText(answer);

        if (normalizedAnswer == "y" || normalizedAnswer == "yes") {
            return true;
        }
        if (normalizedAnswer == "n" || normalizedAnswer == "no") {
            return false;
        }
        cout << "Invalid input. Please enter Y or N." << endl;
    }
}

// Standard ticket ID prompt used by search/cancel/edit.
int Registry::getTicketIDFromInput() const {
    return getValidatedTicketID("Enter ticket ID, or 0 to cancel: ");
}

// Ask how many tickets to create for the selected screening.
// Entering 0 skips the booking flow and returns to the Customer Menu.
int Registry::getBookingQuantity() const {
    while (true) {
        cout << "Enter number of tickets to book, or 0 to skip booking: ";
        int quantity;
        if (!(cin >> quantity)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number." << endl;
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        int remainingCapacity = MaxTickets - ticketCount;
        if (quantity == 0) {
            return 0;
        }
        if (quantity <= 0) {
            cout << "Invalid quantity. Please enter at least 1 ticket, or 0 to skip booking." << endl;
            continue;
        }
        if (quantity > remainingCapacity) {
            cout << "Cannot book " << quantity << " ticket(s). Only "
                 << remainingCapacity << " ticket slot(s) available." << endl;
            continue;
        }
        return quantity;
    }
}

// Insert a ticket into the ticket registry array.
void Registry::addTicketToRegistry(const Ticket& ticket) {
    if (ticketCount >= MaxTickets) {
        cout << "Cannot add ticket: ticket registry is full." << endl;
        return;
    }
    tickets[ticketCount] = ticket;
    ticketCount++;
}

// Rebuild queue after loading ticket records from text file.
void Registry::rebuildWaitingLine() {
    waitingLine.clearLine();
    for (int index = 0; index < ticketCount; ++index) {
        const Ticket& ticket = tickets[index];
        if (ticket.status == TicketStatus::WAITING) {
            waitingLine.joinLineSilently(ticket);
        }
    }
}

void Registry::rebuildCancellationLogFromCancelledTickets() {
    cancelLog.clear();
    for (int index = 0; index < ticketCount; ++index) {
        const Ticket& ticket = tickets[index];
        if (ticket.status == TicketStatus::CANCELLED) {
            CancelEntry entry{
                ticket.ticketID,
                ticket.customerName,
                "Recovered from loaded ticket record",
                "Loaded from ticket file"
            };
            cancelLog.recordCancelSilently(entry);
        }
    }
}

// Print one compact ticket row to console or report file.
void Registry::printTicketLine(const Ticket& ticket, ostream& output) const {
    output << "  Ticket #" << ticket.ticketID << " | " << ticket.customerName
           << " | " << ticket.movieTitle << " | " << ticket.genre
           << " | " << ticket.classification << " | " << ticket.language
           << " | " << ticket.duration
           << " | " << ticket.hallName
           << " | " << ticket.showTime << " | " << ticket.seatCode << " | " << ticket.ticketType
           << " | RM" << fixed << setprecision(2) << ticket.ticketPrice << " | "
           << ticketStatusToString(ticket.status);
}

// Print booking receipt immediately after a successful booking.
void Registry::printReceipt(const Ticket& ticket) const {
    cout << endl;
    cout << string(DisplayWidth, '=') << endl;
    printDisplayCenteredLine("Booking Receipt");
    cout << string(DisplayWidth, '=') << endl;
    cout << "Ticket ID: " << ticket.ticketID << endl;
    cout << "Customer: " << ticket.customerName << endl;
    cout << "Movie: " << ticket.movieTitle << endl;
    cout << "Genre: " << ticket.genre << endl;
    cout << "Classification: " << ticket.classification << endl;
    cout << "Language: " << ticket.language << endl;
    cout << "Duration: " << ticket.duration << endl;
    cout << "Hall: " << ticket.hallName << endl;
    cout << "Showtime: " << ticket.showTime << endl;
    cout << "Seat: " << ticket.seatCode << endl;
    cout << "Ticket Type: " << ticket.ticketType << endl;
    cout << fixed << setprecision(2);
    cout << "Price: RM" << ticket.ticketPrice << endl;
    cout << "Status: " << ticketStatusToString(ticket.status) << endl;
    cout << string(DisplayWidth, '=') << endl;
}

// Print one receipt for a booking that contains multiple seats.
void Registry::printGroupReceipt(const Ticket bookedTickets[], int bookedCount) const {
    if (bookedCount <= 0) {
        return;
    }

    double totalPrice = 0.0;
    string ticketIDs;
    string seats;
    string ticketTypes;
    for (int index = 0; index < bookedCount; ++index) {
        const Ticket& ticket = bookedTickets[index];
        if (index > 0) {
            ticketIDs += ", ";
            seats += ", ";
            ticketTypes += ", ";
        }
        ticketIDs += to_string(ticket.ticketID);
        seats += ticket.seatCode;
        ticketTypes += ticket.ticketType;
        totalPrice += ticket.ticketPrice;
    }

    const Ticket& firstTicket = bookedTickets[0];
    cout << endl;
    cout << string(DisplayWidth, '=') << endl;
    printDisplayCenteredLine("Group Booking Receipt");
    cout << string(DisplayWidth, '=') << endl;
    printDisplayField("Ticket IDs", ticketIDs);
    printDisplayField("Customer", firstTicket.customerName);
    printDisplayField("Movie", firstTicket.movieTitle);
    printDisplayField("Genre", firstTicket.genre);
    printDisplayField("Classification", firstTicket.classification);
    printDisplayField("Language", firstTicket.language);
    printDisplayField("Duration", firstTicket.duration);
    printDisplayField("Hall", firstTicket.hallName);
    printDisplayField("Showtime", firstTicket.showTime);
    printDisplayField("Seats", seats);
    printDisplayField("Ticket Types", ticketTypes);

    stringstream totalStream;
    totalStream << fixed << setprecision(2) << "RM" << totalPrice;
    printDisplayField("Total Price", totalStream.str());
    printDisplayField("Status", "WAITING");
    cout << string(DisplayWidth, '=') << endl;
}

// Display a simple cinema seat map for one hall and showtime.
// O means available, T means twin seat, W means wheelchair/OKU seat, X means sold/booked.
void Registry::displaySeatMapForHall(const string& hallName, const string& showTime) const {
    cout << endl;
    cout << string(SeatMapWidth, '-') << endl;
    printSeatMapCenteredLine("Screen / Seat Map - " + hallName + " (" + showTime + ")");
    cout << string(SeatMapWidth, '-') << endl;
    printSeatMapCenteredLine("O = Available, T = Twin Seat, W = Wheelchair, X = Sold");
    cout << endl;
    cout << "       ";
    for (int seatNumber = 1; seatNumber <= 18; ++seatNumber) {
        string seatLabel;
        if (seatNumber < 10) {
            seatLabel += "0";
        }
        seatLabel += to_string(seatNumber);
        cout << setw(4) << seatLabel;
        if (seatNumber == 2 || seatNumber == 16) {
            cout << "    ";
        }
    }
    cout << endl;
    cout << endl;

    for (char rowLetter = 'K'; rowLetter >= 'A'; --rowLetter) {
        cout << rowLetter << "      ";
        for (int seatNumber = 1; seatNumber <= 18; ++seatNumber) {
            string seatCode;
            seatCode += rowLetter;
            if (seatNumber < 10) {
                seatCode += "0";
            }
            seatCode += to_string(seatNumber);

            if (seatCodeTaken(hallName, showTime, seatCode)) {
                cout << setw(4) << "X";
            } else if (isDoubleSeat(seatCode)) {
                cout << setw(4) << "T";
            } else if (isWheelchairSeat(seatCode)) {
                cout << setw(4) << "W";
            } else {
                cout << setw(4) << "O";
            }

            if (seatNumber == 2 || seatNumber == 16) {
                cout << "    ";
            }
        }
        cout << endl;
    }
}

// Shared file-writing helper for both manual save and autosave.
bool Registry::saveTicketsToFile(const string& fileName, bool showMessage) const {
    ofstream outputFile(fileName);
    if (!outputFile) {
        if (showMessage) {
            cout << "Unable to open file for saving." << endl;
        }
        return false;
    }

    outputFile << "TicketID|CustomerName|MovieTitle|Genre|Classification|Language|Duration|Hall|Showtime|SeatCode|TicketType|TicketPrice|Status" << endl;
    for (int index = 0; index < ticketCount; ++index) {
        const Ticket& ticket = tickets[index];
        outputFile << ticket.ticketID << "|"
                   << ticket.customerName << "|"
                   << ticket.movieTitle << "|"
                   << ticket.genre << "|"
                   << ticket.classification << "|"
                   << ticket.language << "|"
                   << ticket.duration << "|"
                   << ticket.hallName << "|"
                   << ticket.showTime << "|"
                   << ticket.seatCode << "|"
                   << ticket.ticketType << "|"
                   << fixed << setprecision(2) << ticket.ticketPrice << "|"
                   << ticketStatusToString(ticket.status) << endl;
    }

    if (showMessage) {
        cout << "Tickets saved to " << fileName << "." << endl;
    }
    return true;
}

// Shared movie catalog file-writing helper for both manual save and autosave.
bool Registry::saveMovieCatalogToFile(const string& fileName, bool showMessage) const {
    ofstream outputFile(fileName);
    if (!outputFile) {
        if (showMessage) {
            cout << "Unable to open movie catalog file for saving." << endl;
        }
        return false;
    }

    outputFile << "MovieTitle|Genre|Classification|Language|Duration|Hall|Showtime" << endl;
    for (int movieNumber = 1; movieNumber <= movieCatalog.getMovieCount(); ++movieNumber) {
        outputFile << movieCatalog.getMovieTitle(movieNumber) << "|"
                   << movieCatalog.getGenre(movieNumber) << "|"
                   << movieCatalog.getClassification(movieNumber) << "|"
                   << movieCatalog.getLanguage(movieNumber) << "|"
                   << movieCatalog.getDuration(movieNumber) << "|"
                   << movieCatalog.getHallName(movieNumber) << "|"
                   << movieCatalog.getShowTime(movieNumber) << endl;
    }

    if (showMessage) {
        cout << "Movie catalog saved to " << fileName << "." << endl;
    }
    return true;
}

// Automatically save ticket records after every ticket-related change.
void Registry::autoSaveTickets() const {
    if (saveTicketsToFile("tickets_autosave.txt", false)) {
        cout << endl << "Auto-saved ticket records to tickets_autosave.txt." << endl;
    }
}

// Automatically save movie catalog after every movie-related change.
void Registry::autoSaveMovieCatalog() const {
    if (saveMovieCatalogToFile("movies_autosave.txt", false)) {
        cout << "Auto-saved movie catalog to movies_autosave.txt." << endl;
    }
}

// Automatically save cancellation stack records after cancel or undo.
void Registry::autoSaveCancellationLog() const {
    if (cancelLog.saveToFile("cancellations_autosave.txt", false)) {
        cout << "Auto-saved cancellation log to cancellations_autosave.txt." << endl;
    }
}

// Assignment Part A: Add customer to booking queue.
// Customer booking flow: validate input, create ticket, enqueue it, print receipt, and autosave.
void Registry::bookSelectedScreening(int movieNumber, bool showSeatMap) {
    // If no screening was selected, stop the booking process immediately.
    if (movieNumber == 0) {
        cout << "No movie selected." << endl;
        return;
    }

    // The ticket registry uses a fixed array, so it must still check capacity.
    // This is separate from the waiting-line queue, which is a linked list.
    if (ticketCount >= MaxTickets) {
        cout << "Cannot join line: ticket registry is full." << endl;
        return;
    }

    // Copy the selected movie/showtime details from MovieCatalog.
    // These details will be stored inside every ticket created for this booking.
    string movieTitle = movieCatalog.getMovieTitle(movieNumber);
    string genre = movieCatalog.getGenre(movieNumber);
    string classification = movieCatalog.getClassification(movieNumber);
    string language = movieCatalog.getLanguage(movieNumber);
    string duration = movieCatalog.getDuration(movieNumber);
    string hallName = movieCatalog.getHallName(movieNumber);
    string showTime = movieCatalog.getShowTime(movieNumber);

    // Ask how many seats the customer wants.
    // A quantity of 0 means the customer chooses to skip this booking flow.
    int bookingQuantity = getBookingQuantity();
    if (bookingQuantity == 0) {
        cout << "Booking skipped. Returning to Customer Menu." << endl;
        return;
    }
    // One customer name is used for the whole booking group.
    // If the customer books multiple seats, the system creates multiple tickets under this name.
    string customerName = getValidatedText("Enter customer name: ", "Customer name");
    string selectedSeats[MaxTickets];

    // Show the seat map before seat selection.
    // For multi-ticket bookings, the map is always shown so the customer can choose seats together.
    if (showSeatMap || bookingQuantity > 1) {
        cout << endl;
        printDisplayCenteredLine("Seat Selection");
        displaySeatMapForHall(hallName, showTime);
    }

    // Keep asking for seat input until all selected seats are valid and available.
    while (true) {
        if (bookingQuantity == 1) {
            // Single-ticket booking: validate one seat code.
            while (true) {
                string seatCode = getOptionalText("Enter seat code (example A01, K18), or 0 to skip booking: ");
                if (seatCode == "0") {
                    cout << "Booking skipped. Returning to Customer Menu." << endl;
                    return;
                }
                if (isValidSeatCode(seatCode) && seatCodeInsideSeatMap(seatCode)) {
                    // Normalize seat code format.
                    // Example: a1 or A1 becomes A01, so comparisons are consistent.
                    char rowLetter = static_cast<char>(toupper(static_cast<unsigned char>(seatCode[0])));
                    int seatNumber = 0;
                    stringstream seatStream(seatCode.substr(1));
                    seatStream >> seatNumber;

                    selectedSeats[0] = "";
                    selectedSeats[0] += rowLetter;
                    if (seatNumber < 10) {
                        selectedSeats[0] += "0";
                    }
                    selectedSeats[0] += to_string(seatNumber);
                    break;
                }
                cout << "Invalid seat. Please choose a seat from A01 to K18, or enter 0 to skip booking." << endl;
            }
        } else {
            // Multi-ticket booking: allow the customer to enter all seats in one line.
            // Spaces or commas are accepted, for example: B01 B02 or B01,B02.
            cout << endl << "Enter " << bookingQuantity
                 << " seat codes separated by spaces (example B01 B02), or 0 to skip booking: ";
            string seatLine;
            getline(cin, seatLine);
            if (seatLine == "0") {
                cout << "Booking skipped. Returning to Customer Menu." << endl;
                return;
            }
            // Change commas into spaces so stringstream can read both input styles.
            for (char& ch : seatLine) {
                if (ch == ',') {
                    ch = ' ';
                }
            }

            stringstream seatStream(seatLine);
            string rawSeatCode;
            int seatCount = 0;
            while (seatStream >> rawSeatCode && seatCount < bookingQuantity) {
                // Every entered seat must follow the valid cinema seat range.
                if (!isValidSeatCode(rawSeatCode) || !seatCodeInsideSeatMap(rawSeatCode)) {
                    seatCount = -1;
                    break;
                }

                // Normalize each seat code before storing it in the selectedSeats array.
                char rowLetter = static_cast<char>(toupper(static_cast<unsigned char>(rawSeatCode[0])));
                int seatNumber = 0;
                stringstream numberStream(rawSeatCode.substr(1));
                numberStream >> seatNumber;

                selectedSeats[seatCount] = "";
                selectedSeats[seatCount] += rowLetter;
                if (seatNumber < 10) {
                    selectedSeats[seatCount] += "0";
                }
                selectedSeats[seatCount] += to_string(seatNumber);
                seatCount++;
            }

            // The number of seat codes must match the number of tickets requested.
            // This prevents booking 3 tickets but only entering 2 seats, or entering extra seats.
            string extraSeatCode;
            if (seatStream >> extraSeatCode || seatCount != bookingQuantity) {
                cout << "Invalid seat list. Please enter exactly " << bookingQuantity
                     << " valid seat code(s)." << endl;
                continue;
            }
        }

        // Check seat availability after input validation.
        // A seat is considered taken only if it is sold in the same hall and same showtime.
        bool validSeatList = true;
        for (int index = 0; index < bookingQuantity; ++index) {
            if (seatCodeTaken(hallName, showTime, selectedSeats[index])) {
                cout << "Cannot join line: Seat " << selectedSeats[index]
                     << " is already taken in " << hallName << "." << endl;
                validSeatList = false;
                break;
            }

            // Also prevent the same seat from being typed twice in one multi-ticket booking.
            for (int previousIndex = 0; previousIndex < index; ++previousIndex) {
                if (selectedSeats[previousIndex] == selectedSeats[index]) {
                    cout << "Cannot join line: Seat " << selectedSeats[index]
                         << " was selected more than once." << endl;
                    validSeatList = false;
                    break;
                }
            }
            if (!validSeatList) {
                break;
            }
        }

        // If all seats are available and unique, leave the validation loop.
        if (validSeatList) {
            break;
        }
    }

    // Store created tickets temporarily so the receipt can print single or group booking details.
    Ticket bookedTickets[MaxTickets];
    int bookedCount = 0;

    // Create one Ticket object for each selected seat.
    // Each ticket gets its own auto-generated Ticket ID.
    for (int bookingNumber = 1; bookingNumber <= bookingQuantity; ++bookingNumber) {
        string seatCode = selectedSeats[bookingNumber - 1];

        int ticketID = generateTicketID();
        cout << endl << "Generated Ticket ID: " << ticketID << endl;
        cout << "Seat Code: " << seatCode << endl;

        string ticketType;
        double ticketPrice = 0.0;
        // Special seats automatically decide ticket type and price.
        // Normal seats let the user choose Adult/Student/Child.
        if (isDoubleSeat(seatCode)) {
            ticketType = "Twin Seat";
            ticketPrice = 30.00;
            cout << "Twin seat selected. Ticket type automatically set to Twin Seat - RM30.00." << endl;
        } else if (isWheelchairSeat(seatCode)) {
            ticketType = "Wheelchair Seat";
            ticketPrice = 18.00;
            cout << "Wheelchair seat selected. Ticket type automatically set to Wheelchair Seat - RM18.00." << endl;
        } else {
            getTicketTypeFromSelection(ticketType, ticketPrice);
        }

        // New tickets start as WAITING because they are not served yet.
        Ticket ticket{ticketID, customerName, movieTitle, genre, classification, language, hallName, showTime, duration, seatCode, ticketType, ticketPrice, TicketStatus::WAITING};
        addTicketToRegistry(ticket);

        // Assignment Part A Queue:
        // joinLine() enqueues the ticket at the rear of the linked-list waiting line.
        // This follows FIFO: the first customer added will be served first.
        if (waitingLine.joinLine(ticket)) {
            bookedTickets[bookedCount] = ticket;
            bookedCount++;
        }
    }

    // Print receipt after the ticket(s) successfully enter the waiting queue.
    if (bookedCount == 1) {
        printReceipt(bookedTickets[0]);
    } else if (bookedCount > 1) {
        printGroupReceipt(bookedTickets, bookedCount);
    }
    // Auto-save ticket records immediately after booking.
    if (bookedCount > 0) {
        autoSaveTickets();
    }
}

// Combined customer browsing flow: movies, showtimes, seat map, and optional booking.
void Registry::browseMoviesAndBook() {
    int movieNumber = getScreeningNumberFromSelection();
    if (movieNumber == 0) {
        cout << "Please ask Staff/Admin to add movies first." << endl;
        return;
    }
    string movieTitle = movieCatalog.getMovieTitle(movieNumber);
    string showTime = movieCatalog.getShowTime(movieNumber);
    string duration = movieCatalog.getDuration(movieNumber);
    string hallName = movieCatalog.getHallName(movieNumber);

    cout << endl;
    printDisplayCenteredLine("Selected: " + movieTitle + " (" + showTime + ", " + duration + ")");
    displaySeatMapForHall(hallName, showTime);

    cout << endl;
    if (getYesNoAnswer("Book this showtime? (Y/N): ")) {
        bookSelectedScreening(movieNumber, true);
    } else {
        cout << "Returning to Customer Menu." << endl;
    }
}

void Registry::joinWaitingLine() {
    int movieNumber = getScreeningNumberFromSelection();
    if (movieNumber == 0) {
        cout << "Please ask Staff/Admin to add movies first." << endl;
        return;
    }
    bookSelectedScreening(movieNumber, true);
}

// Assignment Part A: Serve/remove customer from queue using FIFO.
// Staff serves the front customer in FIFO order and updates ticket status.
void Registry::serveNextCustomer() {
    // isEmpty() is one of the required queue operations.
    // It prevents dequeuing from an empty queue.
    if (waitingLine.isEmpty()) {
        cout << "No customers to serve: the waiting line is empty." << endl;
        return;
    }

    // Show the queue first so staff can see who is at the front before serving.
    waitingLine.showLine();
    if (!getYesNoAnswer("Confirm serve next customer? (Y/N): ")) {
        cout << "Serve cancelled. Ticket status remains WAITING." << endl;
        return;
    }

    Ticket servedTicket;
    // Assignment Part A Queue:
    // serveNext() dequeues/removes the front node from the linked-list queue.
    // This proves FIFO because the oldest waiting ticket is served first.
    if (waitingLine.serveNext(servedTicket)) {
        Ticket* ticket = findTicketByID(servedTicket.ticketID);
        if (ticket != nullptr) {
            // After dequeuing from the waiting line, update the master ticket record.
            ticket->status = TicketStatus::SERVED;
            cout << endl << "Served ticket details:" << endl;
            displayTicketDetails(*ticket);
            autoSaveTickets();
        }
    }
}

// Serve every waiting customer in FIFO order.
void Registry::serveAllCustomers() {
    // Do not run the serve-all loop if the queue has no nodes.
    if (waitingLine.isEmpty()) {
        cout << "No customers to serve: the waiting line is empty." << endl;
        return;
    }

    // Staff can review the current queue before confirming bulk service.
    waitingLine.showLine();
    if (!getYesNoAnswer("Confirm serve all waiting customers? (Y/N): ")) {
        cout << "Serve all cancelled. Ticket statuses remain WAITING." << endl;
        return;
    }

    int servedCount = 0;
    // Repeatedly dequeue from the front until the linked-list queue becomes empty.
    while (!waitingLine.isEmpty()) {
        Ticket servedTicket;
        if (waitingLine.serveNext(servedTicket)) {
            Ticket* ticket = findTicketByID(servedTicket.ticketID);
            if (ticket != nullptr) {
                ticket->status = TicketStatus::SERVED;
                servedCount++;
            }
        }
    }
    cout << endl << "Served " << servedCount << " customer(s) from the waiting line." << endl;
    autoSaveTickets();
}

// Display all queue nodes from front to rear, then show queue status.
void Registry::viewWaitingLine() const {
    waitingLine.showLine();
    viewQueueStatus();
}

// showFront() is a peek operation:
// it displays the front customer without removing the customer from the queue.
void Registry::viewFrontOfLine() const {
    waitingLine.showFront();
    viewQueueStatus();
}

// showRear() is also a peek operation:
// it displays the last customer without changing the queue.
void Registry::viewRearOfLine() const {
    waitingLine.showRear();
    viewQueueStatus();
}

// Assignment Part A: Check whether queue is empty/full.
// Display whether the waiting-line queue is empty or full.
void Registry::viewQueueStatus() const {
    // getCount() traverses the linked list and counts current queue nodes.
    const int queueCount = waitingLine.getCount();

    // Linked-list queues have no fixed maximum size, so this demo capacity
    // is only used to make a readable percentage bar for presentation.
    const int queueDemoCapacity = 50;
    const int barWidth = 30;
    int percentage = (queueCount * 100) / queueDemoCapacity;
    if (percentage > 100) {
        percentage = 100;
    }
    int filledBlocks = (percentage * barWidth) / 100;

    // Build an ASCII queue usage bar, for example [###.......] 30%.
    string usageBar = "[";
    for (int index = 0; index < barWidth; ++index) {
        usageBar += (index < filledBlocks) ? '#' : '.';
    }
    usageBar += "] " + to_string(percentage) + "%";

    cout << endl;
    cout << string(DisplayWidth, '-') << endl;
    printDisplayCenteredLine("Queue Status");
    cout << string(DisplayWidth, '-') << endl;
    printDisplayField("Waiting Count", to_string(queueCount) + " ticket(s)");
    printDisplayField("Capacity View", to_string(queueCount) + " / " + to_string(queueDemoCapacity) + " demo slots");
    printDisplayField("Usage", usageBar);
    printDisplayField("Waiting Empty", waitingLine.isEmpty() ? "Yes" : "No");
    printDisplayField("Queue Full", waitingLine.isFull() ? "Yes" : "No (linked list has no fixed capacity)");
    cout << string(DisplayWidth, '-') << endl;
}

// Assignment Part B: Push cancellation record into stack.
// Customer cancellation flow: remove from queue, push to cancellation stack, then autosave.
void Registry::cancelTicket() {
    int ticketID = getTicketIDFromInput();
    if (ticketID == 0) {
        cout << "Action cancelled. Returning to menu." << endl;
        return;
    }
    Ticket* ticket = findTicketByID(ticketID);
    if (ticket == nullptr) {
        cout << "Ticket ID not found in registry." << endl;
        return;
    }
    if (ticket->status == TicketStatus::CANCELLED) {
        cout << "Ticket #" << ticketID << " is already cancelled." << endl;
        return;
    }
    if (ticket->status == TicketStatus::SERVED) {
        cout << "Ticket #" << ticketID << " has already been served and cannot be cancelled." << endl;
        return;
    }

    if (waitingLine.removeTicketByID(ticketID)) {
        cout << "Removed Ticket #" << ticketID << " from waiting line for cancellation." << endl;
    }

    string reason = getValidatedText("Enter cancellation reason: ", "Cancellation reason");
    ticket->status = TicketStatus::CANCELLED;
    CancelEntry entry{ticketID, ticket->customerName, reason, getCurrentTimestamp()};
    if (cancelLog.recordCancel(entry)) {
        cout << "Ticket #" << ticketID << " status -> " << ticketStatusToString(ticket->status) << endl;
        autoSaveTickets();
        autoSaveCancellationLog();
    } else {
        ticket->status = TicketStatus::WAITING;
        cout << "Cancellation failed. Ticket #" << ticketID << " reverted to WAITING." << endl;
        waitingLine.joinLine(*ticket);
    }
}

// Assignment Part B: Undo latest cancellation using Pop.
// Undo the latest cancellation by popping from the cancellation stack.
void Registry::undoLastCancellation() {
    CancelEntry entry;
    if (!cancelLog.peekLatest(entry)) {
        return;
    }

    Ticket* ticket = findTicketByID(entry.ticketID);
    if (ticket == nullptr) {
        cout << "Error: ticket record not found while checking latest cancellation." << endl;
        return;
    }

    cout << endl << "Ticket details to restore:" << endl;
    displayTicketDetails(*ticket);
    if (!getYesNoAnswer("Confirm undo latest cancellation? (Y/N): ")) {
        cout << "Undo cancellation cancelled. Returning to menu." << endl;
        return;
    }

    if (!cancelLog.undoCancel(entry)) {
        return;
    }

    ticket = findTicketByID(entry.ticketID);
    if (ticket == nullptr) {
        cout << "Error: ticket record not found while undoing cancellation." << endl;
        return;
    }

    ticket->status = TicketStatus::WAITING;
    waitingLine.joinLine(*ticket);
    cout << "Ticket #" << ticket->ticketID << " status -> " << ticketStatusToString(ticket->status) << endl;
    autoSaveTickets();
    autoSaveCancellationLog();
}

void Registry::viewCancellationLog() const {
    if (!cancelLog.isEmpty()) {
        cancelLog.showAllCancels();
        return;
    }

    int cancelledTicketCount = 0;
    for (int index = 0; index < ticketCount; ++index) {
        if (tickets[index].status == TicketStatus::CANCELLED) {
            cancelledTicketCount++;
        }
    }

    if (cancelledTicketCount == 0) {
        cout << "Cancellation log is empty." << endl;
        return;
    }

    cout << endl;
    cout << string(DisplayWidth, '-') << endl;
    printDisplayCenteredLine("Cancellation Records (Most Recent First)");
    cout << string(DisplayWidth, '-') << endl;

    int displayNumber = 1;
    for (int index = ticketCount - 1; index >= 0; --index) {
        const Ticket& ticket = tickets[index];
        if (ticket.status == TicketStatus::CANCELLED) {
            cout << endl;
            printDisplayCenteredLine("Cancellation " + to_string(displayNumber));
            printDisplayField("Ticket ID", to_string(ticket.ticketID));
            printDisplayField("Customer", ticket.customerName);
            printDisplayField("Movie", ticket.movieTitle);
            printDisplayField("Seat", ticket.hallName + " | " + ticket.showTime + " | " + ticket.seatCode);
            printDisplayField("Reason", "Recovered from ticket record");
            printDisplayField("Cancelled At", "Loaded from ticket file");
            displayNumber++;
        }
    }
    cout << string(DisplayWidth, '-') << endl;
}

void Registry::peekLatestCancellation() const {
    CancelEntry entry;
    cancelLog.peekLatest(entry);
}

// Assignment Part C: Linear Search by Booking/Ticket ID.
// Linear search feature shown in Search Ticket submenu.
void Registry::findTicketLinear() const {
    int ticketID = getTicketIDFromInput();
    if (ticketID == 0) {
        cout << "Search cancelled. Returning to menu." << endl;
        return;
    }
    int comparisons = 0;
    int index = linearSearchIndex(ticketID, comparisons);
    if (index >= 0) {
        cout << "Linear search checked tickets one by one from the start." << endl;
        cout << "Ticket found after " << comparisons << " linear-search comparisons." << endl;
        displayTicketDetails(tickets[index]);
    } else {
        cout << "No ticket found with that ID." << endl;
    }
}

// Assignment Part C: Binary Search by Booking/Ticket ID.
// Binary search feature shown in Search Ticket submenu.
void Registry::findTicketBinary() const {
    int ticketID = getTicketIDFromInput();
    if (ticketID == 0) {
        cout << "Search cancelled. Returning to menu." << endl;
        return;
    }
    int comparisons = 0;
    int index = binarySearchIndex(ticketID, comparisons);
    if (index >= 0) {
        cout << "Binary search sorted a copy by Ticket ID before searching." << endl;
        cout << "Ticket found after " << comparisons << " binary-search comparisons." << endl;
        Ticket sortedTickets[MaxTickets];
        int sortComparisons = 0;
        int shifts = 0;
        getTicketsSortedByID(sortedTickets, sortComparisons, shifts);
        displayTicketDetails(sortedTickets[index]);
    } else {
        cout << "No ticket found with that ID." << endl;
    }
}

// Assignment Part D: Compare Linear Search and Binary Search.
// Run both search algorithms and compare their number of comparisons.
void Registry::compareLinearVsBinary() const {
    int ticketID = getTicketIDFromInput();
    if (ticketID == 0) {
        cout << "Search comparison cancelled. Returning to menu." << endl;
        return;
    }
    int linearComparisons = 0;
    int binaryComparisons = 0;
    int linearIndex = linearSearchIndex(ticketID, linearComparisons);
    int binaryIndex = binarySearchIndex(ticketID, binaryComparisons);

    cout << "\nSearch comparison for Ticket ID " << ticketID << ":" << endl;
    if (linearIndex >= 0) {
        cout << "  Linear search found the ticket after " << linearComparisons << " comparisons." << endl;
    } else {
        cout << "  Linear search did not find the ticket after " << linearComparisons << " comparisons." << endl;
    }
    if (binaryIndex >= 0) {
        cout << "  Binary search found the ticket after " << binaryComparisons << " comparisons." << endl;
    } else {
        cout << "  Binary search did not find the ticket after " << binaryComparisons << " comparisons." << endl;
    }

    cout << "  Big-O: Linear O(n), Binary O(log n) with sort overhead." << endl;
    if (linearIndex >= 0 && binaryIndex >= 0) {
        if (linearComparisons < binaryComparisons) {
            cout << "  Verdict: Linear search was faster on this dataset." << endl;
        } else if (binaryComparisons < linearComparisons) {
            cout << "  Verdict: Binary search was faster on this dataset." << endl;
        } else {
            cout << "  Verdict: Both searches used the same number of comparisons on this dataset." << endl;
        }
    } else if (linearIndex >= 0) {
        cout << "  Verdict: Only linear search found the ticket in this dataset." << endl;
    } else if (binaryIndex >= 0) {
        cout << "  Verdict: Only binary search found the ticket in this dataset." << endl;
    } else {
        cout << "  Verdict: Neither search found the ticket." << endl;
    }

    Finder::printComparisonTable(linearComparisons, binaryComparisons);
}

// Display every ticket stored in the ticket registry array.
void Registry::viewFullRegistry() const {
    if (ticketCount == 0) {
        cout << "Full ticket registry is empty." << endl;
        return;
    }

    cout << endl;
    cout << string(DisplayWidth, '-') << endl;
    printDisplayCenteredLine("Full Ticket Registry");
    cout << string(DisplayWidth, '-') << endl;
    for (int index = 0; index < ticketCount; ++index) {
        const Ticket& ticket = tickets[index];
        displayTicketDetails(ticket);
        cout << endl;
    }
}

// Display summary counts and active revenue.
void Registry::viewSystemSummary() const {
    int waitingCount = 0;
    int servedCount = 0;
    int cancelledCount = 0;
    double totalRevenue = 0.0;

    for (int index = 0; index < ticketCount; ++index) {
        const Ticket& ticket = tickets[index];
        if (ticket.status == TicketStatus::WAITING) {
            waitingCount++;
            totalRevenue += ticket.ticketPrice;
        } else if (ticket.status == TicketStatus::SERVED) {
            servedCount++;
            totalRevenue += ticket.ticketPrice;
        } else if (ticket.status == TicketStatus::CANCELLED) {
            cancelledCount++;
        }
    }

    cout << endl;
    cout << string(DisplayWidth, '-') << endl;
    printDisplayCenteredLine("System Summary");
    cout << string(DisplayWidth, '-') << endl;
    printDisplayField("Total Tickets", to_string(ticketCount));
    printDisplayField("Waiting Tickets", to_string(waitingCount));
    printDisplayField("Served Tickets", to_string(servedCount));
    printDisplayField("Cancelled Tickets", to_string(cancelledCount));
    printDisplayField("Cancellation Log", to_string(cancelLog.getCount()));

    stringstream revenueStream;
    revenueStream << fixed << setprecision(2) << "RM" << totalRevenue;
    printDisplayField("Active Revenue", revenueStream.str());
    cout << string(DisplayWidth, '-') << endl;
    printDisplayCenteredLine("Data Structure Summary");
    cout << string(DisplayWidth, '-') << endl;
    printDisplayField("Queue", "Linked List FIFO");
    printDisplayField("Stack", "Array LIFO");
    printDisplayField("Ticket Records", "Array");
    printDisplayField("Sorting", "Insertion Sort");
    printDisplayField("Searching", "Linear Search, Binary Search");
    cout << string(DisplayWidth, '-') << endl;
}

// Binary search needs sorted data, so insertion sort is used to sort ticket IDs.
// Display sorted tickets using insertion sort.
void Registry::viewTicketsSortedByID() const {
    if (ticketCount == 0) {
        cout << "No tickets available to sort." << endl;
        return;
    }

    int comparisons = 0;
    int shifts = 0;
    Ticket sortedTickets[MaxTickets];
    getTicketsSortedByID(sortedTickets, comparisons, shifts);

    printSortedTicketTable("Tickets Sorted by Ticket ID", sortedTickets, ticketCount, comparisons, shifts);
}

// Display tickets sorted alphabetically by movie title.
void Registry::viewTicketsSortedByMovie() const {
    if (ticketCount == 0) {
        cout << "No tickets available to sort." << endl;
        return;
    }

    int comparisons = 0;
    int shifts = 0;
    Ticket sortedTickets[MaxTickets];
    getTicketsSortedByMovie(sortedTickets, comparisons, shifts);

    printSortedTicketTable("Tickets Sorted by Movie", sortedTickets, ticketCount, comparisons, shifts);
}

// Display tickets sorted by hall and showtime.
void Registry::viewTicketsSortedByHall() const {
    if (ticketCount == 0) {
        cout << "No tickets available to sort." << endl;
        return;
    }

    int comparisons = 0;
    int shifts = 0;
    Ticket sortedTickets[MaxTickets];
    getTicketsSortedByHall(sortedTickets, comparisons, shifts);

    printSortedTicketTable("Tickets Sorted by Hall", sortedTickets, ticketCount, comparisons, shifts);
}

// Display tickets sorted by ticket type.
void Registry::viewTicketsSortedByType() const {
    if (ticketCount == 0) {
        cout << "No tickets available to sort." << endl;
        return;
    }

    int comparisons = 0;
    int shifts = 0;
    Ticket sortedTickets[MaxTickets];
    getTicketsSortedByType(sortedTickets, comparisons, shifts);

    printSortedTicketTable("Tickets Sorted by Ticket Type", sortedTickets, ticketCount, comparisons, shifts);
}

// Display tickets sorted by ticket status.
void Registry::viewTicketsSortedByStatus() const {
    if (ticketCount == 0) {
        cout << "No tickets available to sort." << endl;
        return;
    }

    int comparisons = 0;
    int shifts = 0;
    Ticket sortedTickets[MaxTickets];
    getTicketsSortedByStatus(sortedTickets, comparisons, shifts);

    printSortedTicketTable("Tickets Sorted by Status", sortedTickets, ticketCount, comparisons, shifts);
}

// Staff display of screening catalog, including halls for management.
void Registry::viewMovieList() const {
    movieCatalog.showMovieSummary();
}

// Customer-friendly movie display.
void Registry::viewTodaysMovies() const {
    movieCatalog.showTodaysMovies();
}

// Let customers check available seats before booking.
void Registry::viewSeatMap() const {
    int movieNumber = getScreeningNumberFromSelection();
    if (movieNumber == 0) {
        cout << "Seat map view cancelled. Returning to menu." << endl;
        return;
    }
    string movieTitle = movieCatalog.getMovieTitle(movieNumber);
    string hallName = movieCatalog.getHallName(movieNumber);
    string showTime = movieCatalog.getShowTime(movieNumber);

    cout << "Movie: " << movieTitle << " (" << showTime << ")" << endl;
    displaySeatMapForHall(hallName, showTime);
}

// Staff adds movie information into the movie catalog array.
void Registry::addMovie() {
    string previousMovieTitle;
    string previousGenre;
    string previousClassification;
    string previousLanguage;
    string previousDuration;
    bool addingMovie = true;

    while (addingMovie) {
        string movieTitle;
        string genre;
        string classification;
        string language;
        string duration;

        if (isValidText(previousMovieTitle)
            && getYesNoAnswer("Use same movie details as previous screening? (Y/N): ")) {
            movieTitle = previousMovieTitle;
            genre = previousGenre;
            classification = previousClassification;
            language = previousLanguage;
            duration = previousDuration;
        } else {
            movieTitle = getValidatedText("Enter new movie title, or 0 to cancel: ", "Movie title");
            if (movieTitle == "0") {
                cout << "Insert movie cancelled. Returning to menu." << endl;
                return;
            }
            genre = getValidatedText("Enter genre: ", "Genre");
            classification = getValidatedClassification();
            language = getValidatedText("Enter language: ", "Language");
            duration = getValidatedDuration();
        }

        string hallName = getValidatedHallName();
        string showTime = getValidatedShowTime();

        if (movieCatalog.addMovie(movieTitle, genre, classification, language, duration, hallName, showTime)) {
            cout << "Movie added successfully: " << movieTitle << " (" << hallName
                 << ", " << showTime << ")" << endl;
            autoSaveMovieCatalog();
            previousMovieTitle = movieTitle;
            previousGenre = genre;
            previousClassification = classification;
            previousLanguage = language;
            previousDuration = duration;
        }

        addingMovie = getYesNoAnswer("Add another screening? (Y/N): ");
    }
}

// Staff edits an existing screening if no active tickets depend on it.
void Registry::editMovie() {
    movieCatalog.showMovieSummary();
    if (movieCatalog.getMovieCount() == 0) {
        return;
    }

    cout << endl << "Enter movie number to edit, or 0 to cancel: ";
    int selectedMovieNumber;
    if (!(cin >> selectedMovieNumber)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Please enter a movie number." << endl;
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (selectedMovieNumber == 0) {
        cout << "Edit movie cancelled. Returning to menu." << endl;
        return;
    }

    if (!movieCatalog.isValidUniqueMovieNumber(selectedMovieNumber)) {
        cout << "Invalid movie number. Please select from the movie list." << endl;
        return;
    }

    string movieTitleForShowtimes = movieCatalog.getUniqueMovieTitle(selectedMovieNumber);
    movieCatalog.showShowtimesForMovie(movieTitleForShowtimes);
    cout << endl << "Select showtime number to edit, or 0 to cancel: ";
    int showtimeNumber;
    if (!(cin >> showtimeNumber)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Please enter a showtime number." << endl;
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (showtimeNumber == 0) {
        cout << "Edit movie cancelled. Returning to menu." << endl;
        return;
    }

    int movieNumber = movieCatalog.getScreeningNumberForMovieShowtime(movieTitleForShowtimes, showtimeNumber);
    if (movieNumber == 0) {
        cout << "Invalid showtime number. Please select from the showtime list." << endl;
        return;
    }

    string currentMovieTitle = movieCatalog.getMovieTitle(movieNumber);
    string currentHallName = movieCatalog.getHallName(movieNumber);
    string currentShowTime = movieCatalog.getShowTime(movieNumber);
    if (screeningHasActiveTickets(currentMovieTitle, currentHallName, currentShowTime)) {
        cout << "Cannot edit movie: active tickets exist for " << currentMovieTitle
             << " at " << currentShowTime << "." << endl;
        return;
    }

    cout << "Press Enter without typing anything to keep the current value." << endl;
    string movieTitle = getOptionalValidatedText("New movie title: ", currentMovieTitle, "movie title");
    string genre = getOptionalValidatedText("New genre: ", movieCatalog.getGenre(movieNumber), "genre");
    string classification = getOptionalClassification(movieCatalog.getClassification(movieNumber));
    string language = getOptionalValidatedText("New language: ", movieCatalog.getLanguage(movieNumber), "language");
    string duration = getOptionalDuration(movieCatalog.getDuration(movieNumber));
    string hallName = getOptionalHallName(currentHallName);
    string showTime = getOptionalShowTime(currentShowTime);

    if (movieCatalog.updateMovie(movieNumber, movieTitle, genre, classification, language, duration, hallName, showTime)) {
        cout << "Movie updated successfully." << endl;
        autoSaveMovieCatalog();
    }
}

// Staff deletes a movie if no active tickets depend on it.
void Registry::deleteMovie() {
    movieCatalog.showMovieSummary();
    if (movieCatalog.getMovieCount() == 0) {
        return;
    }

    cout << endl << "Enter movie number to delete, or 0 to cancel: ";
    int selectedMovieNumber;
    if (!(cin >> selectedMovieNumber)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Please enter a movie number." << endl;
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (selectedMovieNumber == 0) {
        cout << "Delete movie cancelled. Returning to menu." << endl;
        return;
    }

    if (!movieCatalog.isValidUniqueMovieNumber(selectedMovieNumber)) {
        cout << "Invalid movie number. Please select from the movie list." << endl;
        return;
    }

    string movieTitleForShowtimes = movieCatalog.getUniqueMovieTitle(selectedMovieNumber);
    movieCatalog.showShowtimesForMovie(movieTitleForShowtimes);
    cout << endl << "Select showtime number to delete, or 0 to cancel: ";
    int showtimeNumber;
    if (!(cin >> showtimeNumber)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Please enter a showtime number." << endl;
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (showtimeNumber == 0) {
        cout << "Delete movie cancelled. Returning to menu." << endl;
        return;
    }

    int movieNumber = movieCatalog.getScreeningNumberForMovieShowtime(movieTitleForShowtimes, showtimeNumber);
    if (movieNumber == 0) {
        cout << "Invalid showtime number. Please select from the showtime list." << endl;
        return;
    }

    string movieTitle = movieCatalog.getMovieTitle(movieNumber);
    string hallName = movieCatalog.getHallName(movieNumber);
    string showTime = movieCatalog.getShowTime(movieNumber);
    if (screeningHasActiveTickets(movieTitle, hallName, showTime)) {
        cout << "Cannot delete movie: active tickets exist for " << movieTitle
             << " at " << showTime << "." << endl;
        return;
    }

    if (movieCatalog.removeMovie(movieNumber)) {
        cout << "Movie deleted successfully: " << movieTitle << endl;
        autoSaveMovieCatalog();
    }
}

// Staff edits ticket information and keeps queue data synchronized.
void Registry::editTicketInfo() {
    int ticketID = getTicketIDFromInput();
    if (ticketID == 0) {
        cout << "Edit ticket cancelled. Returning to menu." << endl;
        return;
    }
    Ticket* ticket = findTicketByID(ticketID);
    if (ticket == nullptr) {
        cout << "Ticket ID not found in registry." << endl;
        return;
    }
    if (ticket->status == TicketStatus::CANCELLED) {
        cout << "Cancelled tickets cannot be edited. Undo the cancellation first if needed." << endl;
        return;
    }

    cout << "Current ticket details:" << endl;
    displayTicketDetails(*ticket);
    cout << "Press Enter without typing anything to keep the current value." << endl;

    string newCustomerName = getOptionalText("New customer name: ");
    if (isValidText(newCustomerName)) {
        ticket->customerName = newCustomerName;
    }

    int updatedMovieNumber = getOptionalMovieNumberFromSelection();
    string updatedMovieTitle = ticket->movieTitle;
    string updatedGenre = ticket->genre;
    string updatedClassification = ticket->classification;
    string updatedLanguage = ticket->language;
    string updatedDuration = ticket->duration;
    string updatedHallName = ticket->hallName;
    string updatedShowTime = ticket->showTime;
    if (updatedMovieNumber != 0) {
        updatedMovieTitle = movieCatalog.getMovieTitle(updatedMovieNumber);
        updatedGenre = movieCatalog.getGenre(updatedMovieNumber);
        updatedClassification = movieCatalog.getClassification(updatedMovieNumber);
        updatedLanguage = movieCatalog.getLanguage(updatedMovieNumber);
        updatedDuration = movieCatalog.getDuration(updatedMovieNumber);
        updatedHallName = movieCatalog.getHallName(updatedMovieNumber);
        updatedShowTime = movieCatalog.getShowTime(updatedMovieNumber);
    }

    cout << endl;
    printDisplayCenteredLine("Seat Map for Selected Screening");
    displaySeatMapForHall(updatedHallName, updatedShowTime);
    cout << endl;
    string newSeatCode = getOptionalText("New seat code: ");
    string updatedSeatCode = ticket->seatCode;
    if (isValidText(newSeatCode)) {
        if (!isValidSeatCode(newSeatCode) || !seatCodeInsideSeatMap(newSeatCode)) {
            cout << "Cannot update ticket: invalid seat. Please choose a seat from A01 to K18." << endl;
            return;
        }
        char rowLetter = static_cast<char>(toupper(static_cast<unsigned char>(newSeatCode[0])));
        int seatNumber = 0;
        stringstream seatStream(newSeatCode.substr(1));
        seatStream >> seatNumber;
        updatedSeatCode = "";
        updatedSeatCode += rowLetter;
        if (seatNumber < 10) {
            updatedSeatCode += "0";
        }
        updatedSeatCode += to_string(seatNumber);
    }
    if (seatCodeTakenByOtherTicket(updatedHallName, updatedShowTime, updatedSeatCode, ticketID)) {
        cout << "Cannot update ticket: Seat " << updatedSeatCode << " is already taken in "
             << updatedHallName << "." << endl;
        return;
    }

    ticket->movieTitle = updatedMovieTitle;
    ticket->genre = updatedGenre;
    ticket->classification = updatedClassification;
    ticket->language = updatedLanguage;
    ticket->duration = updatedDuration;
    ticket->hallName = updatedHallName;
    ticket->showTime = updatedShowTime;
    ticket->seatCode = updatedSeatCode;

    if (isDoubleSeat(updatedSeatCode)) {
        ticket->ticketType = "Twin Seat";
        ticket->ticketPrice = 30.00;
        cout << "Twin seat selected. Ticket type automatically set to Twin Seat - RM30.00." << endl;
    } else if (isWheelchairSeat(updatedSeatCode)) {
        ticket->ticketType = "Wheelchair Seat";
        ticket->ticketPrice = 18.00;
        cout << "Wheelchair seat selected. Ticket type automatically set to Wheelchair Seat - RM18.00." << endl;
    } else {
        if (ticket->ticketType == "Twin Seat" || ticket->ticketType == "Wheelchair Seat") {
            cout << "Normal seat selected. Please choose a normal ticket type." << endl;
            getTicketTypeFromSelection(ticket->ticketType, ticket->ticketPrice);
        } else {
            string updateTicketType = getOptionalText("Update ticket type? Enter Y for yes, or press Enter to keep current type: ");
            string updateAnswer = toLowerText(updateTicketType);
            if (updateAnswer == "y" || updateAnswer == "yes") {
                getTicketTypeFromSelection(ticket->ticketType, ticket->ticketPrice);
            }
        }
    }

    if (ticket->status == TicketStatus::WAITING) {
        waitingLine.updateTicketByID(*ticket);
    }
    cout << "Ticket #" << ticketID << " updated successfully." << endl;
    autoSaveTickets();
}

// Manual ticket save with user-selected file name.
void Registry::saveTicketsToTextFile() const {
    string fileName = getValidatedText("Enter text file name to save: ", "File name");
    saveTicketsToFile(fileName, true);
}

// Load ticket records from a text file into the ticket array, then rebuild the queue.
bool Registry::loadTicketsFromFile(const string& fileName, bool showMessage, bool autoSaveAfterLoad) {
    ifstream inputFile(fileName);
    if (!inputFile) {
        if (showMessage) {
            cout << "Unable to open file for loading." << endl;
        }
        return false;
    }

    Ticket loadedTickets[MaxTickets];
    string line;
    int lineNumber = 0;
    int loadedCount = 0;
    int skippedCount = 0;

    while (getline(inputFile, line)) {
        lineNumber++;
        if (lineNumber == 1 && line == "TicketID|CustomerName|MovieTitle|Genre|Classification|Language|Duration|Hall|Showtime|SeatCode|TicketType|TicketPrice|Status") {
            continue;
        }
        if (!isValidText(line)) {
            continue;
        }

        stringstream lineStream(line);
        string idText;
        string customerName;
        string movieTitle;
        string genre;
        string classification;
        string language;
        string duration;
        string hallName;
        string showTime;
        string seatCode;
        string ticketType;
        string ticketPriceText;
        string statusText;

        getline(lineStream, idText, '|');
        getline(lineStream, customerName, '|');
        getline(lineStream, movieTitle, '|');
        getline(lineStream, genre, '|');
        getline(lineStream, classification, '|');
        getline(lineStream, language, '|');
        getline(lineStream, duration, '|');
        getline(lineStream, hallName, '|');
        getline(lineStream, showTime, '|');
        getline(lineStream, seatCode, '|');
        getline(lineStream, ticketType, '|');
        getline(lineStream, ticketPriceText, '|');
        getline(lineStream, statusText, '|');

        string extraText;
        bool hasExtraColumn = static_cast<bool>(getline(lineStream, extraText, '|'));
        TicketStatus status;
        int ticketID = 0;
        double ticketPrice = 0.0;
        stringstream idStream(idText);
        stringstream priceStream(ticketPriceText);
        string normalizedDuration;

        bool invalidLine = hasExtraColumn
            || !(idStream >> ticketID)
            || !(priceStream >> ticketPrice)
            || ticketID <= 0
            || ticketPrice <= 0.0
            || !isValidText(customerName)
            || !isValidText(movieTitle)
            || !isValidText(genre)
            || !isValidText(classification)
            || !isValidText(language)
            || !normalizeDurationText(duration, normalizedDuration)
            || !isValidText(hallName)
            || !isValidText(showTime)
            || !isValidText(seatCode)
            || !isValidSeatCode(seatCode)
            || !isValidText(ticketType)
            || !stringToTicketStatus(statusText, status);

        if (!invalidLine) {
            for (int index = 0; index < loadedCount; ++index) {
                const Ticket& loadedTicket = loadedTickets[index];
                if (loadedTicket.ticketID == ticketID) {
                    invalidLine = true;
                    break;
                }
                if (loadedTicket.status != TicketStatus::CANCELLED
                    && status != TicketStatus::CANCELLED
                    && toLowerText(loadedTicket.hallName) == toLowerText(hallName)
                    && toLowerText(loadedTicket.seatCode) == toLowerText(seatCode)) {
                    invalidLine = true;
                    break;
                }
            }
        }

        if (invalidLine) {
            skippedCount++;
            continue;
        }

        if (loadedCount >= MaxTickets) {
            skippedCount++;
            continue;
        }

        if (isDoubleSeat(seatCode)) {
            ticketType = "Twin Seat";
            ticketPrice = 30.00;
        } else if (isWheelchairSeat(seatCode)) {
            ticketType = "Wheelchair Seat";
            ticketPrice = 18.00;
        }

        loadedTickets[loadedCount] = Ticket{ticketID, customerName, movieTitle, genre, classification, language, hallName, showTime, normalizedDuration, seatCode, ticketType, ticketPrice, status};
        loadedCount++;
    }

    for (int index = 0; index < loadedCount; ++index) {
        tickets[index] = loadedTickets[index];
    }
    ticketCount = loadedCount;
    rebuildCancellationLogFromCancelledTickets();
    rebuildWaitingLine();

    if (showMessage) {
        cout << "Loaded " << loadedCount << " ticket(s) from " << fileName << "." << endl;
    }
    if (showMessage && skippedCount > 0) {
        cout << "Skipped " << skippedCount << " invalid row(s)." << endl;
    }
    if (showMessage) {
        cout << "Cancellation log rebuilt from cancelled ticket records." << endl;
    }
    if (autoSaveAfterLoad) {
        autoSaveTickets();
        autoSaveCancellationLog();
    }
    return true;
}

// Manual ticket load from text file; invalid rows are skipped.
void Registry::loadTicketsFromTextFile() {
    string fileName = getValidatedText("Enter text file name to load: ", "File name");
    loadTicketsFromFile(fileName, true, true);
}

// Manual movie catalog save with user-selected file name.
void Registry::saveMovieCatalogToTextFile() const {
    string fileName = getValidatedText("Enter movie catalog file name to save: ", "File name");
    saveMovieCatalogToFile(fileName, true);
}

// Load movie catalog records from a text file into the movie catalog array.
bool Registry::loadMovieCatalogFromFile(const string& fileName, bool showMessage) {
    ifstream inputFile(fileName);
    if (!inputFile) {
        if (showMessage) {
            cout << "Unable to open movie catalog file for loading." << endl;
        }
        return false;
    }

    MovieCatalog loadedCatalog;
    loadedCatalog.clearMovies();

    string line;
    int lineNumber = 0;
    int loadedCount = 0;
    int skippedCount = 0;

    while (getline(inputFile, line)) {
        lineNumber++;
        if (lineNumber == 1 && line == "MovieTitle|Genre|Classification|Language|Duration|Hall|Showtime") {
            continue;
        }
        if (!isValidText(line)) {
            continue;
        }

        stringstream lineStream(line);
        string movieTitle;
        string genre;
        string classification;
        string language;
        string duration;
        string hallName;
        string showTime;
        string extraText;

        getline(lineStream, movieTitle, '|');
        getline(lineStream, genre, '|');
        getline(lineStream, classification, '|');
        getline(lineStream, language, '|');
        getline(lineStream, duration, '|');
        getline(lineStream, hallName, '|');
        getline(lineStream, showTime, '|');
        bool hasExtraColumn = static_cast<bool>(getline(lineStream, extraText, '|'));
        string normalizedDuration;

        if (hasExtraColumn || !isValidText(movieTitle) || !isValidText(genre)
            || !isValidText(classification) || !isValidText(language)
            || !normalizeDurationText(duration, normalizedDuration)
            || !isValidText(hallName) || !isValidText(showTime)) {
            skippedCount++;
            continue;
        }

        if (loadedCatalog.addMovie(movieTitle, genre, classification, language, normalizedDuration, hallName, showTime)) {
            loadedCount++;
        } else {
            skippedCount++;
        }
    }

    movieCatalog = loadedCatalog;
    if (showMessage) {
        cout << "Loaded " << loadedCount << " movie(s) from " << fileName << "." << endl;
    }
    if (showMessage && skippedCount > 0) {
        cout << "Skipped " << skippedCount << " invalid movie row(s)." << endl;
    }
    return true;
}

// Manual movie catalog load. It is blocked when tickets already exist to avoid mismatched data.
void Registry::loadMovieCatalogFromTextFile() {
    if (ticketCount > 0) {
        cout << "Cannot load movie catalog while ticket registry is not empty." << endl;
        return;
    }

    string fileName = getValidatedText("Enter movie catalog file name to load: ", "File name");
    if (!loadMovieCatalogFromFile(fileName, true)) {
        return;
    }
    autoSaveMovieCatalog();
}

// Manual cancellation log save with user-selected file name.
void Registry::saveCancellationLogToTextFile() const {
    string fileName = getValidatedText("Enter cancellation log file name to save: ", "File name");
    cancelLog.saveToFile(fileName, true);
}

// Manual cancellation log load, then refresh the autosave copy.
void Registry::loadCancellationLogFromTextFile() {
    string fileName = getValidatedText("Enter cancellation log file name to load: ", "File name");
    if (cancelLog.loadFromFile(fileName, true)) {
        autoSaveCancellationLog();
    }
}

// Export a formatted report containing summary, revenue, and sorted ticket records.
void Registry::exportReport() const {
    string fileName = getValidatedText("Enter report file name to export: ", "File name");
    ofstream outputFile(fileName);
    if (!outputFile) {
        cout << "Unable to open report file." << endl;
        return;
    }

    int waitingCount = 0;
    int servedCount = 0;
    int cancelledCount = 0;
    double totalRevenue = 0.0;
    for (int index = 0; index < ticketCount; ++index) {
        const Ticket& ticket = tickets[index];
        if (ticket.status == TicketStatus::WAITING) {
            waitingCount++;
            totalRevenue += ticket.ticketPrice;
        } else if (ticket.status == TicketStatus::SERVED) {
            servedCount++;
            totalRevenue += ticket.ticketPrice;
        } else if (ticket.status == TicketStatus::CANCELLED) {
            cancelledCount++;
        }
    }

    int comparisons = 0;
    int shifts = 0;
    Ticket sortedTickets[MaxTickets];
    getTicketsSortedByID(sortedTickets, comparisons, shifts);
    int sampleTicketID = 0;
    int linearComparisons = 0;
    int binaryComparisons = 0;
    int linearIndex = -1;
    int binaryIndex = -1;
    if (ticketCount > 0) {
        sampleTicketID = sortedTickets[ticketCount - 1].ticketID;
        linearIndex = linearSearchIndex(sampleTicketID, linearComparisons);
        binaryIndex = binarySearchIndex(sampleTicketID, binaryComparisons);
    }

    outputFile << "Cinema Ticket Booking System Report" << endl;
    outputFile << "Generated At: " << getCurrentTimestamp() << endl << endl;
    outputFile << "System Summary" << endl;
    outputFile << "Total tickets: " << ticketCount << endl;
    outputFile << "Waiting tickets: " << waitingCount << endl;
    outputFile << "Served tickets: " << servedCount << endl;
    outputFile << "Cancelled tickets: " << cancelledCount << endl;
    outputFile << "Cancellations in log: " << cancelLog.getCount() << endl << endl;
    outputFile << fixed << setprecision(2);
    outputFile << "Total active revenue: RM" << totalRevenue << endl << endl;

    outputFile << "Tickets Sorted by Ticket ID" << endl;
    if (ticketCount == 0) {
        outputFile << "No tickets available." << endl;
    } else {
        for (int index = 0; index < ticketCount; ++index) {
            const Ticket& ticket = sortedTickets[index];
            printTicketLine(ticket, outputFile);
            outputFile << endl;
        }
    }
    outputFile << endl;
    outputFile << "Searching Algorithm Comparison" << endl;
    outputFile << "Linear Search: O(n), checks records one by one and does not need sorted data." << endl;
    outputFile << "Binary Search: O(log n), faster on sorted data but requires ticket IDs to be sorted first." << endl;
    if (ticketCount > 0) {
        outputFile << "Sample Ticket ID tested: " << sampleTicketID << endl;
        outputFile << "Linear search comparisons: " << linearComparisons
                   << (linearIndex >= 0 ? " (found)" : " (not found)") << endl;
        outputFile << "Binary search comparisons: " << binaryComparisons
                   << (binaryIndex >= 0 ? " (found)" : " (not found)") << endl;
    } else {
        outputFile << "No ticket records available for comparison count testing." << endl;
    }
    outputFile << "Linear advantage: simple and works on unsorted ticket records." << endl;
    outputFile << "Linear disadvantage: slower when ticket records become large." << endl;
    outputFile << "Binary advantage: fewer comparisons on sorted ticket IDs." << endl;
    outputFile << "Binary disadvantage: ticket IDs must be sorted before searching." << endl;
    outputFile << "For small unsorted data, linear search is simpler. For large sorted data, binary search performs better." << endl << endl;

    outputFile << "Sorting Algorithm: Insertion Sort" << endl;
    outputFile << "Insertion sort comparisons: " << comparisons << endl;
    outputFile << "Insertion sort shifts: " << shifts << endl;

    cout << "Report exported to " << fileName << "." << endl;
}
