#include <iostream>
#include <string>
#include <limits>          // Needed for numeric_limits (used to clear input buffer)
using namespace std;

class Student {
private:
    string name;         // Student's name (hidden from outside)
    double marks[3];      // Marks for 3 subjects (hidden from outside)
    double total;         // Total marks
    double average;       // Average marks

    // Private helper method to get a valid mark between 0 and 100
    double getValidMark(int subjectNum) {
        double mark;
        while (true) {
            cout << "Enter marks for subject " << subjectNum << ": ";
            cin >> mark;

            // Check if input was not a number
            if (cin.fail()) {
                cin.clear();                                              // Clear the error flag on cin
                cin.ignore(numeric_limits<streamsize>::max(), '\n');      // Discard the bad input left in buffer
                cout << "Invalid input for Subject " << subjectNum
                     << ". Please enter a number.\n";
                continue;   // Ask again
            }

            // Check if mark is out of the valid range
            if (mark < 0 || mark > 100) {
                cout << "Invalid marks for Subject " << subjectNum
                     << "! Marks must be between 0 and 100.\n";
            } else {
                break;   // Valid mark entered, exit loop
            }
        }
        return mark;
    }

public:
    // Method to read the name and 3 marks from the user
    void inputData() {
        cout << "Enter student's name: ";
        getline(cin, name);

        for (int j = 0; j < 3; j++) { // 1 Student has 3 subjects
            marks[j] = getValidMark(j + 1);   // Get a valid mark (0-100)
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Clear leftover newline for next getline
    }

    // Method to calculate the total marks
    void calculateTotal() {
        total = 0;
        for (int j = 0; j < 3; j++) {
            total += marks[j]; // Total = 3 subjects marks added together
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