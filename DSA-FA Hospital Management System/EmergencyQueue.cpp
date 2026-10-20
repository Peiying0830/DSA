// EmergencyQueue.cpp
// Separate heap priority queue; equal priorities use FIFO arrival order.
// DSA Final Assessment - Hospital Management System.

#include "EmergencyQueue.h"
using namespace std;

// Insert into the priority heap using priority and monotonically increasing arrival sequence.
void EmergencyQueue::enqueue(const EmergencyRecord& record) { waiting.push(record); }
// Remove the highest-priority waiting record; equal priorities are served oldest first.
bool EmergencyQueue::dequeue(EmergencyRecord& record) {
    if (waiting.empty()) return false;
    record = waiting.top(); waiting.pop(); return true;
}
// Pop a copied heap to inspect service order without changing the live queue.
vector<EmergencyRecord> EmergencyQueue::records() const {
    auto copy = waiting;
    vector<EmergencyRecord> result;
    while (!copy.empty()) { result.push_back(copy.top()); copy.pop(); }
    return result;
}
// Check waiting records to avoid duplicate or cross-queue patient entries.
bool EmergencyQueue::hasPatient(int patientID) const {
    for (const auto& record : records()) if (record.patientID == patientID) return true;
    return false;
}
