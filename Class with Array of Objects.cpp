#include <iostream>
#include <string>
using namespace std;

class Student {
private:
    string name;
    double marks[3];
    double total;
    double average;

public:
    void inputData() {
        cout << "Enter student's name: ";
        getline(cin, name);

        for (int j = 0; j < 3; j++) {
            cout << "Enter marks for subject " << (j + 1) << ": ";
            cin >> marks[j];
        }
        cin.ignore();
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