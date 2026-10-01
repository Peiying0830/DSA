// Binary Search Tree - Lab Activity
#include <iostream>
#include <limits>
#include <string>
using namespace std;

// One node of the tree
struct Node {
    int value;       // the data stored in this node
    Node* left;      // left child (smaller values)
    Node* right;     // right child (larger values)

    Node(int v) : value(v), left(nullptr), right(nullptr) {}
};

class BST {
private:
    Node* root;      // empty tree at the start

    // Insert a value (recursive). Duplicates are checked before this is called.
    Node* insert(Node* node, int value) {
        if (node == nullptr)             // found the empty spot
            return new Node(value);
        if (value < node->value)         // smaller -> go left
            node->left = insert(node->left, value);
        else                             // larger -> go right
            node->right = insert(node->right, value);
        return node;
    }

    // Remove a value (recursive)
    Node* remove(Node* node, int value) {
        if (node == nullptr)
            return nullptr;

        if (value < node->value) {           // look in the left subtree
            node->left = remove(node->left, value);
        } else if (value > node->value) {    // look in the right subtree
            node->right = remove(node->right, value);
        } else {                             // this is the node to delete
            // Case 1 and 2: no child, or one child
            if (node->left == nullptr) {
                Node* temp = node->right;
                delete node;
                return temp;
            }
            if (node->right == nullptr) {
                Node* temp = node->left;
                delete node;
                return temp;
            }
            // Case 3: two children
            // Find the smallest value in the right subtree (the successor)
            Node* successor = node->right;
            while (successor->left != nullptr)
                successor = successor->left;
            node->value = successor->value;                      // copy it up
            node->right = remove(node->right, successor->value); // delete the old one
        }
        return node;
    }

    // In-order traversal: left, node, right (gives sorted order)
    void inorder(Node* node) {
        if (node != nullptr) {
            inorder(node->left);
            cout << node->value << " ";
            inorder(node->right);
        }
    }

    // Free all nodes
    void destroy(Node* node) {
        if (node != nullptr) {
            destroy(node->left);
            destroy(node->right);
            delete node;
        }
    }

public:
    BST() : root(nullptr) {}
    ~BST() { destroy(root); }

    // Search for a value. Returns true if found.
    bool search(int value) {
        Node* node = root;
        while (node != nullptr) {
            if (value == node->value)        // found it
                return true;
            else if (value < node->value)    // go left
                node = node->left;
            else                             // go right
                node = node->right;
        }
        return false;                        // reached the end, not found
    }

    // Insert a value. Returns false if it already exists.
    bool insert(int value) {
        if (search(value))                   // duplicates are not allowed
            return false;
        root = insert(root, value);
        return true;
    }

    // Remove a value. Returns false if it is not in the tree.
    bool remove(int value) {
        if (!search(value))
            return false;
        root = remove(root, value);
        return true;
    }

    // Display all items in sorted order
    void display() {
        inorder(root);
        cout << endl;
    }
};

// Retry invalid input; return false when input ends or cannot be read.
bool readNumber(const string& message, int& number) {
    cout << message;
    while (!(cin >> number)) {
        if (cin.eof() || cin.bad())
            return false;
        cin.clear();                                              // reset error state
        cin.ignore(numeric_limits<streamsize>::max(), '\n');      // discard bad input
        cout << "Please enter a whole number: ";
    }
    return true;
}

int main() {
    BST tree;

    // Task 1: initialise the tree with the given values
    int initial[] = {23, 45, 12, -5, 35, -45, 78};
    for (int v : initial)
        tree.insert(v);
    cout << "Initial tree: ";
    tree.display();

    // Menu so the user can try each function
    int choice;
    do {
        cout << "\n1. Insert  2. Search  3. Remove  4. Display  5. Exit\n";
        if (!readNumber("Choose: ", choice))
            break;

        if (choice == 1) {
            // Task 2: insert a user input value
            int v;
            if (!readNumber("Enter value to insert: ", v))
                break;
            if (tree.insert(v))
                cout << v << " inserted.\n";
            else
                cout << v << " already exists. Duplicates are not allowed.\n";
        } else if (choice == 2) {
            // Task 3: search for a user input value
            int v;
            if (!readNumber("Enter value to search: ", v))
                break;
            cout << v << (tree.search(v) ? " found.\n" : " not found.\n");
        } else if (choice == 3) {
            // Task 4: remove a user input value
            int v;
            if (!readNumber("Enter value to remove: ", v))
                break;
            if (tree.remove(v))
                cout << v << " removed.\n";
            else
                cout << v << " is not in the tree.\n";
        } else if (choice == 4) {
            cout << "Tree (sorted): ";
            tree.display();
        } else if (choice != 5) {
            cout << "Invalid choice.\n";
        }
    } while (choice != 5);

    cout << "Goodbye.\n";
    return 0;
}
