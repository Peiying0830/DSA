#include <iostream>
using namespace std;

// Maximum number of elements that can be stored in the array.
const int MAX_SIZE = 100;

// Class to manage an array and sort it using Bubble Sort.
class BubbleSortArray {
private:
    // Static array used to store integer values.
    int arr[MAX_SIZE];

    // Current number of elements inside the array.
    int size;

public:
    // Constructor
    BubbleSortArray() {
        // The array starts empty, so size is set to 0.
        size = 0;
    }

    // Insert a new value at the end of the array.
    void insert(int value) {
        if (size >= MAX_SIZE) {
            cout << "Array is full!" << endl;
            return;
        }

        // Store the value at the next available position.
        arr[size] = value;

        // Increase size after inserting a new element.
        size++;
    }

    // Display all elements in the array.
    void display() {
        if (size == 0) {
            cout << "Array is empty." << endl;
            return;
        }

        cout << "Array: ";

        // Print elements from index 0 until size - 1.
        for (int i = 0; i < size; i++) {
            cout << arr[i] << " ";
        }

        cout << endl;
    }

    // Sort the array in ascending order using Bubble Sort.
    void bubbleSort() {
        // comparisons counts how many times two elements are compared.
        // swaps counts how many times two elements are exchanged.
        int comparisons = 0, swaps = 0;

        // Outer loop controls the number of passes.
        for (int i = 0; i < size - 1; i++) {
            // This checks whether any swap happens in the current pass.
            bool swapped = false;

            // Inner loop compares neighboring elements.
            // After each pass, the largest remaining value moves to the end.
            for (int j = 0; j < size - 1 - i; j++) {
                comparisons++;

                // Swap if the left value is greater than the right value.
                if (arr[j] > arr[j + 1]) {
                    int temp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = temp;

                    swaps++;
                    swapped = true;
                }
            }

            // If no swaps happened, the array is already sorted.
            if (!swapped) break;
        }

        cout << "\nBubble Sort complete. Comparisons: " << comparisons
             << ", Swaps: " << swaps << endl;
    }
};

// Main function
int main() {
    BubbleSortArray numbers;
    int n;

    // Ask the user how many elements they want to insert.
    cout << "Enter number of elements: ";
    cin >> n;

    cout << "Enter " << n << " values:\n";

    // Read values from the user and insert them into the array.
    for (int i = 0; i < n; i++) {
        int value;
        cin >> value;
        numbers.insert(value);
    }

    // Display the array before sorting.
    cout << "\nOriginal ";
    numbers.display();

    // Sort the array in ascending order.
    numbers.bubbleSort();

    // Display the array after sorting.
    cout << "Sorted ";
    numbers.display();

    return 0;
}
