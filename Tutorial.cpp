#include <iostream>
#include <string>
using namespace std;

// ===== Stack (array-based) for storing browser history =====
class BrowserHistory {
private:
    string* stackArr;   // dynamic array acting as the stack
    int top;             // index of the top element (-1 = empty)
    int capacity;         // maximum history size (user-defined)

public:
    // Constructor: allocate the array based on user-chosen max size
    BrowserHistory(int maxSize) {
        capacity = maxSize;
        stackArr = new string[capacity];
        top = -1; // stack starts empty
    }

    // Destructor: free the dynamically allocated array
    ~BrowserHistory() {
        delete[] stackArr;
    }

    // Check if stack is empty
    bool isEmpty() {
        return top == -1;
    }

    // Check if stack is full
    bool isFull() {
        return top == capacity - 1;
    }

    // push(): visit a new page -> add to top of stack
    void push(const string& url) {
        if (isFull()) {
            cout << "History is full. Cannot visit a new page.\n";
            return;
        }
        stackArr[++top] = url; // increment top, then insert
        cout << "Page added successfully.\n";
    }

    // pop(): go back -> remove the most recent page
    void pop() {
        if (isEmpty()) {
            cout << "History is empty. Cannot go back.\n";
            return;
        }
        string removed = stackArr[top];
        top--; // decrement top (logical removal)
        cout << "Going back from: " << removed << "\n";

        if (isEmpty())
            cout << "No previous page available.\n";
        else
            cout << "Current Page: " << stackArr[top] << "\n";
    }

    // peek()/top: show current page without removing it
    void peek() {
        if (isEmpty()) {
            cout << "No previous page available.\n";
            return;
        }
        cout << "Current Page: " << stackArr[top] << "\n";
    }

    // display(): show full history, most recent first
    void display() {
        if (isEmpty()) {
            cout << "History is empty.\n";
            return;
        }
        cout << "===== BROWSING HISTORY =====\n";
        int count = 1;
        for (int i = top; i >= 0; i--) {   // walk downward from top
            cout << count << ". " << stackArr[i] << "\n";
            count++;
        }
    }

    // Check current number of pages stored
    void size() {
        cout << "Total pages in history: " << (top + 1) << "\n";
    }

    // Challenge extension: clear all history
    void clearHistory() {
        top = -1; // just reset the pointer; old values become "inactive"
        cout << "History cleared.\n";
    }
};

int main() {
    int maxSize;

    // Additional challenge: let user set max history size at start
    cout << "Enter maximum history size: ";
    cin >> maxSize;
    cin.ignore(); // clear leftover newline from cin >> before using getline

    BrowserHistory history(maxSize);

    int choice;
    do {
        // Menu display
        cout << "\n===== BROWSER HISTORY =====\n";
        cout << "1. Visit New Page\n";
        cout << "2. Go Back\n";
        cout << "3. Display Current Page\n";
        cout << "4. Display History\n";
        cout << "5. Check History Size\n";
        cout << "6. Clear History\n";   // challenge extension option
        cout << "7. Exit\n";
        cout << "Enter your choice: ";

        // Input validation in case user types a non-integer
        if (!(cin >> choice)) {
            cin.clear();               // reset error flag
            cin.ignore(1000, '\n');    // discard bad input
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }
        cin.ignore(); // remove leftover newline before getline

        switch (choice) {
            case 1: {
                string url;
                cout << "Enter webpage: ";
                getline(cin, url);
                history.push(url);
                break;
            }
            case 2:
                history.pop();
                break;
            case 3:
                history.peek();
                break;
            case 4:
                history.display();
                break;
            case 5:
                history.size();
                break;
            case 6:
                history.clearHistory();
                break;
            case 7:
                cout << "Exiting program. Goodbye!\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }

    } while (choice != 7);

    return 0;
}