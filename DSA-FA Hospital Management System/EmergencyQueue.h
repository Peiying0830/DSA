// EmergencyQueue.h
// Separate heap priority queue; equal priorities use FIFO arrival order.
// DSA Final Assessment - Hospital Management System.

#pragma once
#include <queue>
#include <string>
#include <vector>
struct EmergencyRecord {
    int patientID, priority;
    unsigned long long sequence;
    std::string reason;
};
// Priority is entered by the user; this demonstration does not assess symptoms.
class EmergencyQueue {
    // priority_queue places the record that loses no comparison at the top.
    // Lower priority numbers win; lower sequence numbers break equal-priority ties.
    struct Compare {
        bool operator()(const EmergencyRecord& a, const EmergencyRecord& b) const {
            if (a.priority != b.priority) return a.priority > b.priority;
            return a.sequence > b.sequence;
        }
    };
    std::priority_queue<EmergencyRecord,std::vector<EmergencyRecord>,Compare> waiting;
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
