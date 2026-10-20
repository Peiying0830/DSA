// ServiceQueue.cpp
// Task 4: circular-array queue demonstrating FIFO (First-In, First-Out).
// DSA Final Assessment - Hospital Management System.

#include "ServiceQueue.h"
using namespace std;

// FIFO: append at (first + count) modulo capacity; reject a full queue.
bool ServiceQueue::enqueue(const QueueRecord& record) {
    if (count == capacity) return false;
    // Modulo reuses freed array positions instead of shifting every waiting record.
    data[(first + count) % capacity] = record;
    ++count;
    return true;
}
// FIFO: remove the oldest front record, advance first with wraparound and reduce count.
bool ServiceQueue::dequeue(QueueRecord& record) {
    if (!count) return false;
    // The front is always the earliest patient still waiting: FIFO.
    record = data[first];
    data[first] = QueueRecord{};
    first = (first + 1) % capacity;
    --count;
    return true;
}
// Inspect the next patient without removing them; return nullptr when empty.
const QueueRecord* ServiceQueue::front() const { return count ? &data[first] : nullptr; }
// Check live circular-array entries to prevent duplicate waiting records.
bool ServiceQueue::hasPatient(int patientID) const {
    for (int i = 0; i < count; ++i)
        if (data[(first + i) % capacity].patientID == patientID) return true;
    return false;
}
// Collect live entries in FIFO service order, including entries after wraparound.
vector<QueueRecord> ServiceQueue::records() const {
    vector<QueueRecord> result;
    for (int i = 0; i < count; ++i) result.push_back(data[(first + i) % capacity]);
    return result;
}
