#include <iostream>
#include <string>
using namespace std;

// ------------------------------------------------------------
// Book class
// Holds information about a single book.
// Data members are private (encapsulation) so they can only be
// accessed/modified through the class's own public methods.
// ------------------------------------------------------------
class Book {
private:
    string bookID;   // Unique ID of the book, e.g. "B101"
    string title;    // Title of the book
    string author;   // Author's name
    int copies;      // Number of copies available in the library

public:
    // ---------------- Part A: Default Constructor ----------------
    // Called when a Book object is created without arguments.
    // Sets all data members to safe default values.
    Book() {
        bookID = "";
        title = "";
        author = "";
        copies = 0;
    }

    // ---------------- Part A: Parameterized Constructor ----------------
    // Called when a Book object is created with specific values.
    // Used in Part B to fill the array of 5 books.
    Book(string id, string t, string a, int c) {
        bookID = id;
        title = t;
        author = a;
        copies = c;
    }

    // ---------------- Getter functions ----------------
    // These are GETTERS - since data members are private, these allow
    // read-only access to each field from outside the class
    // (e.g. when using pointers to display book information).
    string getID() const { return bookID; }
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    int getCopies() const { return copies; }

    // ---------------- Part E: Update Copies ----------------
    // This is a SETTER - it modifies the private "copies" data member
    // from outside the class. Adds newly arrived copies to the
    // existing copy count (instead of overwriting it).
    void addCopies(int extra) { copies += extra; }

    // ---------------- Display function ----------------
    // Prints all details of a single book in the required format.
    // Reused in Part C (listing all books), Part D (search result),
    // and Part E (updated result).
    void display() const {
        cout << "Book ID: " << bookID << endl;
        cout << "Title: " << title << endl;
        cout << "Author: " << author << endl;
        cout << "Copies: " << copies << endl;
    }
};

int main() {
    // ---------------- Part B: Array of Objects ----------------
    // Declare an array of 5 Book objects.
    // Each element is initialized directly using the parameterized
    // constructor with the sample data given in the assignment.
    Book books[5] = {
        Book("B101", "Data Structures", "Mark Allen", 8),
        Book("B102", "C++ Programming", "John Smith", 12),
        Book("B103", "Database Systems", "Peter Brown", 5),
        Book("B104", "Operating Systems", "Linda Tan", 9),
        Book("B105", "Computer Networks", "David Lee", 7)
    };

    // ---------------- Part C: Pointer Operations ----------------
    // ptr points to the first element of the array (books[0]).
    // Array name "books" already decays to the address of its
    // first element, so this is equivalent to &books[0].
    Book *ptr = books;

    cout << "Library Books" << endl;
    cout << endl;

    // Access every book using pointer arithmetic only (ptr + i)
    // together with the -> operator. No array indexing (books[i])
    // is used here, as required by Part C.
    for (int i = 0; i < 5; i++) {
        cout << "Book ID: " << (ptr + i)->getID() << endl;
        cout << "Title: " << (ptr + i)->getTitle() << endl;
        cout << "Author: " << (ptr + i)->getAuthor() << endl;
        cout << "Copies: " << (ptr + i)->getCopies() << endl;
        cout << endl;
    }

    // ---------------- Part D: Search Book ----------------
    // Ask the user for a Book ID to search for.
    string searchID;
    cout << "\nEnter Book ID to search: ";
    cin >> searchID;

    // found will store the address of the matching book (if any).
    // Starts as nullptr so we can tell whether a match was found.
    Book *found = nullptr;

    // Walk through the array using pointer arithmetic and compare
    // each book's ID with the one the user entered.
    for (int i = 0; i < 5; i++) {
        if ((ptr + i)->getID() == searchID) {
            found = (ptr + i);  // Save pointer to the matching book
            break;              // Stop searching once found
        }
    }

    // ---------------- Part D & E: Display Result / Update ----------------
    if (found != nullptr) {
        // Book was found - display its current details
        cout << "\nBook Found" << endl;
        found->display();

        // ---------------- Part E: Update Copies ----------------
        // Ask for the number of newly arrived copies and add them
        // to the book's existing copy count using the pointer.
        int extra;
        cout << "\nEnter additional copies: ";
        cin >> extra;

        found->addCopies(extra);  // SETTER call - updates copies through the pointer

        // Show the book's details again after the update
        cout << "\nUpdated Book Information" << endl;
        found->display();
    } else {
        // No book matched the entered ID
        cout << "Book not found." << endl;
    }

    return 0;
}