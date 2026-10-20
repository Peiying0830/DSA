// EmergencyQueue.h
// Separate heap priority queue; equal priorities use FIFO arrival order.
// DSA Final Assessment - Hospital Management System.

#pragma once
#include <cstddef>
#include <string>
#include <vector>
struct EmergencyRecord {
    int patientID, priority;
    unsigned long long sequence; // Increasing arrival number preserves FIFO ties.
    std::string reason;
};
// Priority is entered by the user; this demonstration does not assess symptoms.
class EmergencyQueue {
    // Array-backed min-heap. Heap ordering and repair are implemented manually.
    // Root index 0 is served next. Children of i are 2*i+1 and 2*i+2.
    // vector supplies storage only; no standard heap/priority-queue operations are used.
    std::vector<EmergencyRecord> waiting;
    static bool before(const EmergencyRecord& a, const EmergencyRecord& b); // Priority, then arrival.
    void siftUp(std::size_t index);
    void siftDown(std::size_t index);
public:
    // Append a leaf and repair upward; heap repair takes O(log n).
    void enqueue(const EmergencyRecord& record);
    // Remove the root and repair downward in O(log n); equal priorities use FIFO.
    bool dequeue(EmergencyRecord& record);
    // O(n) scan to avoid duplicate or cross-queue patient entries.
    bool hasPatient(int patientID) const;
    // O(n log n): drain a copied heap in service order without changing the live queue.
    std::vector<EmergencyRecord> records() const;
};
