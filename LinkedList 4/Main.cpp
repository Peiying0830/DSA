#include "List.cpp"
#include <iostream>
using namespace std;
// Main function to demonstrate linked list operations
int main() {
    List<int> myList;

    // Add nodes
    myList.addAtBeginning(10);
    myList.addAtEnd(20);
    myList.addAtPosition(2, 15);
    myList.traverse(); // Output: 10 -> 15 -> 20 -> NULL

    // Delete nodes
    myList.deleteFromBeginning();
    myList.traverse(); // Output: 15 -> 20 -> NULL

    myList.deleteFromEnd();
    myList.traverse(); // Output: 15 -> NULL

    // Search nodes
    cout << "Found 15: " << (myList.search(15) ? "Yes" : "No") << endl;
    cout << "Found 20: " << (myList.search(20) ? "Yes" : "No") << endl;

    return 0;
}
