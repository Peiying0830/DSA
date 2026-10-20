#include <iostream>
using namespace std;

// Maximum number of elements that can be stored in the array.
const int MAX_SIZE = 100;

// Class to manage an unsorted array.
// In an unsorted array, elements are stored in the order they are inserted.
class UnsortedArray {
private:
    // Static array used to store integer values.
    int arr[MAX_SIZE];

    // Current number of elements inside the array.
    int size;

public:
    // Constructor
    UnsortedArray() {
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

    // Search for a value in the array.
    // Returns the index if found, otherwise returns -1.
    int search(int value) {
        // Linear search is used because the array is unsorted.
        for (int i = 0; i < size; i++) {
            if (arr[i] == value) {
                return i;
            }
        }

        // -1 means the value does not exist in the array.
        return -1;
    }

    // Remove the first occurrence of a value from the array.
    bool remove(int value) {
        // First, find the position of the value to remove.
        int index = search(value);

        // If the value is not found, deletion cannot be done.
        if (index == -1) return false;

        // Shift all elements after the deleted value one position to the left.
        for (int i = index; i < size - 1; i++) {
            arr[i] = arr[i + 1];
        }

        // Reduce the size because one element has been removed.
        size--;

        return true;
    }

    // Return the current number of elements in the array.
    int getSize() {
        return size;
    }

};

// Main function
int main() {
    UnsortedArray ua;
    int n;

    // Ask the user how many elements they want to insert.
    cout << "Enter number of elements: ";
    cin >> n;

    cout << "Enter " << n << " values:\n";

    // Read values from the user and insert them into the array.
    for (int i = 0; i < n; i++) {
        int value;
        cin >> value;
        ua.insert(value);
    }

    // Display the array after inserting all values.
    cout << "\nOriginal ";
    ua.display();

    // Search demo: ask the user for a value and check if it exists.
    int searchVal;
    cout << "\nEnter a value to search: ";
    cin >> searchVal;

    int index = ua.search(searchVal);

    if (index != -1)
        cout << "Value " << searchVal << " found at index " << index << endl;
    else
        cout << "Value " << searchVal << " not found." << endl;

    return 0;
}
