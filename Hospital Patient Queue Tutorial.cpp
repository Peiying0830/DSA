// Program Name: Hospital Patient Queue Tutorial

#include <iostream>
#include <limits>
#include <string>
using namespace std;

const int MAX = 5;

// Queue class stores and manages the hospital patient waiting list.
class Queue {
private:
    // These arrays store patient details using the same index.
    string patientID[MAX];
    string patientName[MAX];
    int patientAge[MAX];

    // front stores the position of the next patient to attend.
    // rear stores the position of the last patient registered.
    int front;
    int rear;

public:
    // Constructor
    Queue() {
        // -1 means the queue has no patients at the beginning.
        front = -1;
        rear = -1;
    }

    // Check if queue is empty
    bool isEmpty() {
        // If front is -1, there is no patient in the queue.
        return (front == -1);
    }

    // Check if queue is full
    bool isFull() {
        // Queue is full when rear reaches the last array index.
        return (rear == MAX - 1);
    }

    // Add patient to queue
    void registerPatient() {

        if (isFull()) {
            cout << "\nQueue is FULL! Cannot add more patients.\n";
            return;
        }

        string id, name;
        int age;

        cout << "\nEnter Patient ID: ";
        cin >> id;

        cin.ignore();

        cout << "Enter Patient Name: ";
        getline(cin, name);

        cout << "Enter Patient Age: ";
        cin >> age;

        // If this is the first patient, front starts at index 0.
        if (front == -1) {
            front = 0;
        }

        // Move rear to the next empty position before storing the patient.
        rear++;

        // Store patient details at the rear position.
        patientID[rear] = id;
        patientName[rear] = name;
        patientAge[rear] = age;

        cout << "\nPatient registered successfully!\n";
    }

    // Remove patient from queue
    void attendPatient() {

        if (isEmpty()) {
            cout << "\nQueue is EMPTY! No patient to attend.\n";
            return;
        }

        cout << "\nPatient " << patientID[front]
             << " - " << patientName[front]
             << " has been attended.\n";

        // If this is the last patient, reset the queue to empty.
        if (front == rear) {
            front = -1;
            rear = -1;
        }
        else {
            // Move front to the next patient in the queue.
            front++;
        }
    }

    // View first patient
    void viewNextPatient() {

        if (isEmpty()) {
            cout << "\nQueue is EMPTY!\n";
            return;
        }

        cout << "\nNext Patient:\n";
        cout << "ID   : " << patientID[front] << endl;
        cout << "Name : " << patientName[front] << endl;
        cout << "Age  : " << patientAge[front] << endl;
    }

    // Display all patients
    void displayAllPatients() {

        if (isEmpty()) {
            cout << "\nQueue is EMPTY!\n";
            return;
        }

        cout << "\n========== PATIENT QUEUE ==========\n";
        cout << "------------------------------------\n";
        cout << "Position\tID\tName\tAge\n";
        cout << "------------------------------------\n";

        // Display patients from front to rear in queue order.
        for (int i = front; i <= rear; i++) {
            cout << i - front + 1 << "\t\t"
                 << patientID[i] << "\t"
                 << patientName[i] << "\t"
                 << patientAge[i] << endl;
        }

        cout << "------------------------------------\n";
    }

    // Check queue status
    void checkQueueStatus() {

        if (isEmpty()) {
            cout << "\nQueue Status: EMPTY\n";
        }
        else if (isFull()) {
            cout << "\nQueue Status: FULL\n";
        }
        else {
            cout << "\nQueue Status: AVAILABLE\n";
            cout << "Number of patients: "
                 << rear - front + 1 << endl;
            cout << "Available spaces: "
                 << MAX - (rear - front + 1) << endl;
        }
    }

    // Search patient
    void searchPatient() {

        if (isEmpty()) {
            cout << "\nQueue is EMPTY!\n";
            return;
        }

        string id;
        bool found = false;

        cout << "\nEnter Patient ID to search: ";
        cin >> id;

        // Search each patient from front to rear.
        for (int i = front; i <= rear; i++) {

            if (patientID[i] == id) {

                cout << "\nPatient found!\n";
                cout << "ID   : " << patientID[i] << endl;
                cout << "Name : " << patientName[i] << endl;
                cout << "Age  : " << patientAge[i] << endl;
                cout << "Position in Queue: "
                     << i - front + 1 << endl;

                found = true;
                break;
            }
        }

        if (!found) {
            cout << "\nPatient not found in the queue.\n";
        }
    }
};


// Main function
int main() {

    Queue q;

    int choice;

    do {

        cout << "\n\n====================================\n";
        cout << "       HOSPITAL PATIENT QUEUE\n";
        cout << "====================================\n";
        cout << "1. Register Patient\n";
        cout << "2. Attend Patient\n";
        cout << "3. View Next Patient\n";
        cout << "4. Display All Patients\n";
        cout << "5. Check Queue Status\n";
        cout << "6. Search Patient\n";
        cout << "7. Exit\n";
        cout << "====================================\n";

        cout << "Enter your choice: ";

        // Prevent invalid input, such as letters, from breaking the menu loop.
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "\nInvalid choice! Please enter a number.\n";
            continue;
        }

        switch (choice) {

            case 1:
                q.registerPatient();
                break;

            case 2:
                q.attendPatient();
                break;

            case 3:
                q.viewNextPatient();
                break;

            case 4:
                q.displayAllPatients();
                break;

            case 5:
                q.checkQueueStatus();
                break;

            case 6:
                q.searchPatient();
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
