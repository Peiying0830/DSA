// PatientBST.h
// Task 5: binary search tree keyed by Patient ID.
// DSA Final Assessment - Hospital Management System.

#pragma once
#include <memory>
#include <vector>
using std::unique_ptr;
using std::make_unique;
using std::move;
using std::vector;

class PatientBST {
    struct Node {
        int id;
        unique_ptr<Node> left, right;
        explicit Node(int key) : id(key) {}
    };
    unique_ptr<Node> root;
    // Insert by ID: smaller keys go left, larger keys go right; duplicates are ignored.
    static void insert(unique_ptr<Node>& node, int id);
    // Delete the matching key, handling leaf, one-child and two-child cases.
    static void erase(unique_ptr<Node>& node, int id);
    // Visit left subtree, current key, right subtree to collect ascending Patient IDs.
    static void inorder(const Node* node, vector<int>& ids);
public:
    // Insert by ID: smaller keys go left, larger keys go right; duplicates are ignored.
    void insert(int id);
    // Delete the matching key, handling leaf, one-child and two-child cases.
    void erase(int id);
    // Release all nodes through unique_ptr ownership and reset the empty root.
    void clear();
    // Follow one branch per comparison until the key is found or the branch ends.
    bool contains(int id) const;
    // Return the IDs produced by in-order traversal without changing the tree.
    vector<int> sortedIDs() const;
};
