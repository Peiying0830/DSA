#include <iostream>
#include <string>
using namespace std;  // No need to write "std::"

// Function to get a valid mark between 0 and 100
float getValidMark(const string& subject) {
    float mark;
    while (true) {
        cout << "Enter marks for " << subject << ": ";
        cin >> mark;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        if (mark < 0 || mark > 100) {
            cout << "Invalid marks! Marks must be between 0 and 100.\n";
        } else {
            break;
        }
    }
    return mark;
}

int main() {
    // Declare variables to store the student's data
    string name;                      // Stores the student's name
    float math, science, english;     // Stores marks for each subject
    float total, average;             // Stores the total and average marks

    // Ask the user to enter the student's name
    cout << "Enter student's name: ";
    getline(cin, name);   // Reads the whole line (allows names with spaces)

    // Ask the user to enter marks for each subject (with validation)
    math    = getValidMark("Math");
    science = getValidMark("Science");
    english = getValidMark("English");

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