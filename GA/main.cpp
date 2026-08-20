#include "Registry.h"

#include <iostream>
#include <limits>
#include <string>

using namespace std;

const string AdminPassword = "admin123";
const int MenuWidth = 100;

// main.cpp is responsible for user interface flow only.
// The real booking, queue, stack, search, and file logic is handled by Registry.

// Print a centered title within the fixed console menu width.
void printCenteredTitle(const string& title) {
    int leftPadding = (MenuWidth - static_cast<int>(title.length())) / 2;
    if (leftPadding < 0) {
        leftPadding = 0;
    }
    cout << string(leftPadding, ' ') << title << endl;
}

// Print the same separator width for all menus.
void printSeparator(char symbol) {
    cout << string(MenuWidth, symbol) << endl;
}

// Display the first message shown when the program starts.
void displayWelcomeMessage() {
    cout << endl;
    printSeparator('=');
    printCenteredTitle("Cinema Ticket Booking System");
    printSeparator('=');
    printCenteredTitle("Welcome to our cinema service.");
    printCenteredTitle("Manage bookings, queues, and searches.");
}

// First-level menu for RBAC role selection.
void displayRoleMenu() {
    cout << endl;
    printSeparator('-');
    printCenteredTitle("Select User Role");
    printSeparator('-');
    cout << "1. Customer" << endl;
    cout << "2. Staff / Admin" << endl;
    cout << "3. Exit" << endl;
    cout << endl << "Select an option: ";
}

// Customer menu contains public booking actions from the assignment scenario.
void displayCustomerMenu() {
    cout << endl;
    printSeparator('-');
    printCenteredTitle("Customer Menu");
    printSeparator('-');
    cout << "1. Browse Movies / Book Ticket" << endl;
    cout << "2. Search Ticket" << endl;
    cout << "3. Cancel Ticket" << endl;
    cout << "4. About System" << endl;
    cout << "5. Back" << endl;
    cout << endl << "Select an option: ";
}

// Short customer-facing explanation of the system functions.
void displayAboutSystem() {
    cout << endl;
    printSeparator('-');
    printCenteredTitle("About System");
    printSeparator('-');
    cout << "System Name : Cinema Ticket Booking System" << endl;
    cout << "Version     : 1.0" << endl;
    cout << "User Roles  : Customer and Staff/Admin" << endl;
    cout << "Data Files  : tickets_autosave.txt, movies_autosave.txt," << endl;
    cout << "              cancellations_autosave.txt" << endl;
    cout << endl;
    cout << "Customer Functions" << endl;
    cout << "- Browse today's movies, showtimes, and seat maps." << endl;
    cout << "- Book one or more tickets with auto-generated ticket IDs." << endl;
    cout << "- Search ticket details and cancel bookings." << endl;
    cout << endl;
    cout << "Staff/Admin Functions" << endl;
    cout << "- Insert, view, edit, and delete movie screenings." << endl;
    cout << "- Serve customers from the waiting-line queue." << endl;
    cout << "- View records, summaries, cancellation logs, and sorted tickets." << endl;
    cout << "- Load/save text files and export system reports." << endl;
    cout << endl;
    cout << "Data Structure Features" << endl;
    cout << "- Queue: waiting line using linked list." << endl;
    cout << "- Stack: cancellation log using array." << endl;
    cout << "- Searching: linear search, binary search, and comparison table." << endl;
    cout << "- Sorting: insertion sort by Ticket ID." << endl;
}

// Staff/Admin menu contains management features for queue, stack, search, records, and files.
void displayStaffMenu() {
    cout << endl;
    printSeparator('-');
    printCenteredTitle("Staff / Admin Menu");
    printSeparator('-');
    cout << "1. Movie Management" << endl;
    cout << "2. Serve Customers" << endl;
    cout << "3. View Records" << endl;
    cout << "4. Search Ticket" << endl;
    cout << "5. Edit Ticket Info" << endl;
    cout << "6. Undo Last Cancellation" << endl;
    cout << "7. File Management" << endl;
    cout << "8. Back" << endl;
    cout << endl << "Select an option: ";
}

void displayServeMenu() {
    // Staff can serve one customer or process the whole queue after confirmation.
    cout << endl;
    printSeparator('-');
    printCenteredTitle("Serve Customers");
    printSeparator('-');
    cout << "1. Serve Next Customer" << endl;
    cout << "2. Serve All Waiting Customers" << endl;
    cout << "3. Back" << endl;
    cout << endl << "Select an option: ";
}

void displaySearchMenu() {
    // Search options are kept in a submenu because Part C and Part D are staff/demo features.
    cout << endl;
    printSeparator('-');
    printCenteredTitle("Search Ticket");
    printSeparator('-');
    cout << "1. Linear Search" << endl;
    cout << "2. Binary Search" << endl;
    cout << "3. Compare Linear vs Binary Search" << endl;
    cout << "4. Back" << endl;
    cout << endl << "Select an option: ";
}

void displayRecordsMenu() {
    // Record views group all display-only features, including queue, stack, summary, and sorting.
    cout << endl;
    printSeparator('-');
    printCenteredTitle("View Records");
    printSeparator('-');
    cout << "1. Waiting Line Details" << endl;
    cout << "2. Front Waiting Customer" << endl;
    cout << "3. Rear Waiting Customer" << endl;
    cout << "4. Queue Status" << endl;
    cout << "5. Cancellation Records" << endl;
    cout << "6. Latest Cancellation" << endl;
    cout << "7. Full Ticket Registry" << endl;
    cout << "8. System Summary" << endl;
    cout << "9. Sorted Ticket Records" << endl;
    cout << "10. Today's Movies" << endl;
    cout << "11. Back" << endl;
    cout << endl << "Select an option: ";
}

void displaySortingMenu() {
    // Sorting submenu keeps the record menu shorter and lets staff choose the sorting key.
    cout << endl;
    printSeparator('-');
    printCenteredTitle("Sort Ticket Records");
    printSeparator('-');
    cout << "1. Sort by Ticket ID" << endl;
    cout << "2. Sort by Movie" << endl;
    cout << "3. Sort by Hall" << endl;
    cout << "4. Sort by Ticket Type" << endl;
    cout << "5. Sort by Status" << endl;
    cout << "6. Back" << endl;
    cout << endl << "Select an option: ";
}

void displayMovieMenu() {
    // Movie Management is Staff/Admin only because customers should not edit screening data.
    cout << endl;
    printSeparator('-');
    printCenteredTitle("Movie Management");
    printSeparator('-');
    cout << "1. Insert Movie" << endl;
    cout << "2. View Movie List" << endl;
    cout << "3. Edit Movie" << endl;
    cout << "4. Delete Movie" << endl;
    cout << "5. Back" << endl;
    cout << endl << "Select an option: ";
}

void displayFileMenu() {
    // Autosave runs automatically, but this menu allows manual backup/load/export for demonstration.
    cout << endl;
    printSeparator('-');
    printCenteredTitle("File Management");
    printSeparator('-');
    cout << "1. Save Data to Text File" << endl;
    cout << "2. Load Data from Text File" << endl;
    cout << "3. Export Report" << endl;
    cout << "4. Back" << endl;
    cout << endl << "Select an option: ";
}

void displaySaveDataMenu() {
    // Each data type is saved separately so the text files stay easy to inspect.
    cout << endl;
    printSeparator('-');
    printCenteredTitle("Save Data");
    printSeparator('-');
    cout << "1. Save Ticket Records" << endl;
    cout << "2. Save Movie Catalog" << endl;
    cout << "3. Save Cancellation Log" << endl;
    cout << "4. Back" << endl;
    cout << endl << "Select an option: ";
}

void displayLoadDataMenu() {
    // Load options are separated to avoid mixing ticket, movie, and cancellation file formats.
    cout << endl;
    printSeparator('-');
    printCenteredTitle("Load Data");
    printSeparator('-');
    cout << "1. Load Ticket Records" << endl;
    cout << "2. Load Movie Catalog" << endl;
    cout << "3. Load Cancellation Log" << endl;
    cout << "4. Back" << endl;
    cout << endl << "Select an option: ";
}

// Reusable input validation for all menus.
int getMenuSelection(int minimumOption, int maximumOption) {
    while (true) {
        int choice;
        // If the user types letters instead of a number, clear the input error state.
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number between "
                 << minimumOption << " and " << maximumOption << "." << endl;
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        // Range checking prevents invalid menu options from reaching the switch statement.
        if (choice < minimumOption || choice > maximumOption) {
            cout << "Invalid option. Please select a number between "
                 << minimumOption << " and " << maximumOption << "." << endl;
            continue;
        }
        return choice;
    }
}

// Simple fixed-password check for Staff/Admin access.
bool verifyAdminPassword() {
    string password;
    cout << "Enter admin password: ";
    getline(cin, password);

    // The password is case-sensitive. Only "admin123" is accepted.
    if (password == AdminPassword) {
        cout << "Access granted. Welcome, Staff/Admin." << endl;
        return true;
    }

    cout << "Access denied. Returning to role menu." << endl;
    return false;
}

// Serve submenu lets Staff/Admin choose one FIFO serve or batch serve.
void handleServeMenu(Registry& registry) {
    bool serving = true;
    while (serving) {
        displayServeMenu();
        int option = getMenuSelection(1, 3);

        // The switch redirects each menu choice to the correct Registry function.
        switch (option) {
            case 1:
                registry.serveNextCustomer();
                break;
            case 2:
                registry.serveAllCustomers();
                break;
            case 3:
                serving = false;
                break;
        }
    }
}

// Assignment Part C and Part D:
// Search submenu lets user choose Linear Search, Binary Search, or comparison.
void handleSearchMenu(Registry& registry) {
    bool searching = true;
    while (searching) {
        displaySearchMenu();
        int option = getMenuSelection(1, 6);

        // Linear and binary search are separated so they can be demonstrated individually.
        switch (option) {
            case 1:
                registry.findTicketLinear();
                break;
            case 2:
                registry.findTicketBinary();
                break;
            case 3:
                registry.compareLinearVsBinary();
                break;
            case 4:
                searching = false;
                break;
        }
    }
}

// Sorting submenu for different ticket record views.
void handleSortingMenu(Registry& registry) {
    bool sorting = true;
    while (sorting) {
        displaySortingMenu();
        int option = getMenuSelection(1, 6);

        // All sorting options use insertion sort but compare different ticket fields.
        switch (option) {
            case 1:
                registry.viewTicketsSortedByID();
                break;
            case 2:
                registry.viewTicketsSortedByMovie();
                break;
            case 3:
                registry.viewTicketsSortedByHall();
                break;
            case 4:
                registry.viewTicketsSortedByType();
                break;
            case 5:
                registry.viewTicketsSortedByStatus();
                break;
            case 6:
                sorting = false;
                break;
        }
    }
}

// Assignment display requirements:
// This submenu groups queue display, stack display, booking records, summary, and sorted records.
void handleRecordsMenu(Registry& registry) {
    bool viewing = true;
    while (viewing) {
        displayRecordsMenu();
        int option = getMenuSelection(1, 11);

        // This menu calls display functions only; it does not modify ticket records.
        switch (option) {
            case 1:
                registry.viewWaitingLine();
                break;
            case 2:
                registry.viewFrontOfLine();
                break;
            case 3:
                registry.viewRearOfLine();
                break;
            case 4:
                registry.viewQueueStatus();
                break;
            case 5:
                registry.viewCancellationLog();
                break;
            case 6:
                registry.peekLatestCancellation();
                break;
            case 7:
                registry.viewFullRegistry();
                break;
            case 8:
                registry.viewSystemSummary();
                break;
            case 9:
                handleSortingMenu(registry);
                break;
            case 10:
                registry.viewTodaysMovies();
                break;
            case 11:
                viewing = false;
                break;
        }
    }
}

// Staff movie management submenu for catalog maintenance.
void handleMovieMenu(Registry& registry) {
    bool managingMovies = true;
    while (managingMovies) {
        displayMovieMenu();
        int option = getMenuSelection(1, 5);

        // Staff can perform CRUD operations on movie screenings.
        switch (option) {
            case 1:
                registry.addMovie();
                break;
            case 2:
                registry.viewMovieList();
                break;
            case 3:
                registry.editMovie();
                break;
            case 4:
                registry.deleteMovie();
                break;
            case 5:
                managingMovies = false;
                break;
        }
    }
}

void handleSaveDataMenu(Registry& registry) {
    bool savingData = true;
    while (savingData) {
        displaySaveDataMenu();
        int option = getMenuSelection(1, 4);

        // Manual save is useful when the lecturer wants to inspect the text files.
        switch (option) {
            case 1:
                registry.saveTicketsToTextFile();
                break;
            case 2:
                registry.saveMovieCatalogToTextFile();
                break;
            case 3:
                registry.saveCancellationLogToTextFile();
                break;
            case 4:
                savingData = false;
                break;
        }
    }
}

void handleLoadDataMenu(Registry& registry) {
    bool loadingData = true;
    while (loadingData) {
        displayLoadDataMenu();
        int option = getMenuSelection(1, 4);

        // Manual load replaces/restores data from a selected text file.
        switch (option) {
            case 1:
                registry.loadTicketsFromTextFile();
                break;
            case 2:
                registry.loadMovieCatalogFromTextFile();
                break;
            case 3:
                registry.loadCancellationLogFromTextFile();
                break;
            case 4:
                loadingData = false;
                break;
        }
    }
}

// Staff file management submenu for manual save/load/export operations.
void handleFileMenu(Registry& registry) {
    bool managingFiles = true;
    while (managingFiles) {
        displayFileMenu();
        int option = getMenuSelection(1, 4);

        // File Management is grouped into save/load submenus to keep the main menu tidy.
        switch (option) {
            case 1:
                handleSaveDataMenu(registry);
                break;
            case 2:
                handleLoadDataMenu(registry);
                break;
            case 3:
                registry.exportReport();
                break;
            case 4:
                managingFiles = false;
                break;
        }
    }
}

// Customer role workflow.
void handleCustomerMenu(Registry& registry) {
    bool customerActive = true;
    while (customerActive) {
        displayCustomerMenu();
        int option = getMenuSelection(1, 5);

        // Customer role only exposes booking, searching, cancellation, and system info.
        switch (option) {
            case 1:
                registry.browseMoviesAndBook();
                break;
            case 2:
                registry.findTicketLinear();
                break;
            case 3:
                registry.cancelTicket();
                break;
            case 4:
                displayAboutSystem();
                break;
            case 5:
                customerActive = false;
                break;
        }
    }
}

// Staff/Admin role workflow.
void handleStaffMenu(Registry& registry) {
    bool staffActive = true;
    while (staffActive) {
        displayStaffMenu();
        int option = getMenuSelection(1, 8);

        // Staff/Admin role exposes management features after password verification.
        switch (option) {
            case 1:
                handleMovieMenu(registry);
                break;
            case 2:
                handleServeMenu(registry);
                break;
            case 3:
                handleRecordsMenu(registry);
                break;
            case 4:
                handleSearchMenu(registry);
                break;
            case 5:
                registry.editTicketInfo();
                break;
            case 6:
                registry.undoLastCancellation();
                break;
            case 7:
                handleFileMenu(registry);
                break;
            case 8:
                staffActive = false;
                break;
        }
    }
}

int main() {
    Registry registry;
    bool running = true;

    // Registry constructor auto-loads saved text files before the first menu appears.
    displayWelcomeMessage();

    while (running) {
        displayRoleMenu();
        int option = getMenuSelection(1, 3);

        // Role-based access control starts here.
        switch (option) {
            case 1:
                handleCustomerMenu(registry);
                break;
            case 2:
                if (verifyAdminPassword()) {
                    handleStaffMenu(registry);
                }
                break;
            case 3:
                cout << "Thank you for using Cinema Ticket Booking System. Goodbye!" << endl;
                running = false;
                break;
        }
    }

    return 0;
}
