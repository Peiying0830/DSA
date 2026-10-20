// EmergencyQueue.h
// Separate heap priority queue; equal priorities use FIFO arrival order.
// DSA Final Assessment - Hospital Management System.

#pragma once
#include <cstddef>
#include <string>
#include <vector>
struct EmergencyRecord {
    int patientID, priority;
    unsigned long long sequence;
    std::string reason;
};
// Priority is entered by the user; this demonstration does not assess symptoms.
class EmergencyQueue {
    // Array-backed min-heap. Heap ordering and repair are implemented manually.
    std::vector<EmergencyRecord> waiting;
    static bool before(const EmergencyRecord& a, const EmergencyRecord& b);
    void siftUp(std::size_t index);
    void siftDown(std::size_t index);
public:
    // Insert into the priority heap using priority and monotonically increasing arrival sequence.
    void enqueue(const EmergencyRecord& record);
    // Remove the highest-priority waiting record; equal priorities are served oldest first.
    bool dequeue(EmergencyRecord& record);
    // Check waiting records to avoid duplicate or cross-queue patient entries.
    bool hasPatient(int patientID) const;
    // Pop a copied heap to inspect service order without changing the live queue.
    std::vector<EmergencyRecord> records() const;
};
