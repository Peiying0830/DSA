// PrescriptionManager.cpp
// Prescription records and Pending/Dispensed/Cancelled transitions.
// DSA Final Assessment - Hospital Management System.

#include "PrescriptionManager.h"
#include <set>
#include "Input.h"
using namespace std;

// Validate unique prescription/drug IDs, bounded quantities and an allowed status.
bool PrescriptionManager::add(const Prescription& prescription) {
    if (prescription.id<1 || prescription.id>999999 || find(prescription.id) ||
        prescription.patientID<1 || prescription.patientID>999999 || prescription.doctorID<1 || prescription.doctorID>999999 || !Input::validDate(prescription.date) ||
        prescription.items.empty() || prescription.items.size()>20 ||
        (prescription.status!="Pending" && prescription.status!="Dispensed" && prescription.status!="Cancelled")) return false;
    if(prescription.appointmentID<0 || prescription.appointmentID>999999 || prescription.invoiceID<0 || prescription.invoiceID>999999 || prescription.invoiceItemOffset<0 || prescription.invoiceItemOffset>19 ||
       (prescription.status!="Dispensed" && (prescription.invoiceID || prescription.invoiceItemOffset))) return false;
    set<int> ids;
    for (const auto& item : prescription.items)
        if (item.drugID<1 || item.drugID>999999 || item.quantity<1 || item.quantity>100000 || !Input::validText(item.instructions) || !ids.insert(item.drugID).second) return false;
    prescriptions.push_back(prescription); return true;
}
// Read a retained prescription by ID, including closed records.
const Prescription* PrescriptionManager::find(int id) const {
    for (const auto& prescription : prescriptions) if (prescription.id==id) return &prescription;
    return nullptr;
}
// Allow only Pending -> Dispensed or Pending -> Cancelled transitions.
bool PrescriptionManager::setStatus(int id, const string& status) {
    if (status!="Dispensed" && status!="Cancelled") return false;
    for (auto& prescription : prescriptions)
        if (prescription.id==id && prescription.status=="Pending") { prescription.status=status; return true; }
    return false;
}
// Expose a read-only view for queries, profile display and persistence.
const vector<Prescription>& PrescriptionManager::records() const { return prescriptions; }

// Capture the exact invoice lines charged for this dispensing transaction.
bool PrescriptionManager::markDispensed(int id, int invoiceID, int firstItem, int appointmentID) {
    if(invoiceID<1 || invoiceID>999999 || firstItem<0 || firstItem>19) return false;
    for(auto& prescription:prescriptions) if(prescription.id==id && prescription.status=="Pending") {
        prescription.status="Dispensed";
        prescription.invoiceID=invoiceID;
        prescription.invoiceItemOffset=firstItem;
        if(appointmentID) prescription.appointmentID=appointmentID;
        return true;
    }
    return false;
}

// Service removal can shift the positions of captured medication lines.
void PrescriptionManager::adjustInvoiceOffsetsAfterRemoval(int invoiceID, int removedItem) {
    for(auto& prescription:prescriptions)
        if(prescription.invoiceID==invoiceID && prescription.invoiceItemOffset>removedItem)
            --prescription.invoiceItemOffset;
}
