#include <iostream>
using namespace std;
#include "Node.cpp"

// Template class for Linked List
template <typename T>
class List {
private:
    Node<T>* head;   // Pointer to the first node in the list

public:
    // Constructor to initialize an empty list
    List(){ head = nullptr; } 

    // Destructor to free all nodes
    ~List() {
        while (head) {
            Node<T>* temp = head;
            head = head->next;
            delete temp;
        }
    }

    // Add a node at the beginning of the list
    void addAtBeginning(T value) {
        head = new Node<T>(value, head); // New node becomes the head
    }

    // Add a node at a specific position (1-based index)
    void addAtPosition(int position, T value) {
        if (position <= 0) {
            cout << "Invalid position!\n";
            return;
        }

        if (position == 1) {
            addAtBeginning(value);
            return;
        }

        Node<T>* current = head;
        int currentPos = 1;

        // Traverse to the node before the target position
        while (current && currentPos < position - 1) {
            current = current->next;
            currentPos++;
        }

        if (!current) {
            cout << "Position out of bounds!\n";
            return;
        }

        // Create and insert the new node
        current->next = new Node<T>(value, current->next);
    }

    // Add a node at the end of the list
    void addAtEnd(T value) {
        if (!head) {
            addAtBeginning(value); // If list is empty, add to the beginning
            return;
        }

        Node<T>* current = head;

        // Traverse to the last node
        while (current->next) {
            current = current->next;
        }

        // Add new node at the end
        current->next = new Node<T>(value);
    }

    // Delete the first node of the list
    void deleteFromBeginning() {
        if (!head) {
            cout << "List is empty!\n";
            return;
        }

        Node<T>* temp = head; // Temporary pointer to the head
        head = head->next;    // Move head to the next node
        delete temp;          // Delete the old head
    }

    // Delete a node at a specific position (1-based index)
    void deleteFromPosition(int position) {
        if (position <= 0 || !head) {
            cout << "Invalid position or empty list!\n";
            return;
        }

        if (position == 1) {
            deleteFromBeginning();
            return;
        }

        Node<T>* current = head;
        int currentPos = 1;

        // Traverse to the node before the target position
        while (current->next && currentPos < position - 1) {
            current = current->next;
            currentPos++;
        }

        if (!current->next) {
            cout << "Position out of bounds!\n";
            return;
        }

        Node<T>* temp = current->next;
        current->next = temp->next; // Skip the node to be deleted
        delete temp;
    }

    // Delete the last node of the list
    void deleteFromEnd() {
        if (!head) {
            cout << "List is empty!\n";
            return;
        }

        if (!head->next) {
            deleteFromBeginning(); // If only one node, delete it
            return;
        }

        Node<T>* current = head;

        // Traverse to the second-to-last node
        while (current->next->next) {
            current = current->next;
        }

        delete current->next; // Delete the last node
        current->next = nullptr;
    }

    // Search for a value in the list
    bool search(T value) const {
        Node<T>* current = head;

        // Traverse the list to find the value
        while (current) {
            if (current->data == value) {
                return true;
            }
            current = current->next;
        }

        return false; // Value not found
    }

    // Traverse and display the list
    void traverse() const {
        Node<T>* current = head;

        // Print each node's data
        while (current) {
            cout << current->data << " -> ";
            current = current->next;
        }

        cout << "NULL\n"; // End of list
    }
};
