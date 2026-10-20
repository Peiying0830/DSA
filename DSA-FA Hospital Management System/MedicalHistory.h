// MedicalHistory.h
// Task 2: chronological singly linked list of medical records.
// DSA Final Assessment - Hospital Management System.

#pragma once
#include <memory>
#include <string>
#include <vector>
struct MedicalRecord {
    int id, patientID;
    std::string date, type, details;
    int appointmentID = 0; // Legacy records have no known visit; never guess their appointment.
};
class MedicalHistory {
    struct Node {
        MedicalRecord record;
        std::unique_ptr<Node> next;
        explicit Node(const MedicalRecord& value) : record(value) {}
    };
    std::unique_ptr<Node> head;
public:
    // Deep-copy records into independent nodes so Undo snapshots do not share links.
    MedicalHistory() = default;
    // Deep-copy records into independent nodes so Undo snapshots do not share links.
    MedicalHistory(const MedicalHistory& other);
    // Copy using a temporary list and swap ownership; preserve self-assignment safety.
    MedicalHistory& operator=(const MedicalHistory& other);
    // Insert by date, then record ID, so traversal is chronological even for backdated entries.
    void add(const MedicalRecord& record);
    // Relink the owning pointer around an incorrect record and release its removed node.
    bool remove(int recordID);
    // Traverse nodes to check whether a patient still has linked medical records.
    bool hasPatient(int patientID) const;
    // Traverse in list order, optionally collecting only one patient's records.
    std::vector<MedicalRecord> records(int patientID = 0) const;
};
