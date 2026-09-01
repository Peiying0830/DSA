#include <iostream>
#include <string>
using namespace std;

const int MAX = 5;

class Queue {
private:
    string customerID[MAX];
    string customerName[MAX];
    int front;
    int rear;

public:
    // Constructor
    Queue() {
        front = -1;
        rear = -1;
    }

    // Check if queue is empty
    bool isEmpty() {
        return (front == -1);
    }

    // Check if queue is full
    bool isFull() {
        return (rear == MAX - 1);
    }

    // Add customer to queue
    void enqueue() {

        if (isFull()) {
            cout << "\nQueue is FULL! Cannot add more customers.\n";
            return;
        }

        string id, name;

        cout << "\nEnter Customer ID: ";
        cin >> id;

        cin.ignore();

        cout << "Enter Customer Name: ";
        getline(cin, name);

        // First customer
        if (front == -1) {
            front = 0;
        }

        rear++;

        customerID[rear] = id;
        customerName[rear] = name;

        cout << "\nCustomer added successfully!\n";
    }

    // Remove customer from queue
    void dequeue() {

        if (isEmpty()) {
            cout << "\nQueue is EMPTY! No customer to serve.\n";
            return;
        }

        cout << "\nCustomer " << customerID[front]
             << " - " << customerName[front]
             << " has been served.\n";

        // If this is the last customer
        if (front == rear) {
            front = -1;
            rear = -1;
        }
        else {
            front++;
        }
    }

    // View first customer
    void peek() {

        if (isEmpty()) {
            cout << "\nQueue is EMPTY!\n";
            return;
        }

        cout << "\nNext Customer:\n";
        cout << "ID   : " << customerID[front] << endl;
        cout << "Name : " << customerName[front] << endl;
    }

    // Display all customers
    void display() {

        if (isEmpty()) {
            cout << "\nQueue is EMPTY!\n";
            return;
        }

        cout << "\n========== CUSTOMER QUEUE ==========\n";
        cout << "------------------------------------\n";
        cout << "Position\tID\tName\n";
        cout << "------------------------------------\n";

        for (int i = front; i <= rear; i++) {
            cout << i - front + 1 << "\t\t"
                 << customerID[i] << "\t"
                 << customerName[i] << endl;
        }

        cout << "------------------------------------\n";
    }

    // Check queue status
    void status() {

        if (isEmpty()) {
            cout << "\nQueue Status: EMPTY\n";
        }
        else if (isFull()) {
            cout << "\nQueue Status: FULL\n";
        }
        else {
            cout << "\nQueue Status: AVAILABLE\n";
            cout << "Number of customers: "
                 << rear - front + 1 << endl;
            cout << "Available spaces: "
                 << MAX - (rear - front + 1) << endl;
        }
    }

    // Search customer
    void searchCustomer() {

        if (isEmpty()) {
            cout << "\nQueue is EMPTY!\n";
            return;
        }

        string id;
        bool found = false;

        cout << "\nEnter Customer ID to search: ";
        cin >> id;

        for (int i = front; i <= rear; i++) {

            if (customerID[i] == id) {

                cout << "\nCustomer found!\n";
                cout << "ID   : " << customerID[i] << endl;
                cout << "Name : " << customerName[i] << endl;
                cout << "Position in Queue: "
                     << i - front + 1 << endl;

                found = true;
                break;
            }
        }

        if (!found) {
            cout << "\nCustomer not found in the queue.\n";
        }
    }
};


// Main function
int main() {

    Queue q;

    int choice;

    do {

        cout << "\n\n====================================\n";
        cout << "       CUSTOMER SERVICE QUEUE\n";
        cout << "====================================\n";
        cout << "1. Add Customer (Enqueue)\n";
        cout << "2. Serve Customer (Dequeue)\n";
        cout << "3. View Next Customer (Peek)\n";
        cout << "4. Display Queue\n";
        cout << "5. Check Queue Status\n";
        cout << "6. Search Customer\n";
        cout << "7. Exit\n";
        cout << "====================================\n";

        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {

            case 1:
                q.enqueue();
                break;

            case 2:
                q.dequeue();
                break;

            case 3:
                q.peek();
                break;

            case 4:
                q.display();
                break;

            case 5:
                q.status();
                break;

            case 6:
                q.searchCustomer();
                break;

            case 7:
                cout << "\nThank you! Program terminated.\n";
                break;

            default:
                cout << "\nInvalid choice! Please try again.\n";
        }

    } while (choice != 7);

    return 0;
}
