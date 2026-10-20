// HospitalSystem.h
// Hospital menus and coordination between independent OOP modules.
// DSA Final Assessment - Hospital Management System.

#pragma once
#include <array>
#include <stack>
#include <string>
#include <vector>
using std::array;
using std::stack;
using std::string;
using std::vector;

#include "Models.h"
#include "PatientBST.h"
#include "Input.h"
#include "MedicalHistory.h"
#include "ServiceQueue.h"
#include "ReferralGraph.h"
#include "Algorithms.h"
#include "HospitalData.h"
#include "Persistence.h"
#include "ScheduleBook.h"
class HospitalSystem {
    static constexpr int capacity = 100;
    HospitalData state;
    struct UndoEntry { HospitalData previous; string description; };
    stack<UndoEntry> undo; // Task 3: LIFO; last completed change is undone first.
    PatientBST index;
    int nextServiceOrderID = 1;
    int nextDoctorID = 1;
    int nextPatientID = 1; // Persisted; never reused after deletion or undo.
    int nextRecordID = 1;
    ReferralGraph graph;
    int nextAppointmentID = 1; // Never reuse an ID after cancellation or undo.
    unsigned long long nextEmergencySequence = 1;
    vector<AuditEntry> audit;
    Persistence storage;
    string pendingAction;
    int nextPrescriptionID=1, nextInvoiceID=1;
    // Restore the fixed demonstration catalogue and records used after a reset.
    void loadDefaultData();
    // Autosave the completed change; restore its previous state if saving fails.
    bool commitChange();
    // Collect current records, counters and audit entries into one consistent save object.
    SavedHospital snapshot() const;
    // Find the authoritative patient array index, or return -1 when absent.
    int patientPosition(int id) const;
    // Explain an existing queue entry and how to resolve it without changing data.
    bool showWaitingConflict(int patientID, int appointmentID = 0) const;
    // LIFO: push the pre-change state onto the Undo stack before modifying records.
    void remember(const string& description);
    // Recreate BST keys from the patient array after loading or restoring a snapshot.
    void rebuildIndex();
    // Display all basic fields of one patient record.
    static void printPatient(const Patient& patient);
    void printMedicalHistory(const vector<MedicalRecord>& records, int patientID) const;
    // Read and validate user-entered patient fields while retaining the selected ID.
    Patient readPatient(int id);
    // Generate the next patient ID/check capacity, then insert into the array and BST and autosave.
    void addPatient();
    // Update the patient and their waiting-queue labels without changing queue order.
    void editPatient();
    // Prevent dangling references, shift array records left and remove the BST key.
    void deletePatient();
    // Check active doctor/patient bookings in the same slot, optionally excluding the edited booking.
    bool conflict(const Appointment& proposed, int excludedID = -1) const;
    // Display the doctor directory, optionally including every shift.
    void showDoctors(bool includeShifts = true) const;
    // Locate a doctor by their unique ID rather than assuming consecutive IDs.
    int doctorPosition(int id) const;
    // Register a unique user-entered doctor and save the change as an undoable operation.
    void addDoctor();
    void importDoctors();
    // Calculate free slots from entered shifts and currently active appointments.
    void showAvailableSlots() const;
    bool printAvailableSlots(int doctorID, const string& date, int patientID = 0) const;
    // Filter by doctor and date, then display matching bookings in time order.
    void showDoctorAppointments(int selectedDoctorID = 0) const;
    // Add/view/remove dated doctor shifts, rejecting overlap and removal of booked periods.
    void showDoctorShifts() const;
    void scheduleMenu();
    // Check in into the FIFO queue, record a no-show or complete a served appointment.
    void updateAppointmentStatus();
    // Combine patient basics, history, appointments, queues, prescriptions and invoices.
    void showPatientProfile() const;
    // Display persisted operation descriptions and Malaysia-time timestamps.
    void showAuditLog() const;
    // Serve highest-priority cases first; equal priorities retain arrival order.
    void emergencyMenu();
    // Manage user-entered medicines, restocking, price updates and prescriptions.
    void pharmacyMenu();
    void medicineMenu();
    void prescriptionMenu();
    void pharmacyAction(int choice);
    void importMedicines();
    void importPatients();
    void loadMenu();
    void loadDataBundle(int category = 0);
    void exportData() const;
    void restoreBackup();
    void clearData();
    // View automatic invoices, record confirmed full payments and display receipts.
    void billingMenu();
    void invoiceViewMenu();
    void payInvoice();
    void servicesMenu();
    void clinicalMenu();
    void treatmentOrdersMenu();
    void showServices() const;
    void visitServicesMenu();
    void servicePricesMenu();
    void addVisitService(int appointmentID);
    void showVisitServices(int appointmentID) const;
    void removeVisitService(int appointmentID);
    void changeServicePrice();
    void addService();
    void updateDoctorFee();
    bool completeVisit(Appointment& appointment);
    // Read staff-entered medicines/quantities/instructions; do not reserve stock yet.
    void createPrescription();
    // Validate all stock first, then consume it and create one invoice in a saved transaction.
    void dispensePrescription(int prescriptionID = 0);
    // Display all prescriptions or only those belonging to a selected patient.
    void showPrescriptions(int patientID=0) const;
    // List invoices and sum unpaid balances for all patients or one selected patient.
    void showInvoices(int patientID=0) const;
    // Display fixed charge details; paid invoices are shown as payment receipts.
    void showInvoice(const Invoice& invoice) const;
    // Require an existing patient, doctor and shift; reject slot conflicts before booking.
    void addAppointment();
    // Display current and retained appointments chronologically, with their statuses.
    void showAppointments(int patientID = 0) const;
    void printAppointment(const Appointment& appointment) const;
    // Cancel while retaining history, or reschedule a Scheduled appointment within a shift.
    void changeAppointment(bool cancel);
    // LIFO: restore the most recent snapshot, autosave it, then pop that Undo entry.
    void undoLast();
    // Dispatch array/BST operations until the user returns to the main menu.
    void categoryMenu(int category);
    void patientMenu();
    // Dispatch booking, directory, availability, shift and status operations.
    void appointmentMenu();
    void doctorMenu();
    void doctorView();

    // Add, remove or traverse chronological linked-list medical records by patient.
    void visitRecordsMenu();
    static bool readMedicalDetails(string& details);
    // Demonstrate FIFO: enqueue at the rear, serve the front and display waiting patients.
    void queueMenu();
    void normalQueueMenu();
    // Run DFS and BFS from the selected department using the prescribed directed graph.
    void graphMenu();
    // Copy only manually entered patient records so demonstrations do not change stored data.
    vector<Patient> algorithmData() const;
    // Compare linear/binary searches for the same ID; binary search uses an ID-sorted copy.
    void searchDemo();
    // Compare merge/bubble sorts on identical copies, with key/order selection and timing.
    void sortingDemo();

public:
    // Load saved data if present, restore ID counters and rebuild the BST index.
    explicit HospitalSystem(const std::filesystem::path& dataDirectory);
    // Run the main console menu and route each choice to its responsible module.
    void run();
};
