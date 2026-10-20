// MedicalHistory.cpp
// Task 2: chronological singly linked list of medical records.
// DSA Final Assessment - Hospital Management System.

#include "MedicalHistory.h"
using namespace std;

// Deep-copy records into independent nodes so Undo snapshots do not share links.
MedicalHistory::MedicalHistory(const MedicalHistory& other) {
    for (const auto& record : other.records()) add(record);
}
// Copy using a temporary list and swap ownership; preserve self-assignment safety.
MedicalHistory& MedicalHistory::operator=(const MedicalHistory& other) {
    if (this != &other) {
        MedicalHistory copy(other);
        head.swap(copy.head);
    }
    return *this;
}
// Insert by date, then record ID, so traversal is chronological even for backdated entries.
void MedicalHistory::add(const MedicalRecord& record) {
    auto node = make_unique<Node>(record);
    auto* link = &head;
    while (*link && ((*link)->record.date < record.date ||
           ((*link)->record.date == record.date && (*link)->record.id < record.id)))
        link = &(*link)->next;
    node->next = move(*link);
    *link = move(node);
}
// Relink the owning pointer around an incorrect record and release its removed node.
bool MedicalHistory::remove(int recordID) {
    auto* link = &head;
    while (*link && (*link)->record.id != recordID) link = &(*link)->next;
    if (!*link) return false;
    *link = move((*link)->next);
    return true;
}
// Traverse nodes to check whether a patient still has linked medical records.
bool MedicalHistory::hasPatient(int patientID) const {
    for (auto* node = head.get(); node; node = node->next.get())
        if (node->record.patientID == patientID) return true;
    return false;
}
// Traverse in list order, optionally collecting only one patient's records.
vector<MedicalRecord> MedicalHistory::records(int patientID) const {
    vector<MedicalRecord> result;
    for (auto* node = head.get(); node; node = node->next.get())
        if (!patientID || node->record.patientID == patientID) result.push_back(node->record);
    return result;
}
