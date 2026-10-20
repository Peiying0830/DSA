// EmergencyQueue.cpp
// Separate heap priority queue; equal priorities use FIFO arrival order.
// DSA Final Assessment - Hospital Management System.

#include "EmergencyQueue.h"
#include <utility>
using namespace std;

// Smaller priority numbers are served first; arrival sequence breaks ties (FIFO).
bool EmergencyQueue::before(const EmergencyRecord& a, const EmergencyRecord& b) {
    return a.priority!=b.priority ? a.priority<b.priority : a.sequence<b.sequence;
}
// Restore the heap after appending a leaf: compare with parent (index-1)/2.
void EmergencyQueue::siftUp(size_t index) {
    while(index>0) {
        size_t parent=(index-1)/2;
        if(!before(waiting[index],waiting[parent])) break;
        swap(waiting[index],waiting[parent]);
        index=parent;
    }
}
// Restore the heap after replacing the root: promote the earlier of its children.
void EmergencyQueue::siftDown(size_t index) {
    for(;;) {
        size_t left=2*index+1;
        if(left>=waiting.size()) return;
        size_t right=left+1;
        size_t first=(right<waiting.size() && before(waiting[right],waiting[left])) ? right : left;
        if(!before(waiting[first],waiting[index])) return;
        swap(waiting[index],waiting[first]);
        index=first;
    }
}
// Insert into the priority heap using priority and monotonically increasing arrival sequence.
void EmergencyQueue::enqueue(const EmergencyRecord& record) {
    waiting.push_back(record);
    siftUp(waiting.size()-1);
}
// Remove the highest-priority waiting record; equal priorities are served oldest first.
bool EmergencyQueue::dequeue(EmergencyRecord& record) {
    if (waiting.empty()) return false;
    record=waiting.front();
    if(waiting.size()==1) {waiting.pop_back();return true;}
    waiting.front()=std::move(waiting.back());
    waiting.pop_back();
    siftDown(0);
    return true;
}
// Pop a copied heap to inspect service order without changing the live queue.
vector<EmergencyRecord> EmergencyQueue::records() const {
    auto copy = *this;
    vector<EmergencyRecord> result;
    EmergencyRecord record;
    while(copy.dequeue(record)) result.push_back(record);
    return result;
}
// Check waiting records to avoid duplicate or cross-queue patient entries.
bool EmergencyQueue::hasPatient(int patientID) const {
    for (const auto& record : waiting) if (record.patientID == patientID) return true;
    return false;
}
