// PatientBST.cpp
// Task 5: binary search tree keyed by Patient ID.
// DSA Final Assessment - Hospital Management System.

#include "PatientBST.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace std;

// Insert by ID: smaller keys go left, larger keys go right; duplicates are ignored.
void PatientBST::insert(unique_ptr<Node>& node, int id) {
        if (!node) node = make_unique<Node>(id);
        else if (id < node->id) insert(node->left, id);
        else if (id > node->id) insert(node->right, id);
    }

// Delete the matching key, handling leaf, one-child and two-child cases.
void PatientBST::erase(unique_ptr<Node>& node, int id) {
        if (!node) return;
        if (id < node->id) erase(node->left, id);
        else if (id > node->id) erase(node->right, id);
        // Zero/one child: transfer the surviving subtree into the removed node's position.
        else if (!node->left) node = move(node->right);
        else if (!node->right) node = move(node->left);
        else {
            // Two children: replace the key with the smallest key in the right subtree.
            Node* successor = node->right.get();
            while (successor->left) successor = successor->left.get();
            node->id = successor->id;
            erase(node->right, successor->id);
        }
    }

// Visit left subtree, current key, right subtree to collect ascending Patient IDs.
void PatientBST::inorder(const Node* node, vector<int>& ids) {
        if (!node) return;
        inorder(node->left.get(), ids);
        ids.push_back(node->id);
        inorder(node->right.get(), ids);
    }

// Insert by ID: smaller keys go left, larger keys go right; duplicates are ignored.
void PatientBST::insert(int id) { insert(root, id); }

// Delete the matching key, handling leaf, one-child and two-child cases.
void PatientBST::erase(int id) { erase(root, id); }

// Release all nodes through unique_ptr ownership and reset the empty root.
void PatientBST::clear() { root.reset(); }

// Follow one branch per comparison until the key is found or the branch ends.
bool PatientBST::contains(int id) const {
        const Node* node = root.get();
        while (node) {
            if (id == node->id) return true;
            node = id < node->id ? node->left.get() : node->right.get();
        }
        return false;
    }

// Return the IDs produced by in-order traversal without changing the tree.
vector<int> PatientBST::sortedIDs() const {
        vector<int> ids;
        inorder(root.get(), ids);
        return ids;
    }
