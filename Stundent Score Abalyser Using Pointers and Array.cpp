#include <iostream>
using namespace std;

int main() {
    const int SIZE = 5;      // Number of students
    int scores[SIZE];        // Array to store 5 student scores
    int *ptr = scores;       // Pointer set to point at the start of the array

    // Input scores using pointer arithmetic
    for (int i = 0; i < SIZE; i++) {
        cout << "Enter score for Student " << i + 1 << ": ";
        cin >> *(ptr + i);   // same as scores[i], but written using pointer arithmetic
    }

    // Display all scores using the pointer
    cout << "\nStudent Scores: ";
    for (int i = 0; i < SIZE; i++) {
        cout << *(ptr + i) << " ";   // dereference pointer at offset i to read the value
    }
    cout << endl;

    // Calculate total using the pointer
    int total = 0;
    for (int i = 0; i < SIZE; i++) {
        total += *(ptr + i);         // add each score to total
    }
    cout << "Total Score: " << total << endl;

    // Calculate average
    double average = static_cast<double>(total) / SIZE;  // cast to double so division isn't rounded to an int
    cout << "Average Score: " << average << endl;

    // Find highest score
    int highest = *ptr;              // start by assuming the first score is the highest
    for (int i = 1; i < SIZE; i++) {
        if (*(ptr + i) > highest) {  // compare each score to current highest
            highest = *(ptr + i);    // update highest if a bigger score is found
        }
    }
    cout << "Highest Score: " << highest << endl;

    // Find lowest score
    int lowest = *ptr;               // start by assuming the first score is the lowest
    for (int i = 1; i < SIZE; i++) {
        if (*(ptr + i) < lowest) {   // compare each score to current lowest
            lowest = *(ptr + i);     // update lowest if a smaller score is found
        }
    }
    cout << "Lowest Score: " << lowest << endl;

    return 0;
}