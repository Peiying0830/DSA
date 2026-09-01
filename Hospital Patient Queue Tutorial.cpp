// Program Name: Hospital Patient Queue Tutorial

#include <iostream>
#include <string>
using namespace std;

const int MAX = 5;

// Circular queue implementation for a hospital patient waiting list.
class Queue {
private:
    // Parallel arrays store each patient's details at the same index.
    string patientID[MAX];
    string patientName[MAX];
    int patientAge[MAX];

    // front points to the next patient to be attended.
    // rear points to the most recently registered patient.
    // count tracks how many patients are currently in the queue.
    int front;
    int rear;
    int count;

public:
    // Constructor
    Queue() {
        front = -1;
        rear = -1;
        count = 0;
    }

    // Check if queue is empty
    bool isEmpty() {
        // In this circular queue, count is the easiest way to check empty/full.
        return (count == 0);
    }

    // Check if queue is full
    bool isFull() {
        return (count == MAX);
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

        // When adding the first patient, front must point to index 0.
        if (isEmpty()) {
            front = 0;
        }

        // Move rear forward. % MAX makes it wrap back to 0 after the last index.
        rear = (rear + 1) % MAX;

        patientID[rear] = id;
        patientName[rear] = name;
        patientAge[rear] = age;
        count++;

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

        // If this is the last patient, reset the queue to empty state.
        if (count == 1) {
            front = -1;
            rear = -1;
        }
        else {
            // Move front forward and wrap around if needed.
            front = (front + 1) % MAX;
        }

        count--;
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

        for (int i = 0; i < count; i++) {
            // Convert queue position to the actual array index.
            int index = (front + i) % MAX;

            cout << i + 1 << "\t\t"
                 << patientID[index] << "\t"
                 << patientName[index] << "\t"
                 << patientAge[index] << endl;
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
            cout << "Number of patients: " << count << endl;
            cout << "Available spaces: " << MAX - count << endl;
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

        for (int i = 0; i < count; i++) {
            // Search follows the queue order, starting from front.
            int index = (front + i) % MAX;

            if (patientID[index] == id) {

                cout << "\nPatient found!\n";
                cout << "ID   : " << patientID[index] << endl;
                cout << "Name : " << patientName[index] << endl;
                cout << "Age  : " << patientAge[index] << endl;
                cout << "Position in Queue: " << i + 1 << endl;

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
        cin >> choice;

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
