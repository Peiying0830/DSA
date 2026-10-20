// Models.h
// Shared patient, doctor, appointment, shift and audit records.
// DSA Final Assessment - Hospital Management System.

#pragma once
#include <string>
#include <iomanip>
#include <sstream>
using std::string;

inline string formatPatientID(int id) {
    std::ostringstream out;
    out << 'P' << std::setfill('0') << std::setw(3) << id;
    return out.str();
}

inline string formatDoctorID(int id) {
    std::ostringstream out;
    out << 'D' << std::setfill('0') << std::setw(3) << id;
    return out.str();
}

inline string formatID(const string& prefix, int id) {
    std::ostringstream out;
    out << prefix << std::setfill('0') << std::setw(3) << id;
    return out.str();
}

// Task 1 fields; the patient array owns the authoritative basic record.
struct Patient {
    int id = 0;
    string name;
    int age = 0;
    string gender, department, condition;
};

// User-entered doctor identity; ID need not be consecutive.
struct Doctor { int id; string name, department; long long consultationFeeCents=0; };
struct HospitalService { int id; string name; long long priceCents; };
struct VisitService {
    int id, appointmentID, serviceID, quantity;
    string description;
    long long unitPriceCents;
};

// A retained booking record; cancellation changes status rather than deleting it.
struct Appointment {
    int id, patientID, doctorID;
    string date, time, reason;
    string status = "Scheduled";
};

// Weekly weekday name (or legacy date), inclusive start and exclusive end.
struct DoctorShift { int doctorID; string date, start, end; };
// Logs are kept independently of Undo so reversing an action is also recorded.
struct AuditEntry { string timestamp, action; };
