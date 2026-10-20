// Task 3: manually implemented linked stack for complete hospital snapshots.
#pragma once
#include "HospitalData.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

struct UndoEntry {
    HospitalData previous;
    std::string description;
};

class UndoStack {
    struct Node {
        UndoEntry entry;
        std::unique_ptr<Node> next;
        explicit Node(UndoEntry value) : entry(std::move(value)) {}
    };
    std::unique_ptr<Node> topNode;
public:
    UndoStack() = default;
    UndoStack(const UndoStack&) = delete;
    UndoStack& operator=(const UndoStack&) = delete;
    ~UndoStack() { clear(); }

    bool empty() const { return !topNode; }
    // Link a new snapshot before the current top: O(1) links, plus snapshot copy.
    void push(UndoEntry entry) {
        auto node=std::make_unique<Node>(std::move(entry));
        node->next=std::move(topNode);
        topNode=std::move(node);
    }
    UndoEntry& top() {
        if(empty()) throw std::underflow_error("Undo stack is empty.");
        return topNode->entry;
    }
    const UndoEntry& top() const {
        if(empty()) throw std::underflow_error("Undo stack is empty.");
        return topNode->entry;
    }
    // Unlink exactly the latest entry; the next node becomes the new top.
    void pop() {
        if(empty()) throw std::underflow_error("Undo stack is empty.");
        auto removed=std::move(topNode);
        topNode=std::move(removed->next);
    }
    // Release iteratively so a long session does not recurse through the chain.
    void clear() {
        while(!empty()) pop();
    }
};
