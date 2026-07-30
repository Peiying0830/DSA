#include <iostream>
#include <string>
#include <limits>          // Needed for numeric_limits (used to clear input buffer)
using namespace std;

class Student {
private:
    string name;
    double marks[3];
    double total;
    double average;

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
    void inputData() {
        cout << "Enter student's name: ";
        getline(cin, name);

        for (int j = 0; j < 3; j++) {
            marks[j] = getValidMark(j + 1);   // Get a valid mark (0-100)
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Clear leftover newline for next getline
    }

    void calculateTotal() {
        total = 0;
        for (int j = 0; j < 3; j++) {
            total += marks[j];
        }
    }

    void calculateAverage() {
        average = total / 3;
    }

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
    const int numStudents = 3;
    Student classroom[3];   // Array of 3 Student objects

    // Loop through the array to input data for each student
    for (int i = 0; i < numStudents; i++) {
        cout << "\n--- Entering data for student " << (i + 1) << " ---" << endl;
        classroom[i].inputData();
        classroom[i].calculateTotal();
        classroom[i].calculateAverage();
    }

    // Loop through the array to display each student's report
    cout << "\n=== Classroom Roster ===" << endl;
    for (int i = 0; i < numStudents; i++) {
        classroom[i].displayReport();
    }

    return 0;
}