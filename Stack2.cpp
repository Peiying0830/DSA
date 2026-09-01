#include <iostream>
#include <string>
using namespace std;

const int MAX_SIZE = 100;

class Stack {
private:
    string actions[MAX_SIZE];
    int top;

public:
    Stack() {
        top = -1;
    }

    bool isEmpty() {
        return top == -1;
    }

    bool isFull() {
        return top == MAX_SIZE - 1;
    }

    void push(string action) {
        if (isFull()) {
            cout << "Error: Cannot perform more actions. Stack is full." << endl;
            return;
        }
        top++;
        actions[top] = action;
        cout << "Action added successfully." << endl;
    }

    void pop() {
        if (isEmpty()) {
            cout << "No actions to undo." << endl;
            return;
        }
        cout << "Undo: " << actions[top] << endl;
        top--;
    }

    void peek() {
        if (isEmpty()) {
            cout << "No actions performed yet." << endl;
            return;
        }
        cout << "Last Action: " << actions[top] << endl;
    }

    void display() {
        if (isEmpty()) {
            cout << "No actions performed yet." << endl;
            return;
        }
        cout << "===== ACTION HISTORY =====" << endl;
        int count = 1;
        for (int i = top; i >= 0; i--) {
            cout << count << ". " << actions[i] << endl;
            count++;
        }
    }

    void countActions() {
        cout << "Total Actions: " << (top + 1) << endl;
    }

    void clearAll() {
        top = -1;
        cout << "All actions have been cleared." << endl;
    }
};

int main() {
    Stack history;
    int choice;
    string action;

    do {
        cout << "\n===== TEXT EDITOR =====" << endl;
        cout << "1. Perform Action" << endl;
        cout << "2. Undo Last Action" << endl;
        cout << "3. View Last Action" << endl;
        cout << "4. Display All Actions" << endl;
        cout << "5. Count Actions" << endl;
        cout << "6. Clear All Actions" << endl;
        cout << "7. Exit" << endl;
        cout << "Enter choice: ";
        cin >> choice;
        cin.ignore(); // clear newline left in buffer before getline

        switch (choice) {
            case 1:
                cout << "Enter action: ";
                getline(cin, action);
                history.push(action);
                break;
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
                history.countActions();
                break;
            case 6:
                history.clearAll();
                break;
            case 7:
                cout << "Exiting Text Editor. Goodbye!" << endl;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    } while (choice != 7);

    return 0;
}