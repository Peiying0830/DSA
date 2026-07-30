#include <iostream>
#include <string>
#include <limits>          // Needed for numeric_limits (used to clear input buffer)
using namespace std;

// Function to get a valid mark between 0 and 100 for a given student and subject
double getValidMark(int studentNum, int subjectNum) {
    double mark;
    while (true) {
        cout << "Enter marks for subject " << subjectNum << ": ";
        cin >> mark;

        // Check if input was not a number 
        if (cin.fail()) {
            cin.clear();                                              // Clear the error flag on cin
            cin.ignore(numeric_limits<streamsize>::max(), '\n');      // Discard the bad input left in buffer
            cout << "Invalid input for Student " << studentNum
                 << ", Subject " << subjectNum << ". Please enter a number.\n";
            continue;   // Ask again
        }

        // Check if mark is out of the valid range
        if (mark < 0 || mark > 100) {
            cout << "Invalid marks for Student " << studentNum
                 << ", Subject " << subjectNum << "! Marks must be between 0 and 100.\n";
        } else {
            break;   // Valid mark entered, exit loop
        }
    }
    return mark;
}

int main() {
    const int numStudents = 3;   // Total number of students
    const int numSubjects = 3;   // Total number of subjects

    string names[3];              // 1D array to store 3 student names
    double marks[3][3];           // 2D array: rows = students, columns = subjects
    double total[3];              // Store total marks for each student
    double average[3];            // Store average marks for each student

    // Input names and marks for each student
    for (int i = 0; i < numStudents; i++) {
        cout << "\nEnter name for student " << (i + 1) << ": ";
        getline(cin, names[i]);   // Store name into row i of names array

        total[i] = 0;             // Reset total before adding marks

        // Inner loop: enter validated marks for each subject of this student
        for (int j = 0; j < numSubjects; j++) {
            marks[i][j] = getValidMark(i + 1, j + 1);   // Get a valid mark (0-100)
            total[i] += marks[i][j];                    // Add this subject's mark to the running total
        }

        // Clear leftover newline left by the last cin >> mark,
        // so the next getline() for the following student's name works correctly
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        average[i] = total[i] / numSubjects;  // Calculate average for this student
    }

    // Display report for each student
    cout << "\n--- Class Report ---" << endl;
    for (int i = 0; i < numStudents; i++) {
        cout << "\nStudent: " << names[i] << endl;
        for (int j = 0; j < numSubjects; j++) {
            cout << "Subject " << (j + 1) << " Marks: " << marks[i][j] << endl;
        }
        cout << "Total Marks: " << total[i] << endl;
        cout << "Average Mark: " << average[i] << endl;
    }

    return 0;
}