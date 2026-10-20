// HospitalData.h
// Composed application state, copied for Undo and serialised for autosave.
// DSA Final Assessment - Hospital Management System.

#pragma once
#include "Models.h"
#include "MedicalHistory.h"
#include "ServiceQueue.h"
#include "EmergencyQueue.h"
#include "PharmacyInventory.h"
#include "PrescriptionManager.h"
#include "BillingSystem.h"
#include <array>
#include <vector>
// Composition: independent data structures are owned by one application state.
// Copying this object provides the pre-operation snapshot used by the LIFO stack.
struct HospitalData {
    static constexpr int capacity = 100;
    std::array<Patient,capacity> patients{};
    int count = 0;
    std::vector<Doctor> doctors;
    std::vector<Appointment> appointments;
    std::vector<DoctorShift> shifts;
    MedicalHistory history;
    ServiceQueue serviceQueue;
    EmergencyQueue emergency;
    PharmacyInventory pharmacy;
    PrescriptionManager prescriptions;
    BillingSystem billing;
    // Demonstration prices, configurable from Services.
    std::vector<HospitalService> services{{1,"X-ray",8000},{2,"Blood test",3000},{3,"ECG",5000}};
    std::vector<VisitService> visitServices;
};
// Persistence also stores monotonic counters and the audit log outside Undo state.
struct SavedHospital {
    HospitalData data;
    int nextAppointmentID = 1, nextRecordID = 1;
    unsigned long long nextEmergencySequence = 1;
    std::vector<AuditEntry> audit;
    int nextPrescriptionID=1, nextInvoiceID=1;
    int nextPatientID=1;
    int nextDoctorID=1;
    int nextServiceOrderID=1;
};
