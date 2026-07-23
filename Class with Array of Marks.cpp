#include <iostream>
#include <string>
using namespace std;

class Student {
private:
    string name;         // Student's name (hidden from outside)
    double marks[3];      // Marks for 3 subjects (hidden from outside)
    double total;         // Total marks
    double average;       // Average marks

public:
    // Method to read the name and 3 marks from the user
    void inputData() {
        cout << "Enter student's name: ";
        getline(cin, name);

        for (int j = 0; j < 3; j++) {
            cout << "Enter marks for subject " << (j + 1) << ": ";
            cin >> marks[j];
        }
        cin.ignore();  // Clear leftover newline for next getline
    }

    // Method to calculate the total marks
    void calculateTotal() {
        total = 0;
        for (int j = 0; j < 3; j++) {
            total += marks[j];
        }
    }

    // Method to calculate the average marks
    void calculateAverage() {
        average = total / 3;
    }

    // Method to print the final scorecard
    void displayReport() {
        cout << "\n--- Student Report ---" << endl;
        cout << "Name: " << name << endl;
        for (int j = 0; j < 3; j++) {
            cout << "Subject " << (j + 1) << " Marks: " << marks[j] << endl;
        }
        cout << "Total Marks: " << total << endl;
        cout << "Average Mark: " << average << endl;
    }
};

int main() {
    Student student1;   // Create one Student object

    student1.inputData();          // Call method to input data
    student1.calculateTotal();     // Call method to calculate total
    student1.calculateAverage();   // Call method to calculate average
    student1.displayReport();      // Call method to display report

    return 0;
}