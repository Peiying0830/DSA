// PrescriptionManager.h
// Prescription records and Pending/Dispensed/Cancelled transitions.
// DSA Final Assessment - Hospital Management System.

#pragma once
#include <string>
#include <vector>
struct PrescriptionItem { int drugID=0, quantity=0; std::string instructions; };
struct Prescription {
    int id=0, patientID=0, doctorID=0;
    std::string date, status="Pending";
    std::vector<PrescriptionItem> items;
    int appointmentID=0, invoiceID=0, invoiceItemOffset=0;
};
class PrescriptionManager {
    std::vector<Prescription> prescriptions;
public:
    // Validate unique prescription/drug IDs, bounded quantities and an allowed status.
    bool add(const Prescription& prescription);
    // Read a retained prescription by ID, including closed records.
    const Prescription* find(int id) const;
    // Allow only Pending -> Dispensed or Pending -> Cancelled transitions.
    bool setStatus(int id, const std::string& status);
    bool markDispensed(int id, int invoiceID, int firstItem, int appointmentID = 0);
    void adjustInvoiceOffsetsAfterRemoval(int invoiceID, int removedItem);
    // Expose a read-only view for queries, profile display and persistence.
    const std::vector<Prescription>& records() const;
};
