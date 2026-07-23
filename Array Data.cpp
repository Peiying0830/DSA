#include <iostream>
#include <string>
using namespace std;

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

        // Inner loop: enter marks for each subject of this student
        for (int j = 0; j < numSubjects; j++) {
            cout << "Enter marks for subject " << (j + 1) << ": ";
            cin >> marks[i][j];   // Store mark into row i, column j
            total[i] += marks[i][j];  // Add this subject's mark to the running total
        }
        cin.ignore();  // Clear leftover newline so getline works properly next loop

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