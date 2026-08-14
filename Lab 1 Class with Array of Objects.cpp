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

            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input for Subject " << subjectNum
                     << ". Please enter a number.\n";
                continue;
            }

            if (mark < 0 || mark > 100) {
                cout << "Invalid marks for Subject " << subjectNum
                     << "! Marks must be between 0 and 100.\n";
            } else {
                break;
            }
        }
        return mark;
    }

public:
    void inputData() {
        cout << "Enter student's name: ";
        getline(cin, name);

        for (int j = 0; j < 3; j++) {
            marks[j] = getValidMark(j + 1);
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
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
    Student classroom[numStudents];   // array of Student objects (Part 4 requirement)

    // Loop through the array to call inputData() for each student
    for (int i = 0; i < numStudents; i++) {
        cout << "\n--- Entering data for Student " << (i + 1) << " ---\n";
        classroom[i].inputData();
        classroom[i].calculateTotal();
        classroom[i].calculateAverage();
    }

    // Loop through the array to display the classroom roster
    cout << "\n========== CLASSROOM REPORT ==========\n";
    for (int i = 0; i < numStudents; i++) {
        classroom[i].displayReport();
        cout << "---------------------------------------" << endl;
    }

    return 0;
}