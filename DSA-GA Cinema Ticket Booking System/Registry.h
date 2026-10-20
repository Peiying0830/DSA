#ifndef REGISTRY_H
#define REGISTRY_H

#include "CancelLog.h"
#include "Finder.h"
#include "MovieCatalog.h"
#include "Ticket.h"
#include "WaitingLine.h"

#include <iostream>
#include <string>

using namespace std;

// Assignment controller class.
// It combines Part A Queue, Part B Stack, Part C Searching, Part D Comparison,
// validation, file handling, and report generation.
// Registry is the main controller class.
// It connects the ticket array, waiting-line queue, cancellation stack, and movie catalog.
class Registry {
public:
    // Maximum number of ticket records stored in the ticket registry array.
    static const int MaxTickets = 100;

    Registry();

    // Customer booking workflow.
    void browseMoviesAndBook();
    void joinWaitingLine();

    // Assignment Part A queue operations.
    void serveNextCustomer();
    void serveAllCustomers();
    void viewWaitingLine() const;
    void viewFrontOfLine() const;
    void viewRearOfLine() const;
    void viewQueueStatus() const;

    // Assignment Part B stack operations for cancellation.
    void cancelTicket();
    void undoLastCancellation();
    void viewCancellationLog() const;
    void peekLatestCancellation() const;

    // Assignment Part C and Part D searching/comparison operations.
    void findTicketLinear() const;
    void findTicketBinary() const;
    void compareLinearVsBinary() const;

    // Record display and sorting operations.
    void viewFullRegistry() const;
    void viewSystemSummary() const;
    void viewTicketsSortedByID() const;
    void viewTicketsSortedByMovie() const;
    void viewTicketsSortedByHall() const;
    void viewTicketsSortedByType() const;
    void viewTicketsSortedByStatus() const;

    // Movie catalog operations for Customer and Staff/Admin.
    void viewMovieList() const;
    void viewTodaysMovies() const;
    void viewSeatMap() const;
    void addMovie();
    void editMovie();
    void deleteMovie();
    void editTicketInfo();

    // File handling and report export operations.
    void saveTicketsToTextFile() const;
    void loadTicketsFromTextFile();
    void saveMovieCatalogToTextFile() const;
    void loadMovieCatalogFromTextFile();
    void saveCancellationLogToTextFile() const;
    void loadCancellationLogFromTextFile();
    void exportReport() const;

private:
    // Ticket registry implemented using array + count.
    Ticket tickets[MaxTickets];
    int ticketCount;

    // Main data structure modules used by the system.
    WaitingLine waitingLine;
    CancelLog cancelLog;
    MovieCatalog movieCatalog;

    // Validation and lookup helper functions.
    bool ticketIDExists(int ticketID) const;
    bool seatCodeTaken(const string& hallName, const string& showTime, const string& seatCode) const;
    bool seatCodeTakenByOtherTicket(const string& hallName, const string& showTime, const string& seatCode, int currentTicketID) const;
    bool seatCodeInsideSeatMap(const string& seatCode) const;
    bool isDoubleSeat(const string& seatCode) const;
    bool isWheelchairSeat(const string& seatCode) const;
    bool screeningHasActiveTickets(const string& movieTitle, const string& hallName, const string& showTime) const;
    Ticket* findTicketByID(int ticketID);
    const Ticket* findTicketByID(int ticketID) const;

    // Searching and sorting helper functions.
    int linearSearchIndex(int ticketID, int& comparisons) const;
    int binarySearchIndex(int ticketID, int& comparisons) const;
    void getTicketsSortedByID(Ticket sortedTickets[], int& comparisons, int& shifts) const;
    void getTicketsSortedByMovie(Ticket sortedTickets[], int& comparisons, int& shifts) const;
    void getTicketsSortedByHall(Ticket sortedTickets[], int& comparisons, int& shifts) const;
    void getTicketsSortedByType(Ticket sortedTickets[], int& comparisons, int& shifts) const;
    void getTicketsSortedByStatus(Ticket sortedTickets[], int& comparisons, int& shifts) const;
    void displayTicketDetails(const Ticket& ticket) const;

    // Input validation helper functions.
    int getValidatedTicketID(const string& prompt) const;
    int generateTicketID() const;
    int getScreeningNumberFromSelection() const;
    int getOptionalMovieNumberFromSelection() const;
    string getValidatedSeatCode() const;
    void getTicketTypeFromSelection(string& ticketType, double& ticketPrice) const;
    string getValidatedText(const string& prompt, const string& fieldName) const;
    string getValidatedClassification() const;
    string getValidatedHallName() const;
    string getValidatedDuration() const;
    string getValidatedShowTime() const;
    string getOptionalClassification(const string& currentClassification) const;
    string getOptionalHallName(const string& currentHallName) const;
    string getOptionalDuration(const string& currentDuration) const;
    string getOptionalShowTime(const string& currentShowTime) const;
    string getOptionalValidatedText(const string& prompt, const string& currentValue, const string& fieldName) const;
    string getOptionalText(const string& prompt) const;
    bool getYesNoAnswer(const string& prompt) const;
    int getTicketIDFromInput() const;
    int getBookingQuantity() const;

    // Booking and queue rebuild helper functions.
    void bookSelectedScreening(int movieNumber, bool showSeatMap);
    void addTicketToRegistry(const Ticket& ticket);
    void rebuildWaitingLine();
    void rebuildCancellationLogFromCancelledTickets();
    void printTicketLine(const Ticket& ticket, ostream& output) const;
    void printReceipt(const Ticket& ticket) const;
    void printGroupReceipt(const Ticket bookedTickets[], int bookedCount) const;
    void displaySeatMapForHall(const string& hallName, const string& showTime) const;

    // Shared file handling helper functions.
    bool saveTicketsToFile(const string& fileName, bool showMessage) const;
    bool saveMovieCatalogToFile(const string& fileName, bool showMessage) const;
    bool loadTicketsFromFile(const string& fileName, bool showMessage, bool autoSaveAfterLoad);
    bool loadMovieCatalogFromFile(const string& fileName, bool showMessage);
    void autoSaveTickets() const;
    void autoSaveMovieCatalog() const;
    void autoSaveCancellationLog() const;
};

#endif // REGISTRY_H
