// UndoStack.h
// Task 3: manually implemented linked stack for complete hospital snapshots.
// DSA Final Assessment - Hospital Management System.
#pragma once
#include "HospitalData.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

// One undoable action: restore previous to reverse the action described below.
struct UndoEntry {
    HospitalData previous;
    std::string description;
};

class UndoStack {
    // Each node owns one snapshot and the link to the next older operation.
    // unique_ptr manages memory; the stack links and operations are implemented here.
    struct Node {
        UndoEntry entry;
        std::unique_ptr<Node> next;
        explicit Node(UndoEntry value) : entry(std::move(value)) {}
    };
    std::unique_ptr<Node> topNode; // Null means empty; otherwise the latest action.
public:
    // Start empty. Disable copying because the stack owns its chain of nodes.
    UndoStack() = default;
    UndoStack(const UndoStack&) = delete;
    UndoStack& operator=(const UndoStack&) = delete;
    ~UndoStack() { clear(); } // Release all snapshots when the session ends.

    // O(1): inspecting the top pointer does not traverse the linked list.
    bool empty() const { return !topNode; }
    // Link a new snapshot before the current top: O(1) links, plus snapshot copy.
    void push(UndoEntry entry) {
        auto node=std::make_unique<Node>(std::move(entry));
        // Preserve the old chain underneath the new node (LIFO).
        node->next=std::move(topNode);
        topNode=std::move(node);
    }
    // O(1): inspect the latest snapshot without removing it; reject an empty stack.
    UndoEntry& top() {
        if(empty()) throw std::underflow_error("Undo stack is empty.");
        return topNode->entry;
    }
    // Read-only access for operation-log display and other const callers.
    const UndoEntry& top() const {
        if(empty()) throw std::underflow_error("Undo stack is empty.");
        return topNode->entry;
    }
    // Unlink exactly the latest entry; the next node becomes the new top.
    void pop() {
        if(empty()) throw std::underflow_error("Undo stack is empty.");
        auto removed=std::move(topNode);
        topNode=std::move(removed->next);
        // removed now owns no successor; only its own snapshot is destroyed here.
    }
    // O(k) node removals for k entries, plus releasing the stored snapshot contents.
    // Release iteratively so a long session does not recurse through the chain.
    void clear() {
        while(!empty()) pop();
    }
};
