// ServiceQueue.h
// Task 4: circular-array queue demonstrating FIFO (First-In, First-Out).
// DSA Final Assessment - Hospital Management System.

#pragma once
#include <array>
#include <string>
#include <vector>
// One waiting patient's identity, requested service and informational status.
struct QueueRecord {
    int patientID;
    std::string name, department, service, status;
};
// Circular array queue: status is informational; service order remains FIFO.
class ServiceQueue {
    static constexpr int capacity = 100;
    std::array<QueueRecord, capacity> data{};
    int first = 0, count = 0; // Front array index and number of live entries.
public:
    // FIFO: append at (first + count) modulo capacity; reject a full queue.
    bool enqueue(const QueueRecord& record);
    // FIFO: remove the oldest front record, advance first with wraparound and reduce count.
    bool dequeue(QueueRecord& record);
    // Inspect the next patient without removing them; return nullptr when empty.
    const QueueRecord* front() const;
    // Check live circular-array entries to prevent duplicate waiting records.
    bool hasPatient(int patientID) const;
    // Collect live entries in FIFO service order, including entries after wraparound.
    std::vector<QueueRecord> records() const;
    // Return the number of live entries, not the backing array capacity.
    int size() const { return count; }
};
