#include <iostream>
#include <string>
using namespace std;  // No need to write "std::"

int main() {
    // Declare variables to store the student's data
    string name;                      // Stores the student's name
    float math, science, english;     // Stores marks for each subject
    float total, average;             // Stores the total and average marks

    // Ask the user to enter the student's name
    cout << "Enter student's name: ";
    getline(cin, name);   // Reads the whole line (allows names with spaces)

    // Ask the user to enter marks for each subject
    cout << "Enter marks for Math: ";
    cin >> math;

    cout << "Enter marks for Science: ";
    cin >> science;

    cout << "Enter marks for English: ";
    cin >> english;

    // Calculate the total marks by adding all three subjects
    total = math + science + english;

    // Calculate the average by dividing total by number of subjects (3)
    average = total / 3;

    // Display all the results to the user
    cout << "\n--- Student Report ---" << endl;
    cout << "Name: " << name << endl;
    cout << "Math: " << math << endl;
    cout << "Science: " << science << endl;
    cout << "English: " << english << endl;
    cout << "Total Marks: " << total << endl;
    cout << "Average Mark: " << average << endl;

    return 0;
}