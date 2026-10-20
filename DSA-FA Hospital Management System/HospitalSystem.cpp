// HospitalSystem.cpp
// Hospital menus and coordination between independent OOP modules.
// DSA Final Assessment - Hospital Management System.

#include "HospitalSystem.h"
#include "ConsoleUI.h"
#include "CsvReader.h"
#include <algorithm>
#include <map>
#include <fstream>
#include <set>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace std;

namespace {
template<typename T>
string lowerResult(const string& firstName, T first, const string& secondName, T second,
                   const string& metric) {
    if(first==second) return metric+": Tie.";
    return metric+": "+(first<second ? firstName : secondName)+" (lower is better).";
}
}

// Find the authoritative patient array index, or return -1 when absent.
int HospitalSystem::patientPosition(int id) const {
        for (int i = 0; i < state.count; ++i)
            if (state.patients[i].id == id) return i;
        return -1;
    }


// Recreate BST keys from the patient array after loading or restoring a snapshot.
void HospitalSystem::rebuildIndex() {
        index.clear();
        for (int i = 0; i < state.count; ++i) index.insert(state.patients[i].id);
    }

// Display all basic fields of one patient record.
void HospitalSystem::printPatient(const Patient& patient) {
        ConsoleUI::rule();
        ConsoleUI::row("ID: " + formatPatientID(patient.id) + " | " + patient.name);
        ConsoleUI::row("Age: " + to_string(patient.age) + "     Gender: " + patient.gender);
        ConsoleUI::field("Department", patient.department);
        ConsoleUI::field("Condition", patient.condition);
        ConsoleUI::rule();
    }

// Present each medical record in its own frame, preserving chronological order.
void HospitalSystem::printMedicalHistory(const vector<MedicalRecord>& records, int patientID) const {
    for (const auto& record : records) {
        ConsoleUI::RecordBox frame(true);
        ConsoleUI::row("Patient " + formatPatientID(patientID) + " | " + record.date);
        ConsoleUI::row("Record " + formatID("R",record.id) + " | " + record.type);
        if (record.appointmentID) {
            auto visit=find_if(state.appointments.begin(),state.appointments.end(),[&](const Appointment& appointment){return appointment.id==record.appointmentID;});
            if (visit!=state.appointments.end()) {
                ConsoleUI::field("Visit", "Appointment " + formatID("A",visit->id));
                ConsoleUI::field("Time", visit->time);
                const auto& doctor=state.doctors[doctorPosition(visit->doctorID)];
                ConsoleUI::field("Doctor", formatDoctorID(doctor.id) + " | " + doctor.name);
                ConsoleUI::field("Department", doctor.department);
            }
        } else ConsoleUI::field("Visit", "Older record: appointment not recorded");
        istringstream lines(record.details);
        string line;
        while (getline(lines, line)) {
            // Wrap at words instead of cutting words across the right border.
            istringstream words(line);
            string word, wrapped;
            while (words >> word) {
                if (!wrapped.empty() && ConsoleUI::displayWidth(wrapped + " " + word) > ConsoleUI::contentWidth) {
                    ConsoleUI::row(wrapped);
                    wrapped.clear();
                }
                if (!wrapped.empty()) wrapped += ' ';
                wrapped += word;
            }
            if (!wrapped.empty()) ConsoleUI::row(wrapped);
        }
    }
}

// Read and validate user-entered patient fields while retaining the selected ID.
Patient HospitalSystem::readPatient(int id) {
        Patient patient;
        patient.id = id;
        patient.name = Input::name("Patient name: ");
        patient.age = Input::number("Age (0-130): ", 0, 130);
        int gender = Input::number("Gender: 1. Female  2. Male  3. Other/unspecified: ", 1, 3);
        patient.gender = gender == 1 ? "Female" : gender == 2 ? "Male" : "Other/unspecified";
        patient.department = Input::text("Department: ", 100);
        patient.condition = Input::text("Medical condition: ");
        return patient;
    }

// Generate the next patient ID/check capacity, then insert into the array and BST and autosave.
void HospitalSystem::addPatient() {
        if (state.count >= capacity) { ConsoleUI::message("!", "Patient array is full."); return; }
        if (nextPatientID > 999999) { ConsoleUI::message("!", "Patient ID limit reached."); return; }
        int id = nextPatientID;
        ConsoleUI::panel([&](ostream& out) { out << "Generated Patient ID: " << formatPatientID(id) << '\n'; });
        Patient patient = readPatient(id);
        remember("Add patient " + formatPatientID(id));
        state.patients[state.count++] = patient;
        index.insert(id);
        ++nextPatientID;
        if (!commitChange()) { --nextPatientID; return; }
        ConsoleUI::panel([&](ostream& out) { out << "Patient added to array and BST. ID: " << formatPatientID(id) << '\n'; });
    }

// Update the patient and their waiting-queue labels without changing queue order.
void HospitalSystem::editPatient() {
        int id = Input::patientID("Patient ID to update: ");
        int position = patientPosition(id);
        if (position < 0) { ConsoleUI::message("!", "Patient not found."); return; }
        printPatient(state.patients[position]);
        Patient updated = readPatient(id);
        remember("Update patient " + to_string(id));
        state.patients[position] = updated;
        auto waiting=state.serviceQueue.records();
        state.serviceQueue=ServiceQueue{};
        for (auto& record : waiting) {
            if (record.patientID==id) { record.name=updated.name; record.department=updated.department; }
            state.serviceQueue.enqueue(record);
        }
        if (!commitChange()) return;
        ConsoleUI::message("OK", "Patient updated.");
    }

// Prevent dangling references, shift array records left and remove the BST key.
void HospitalSystem::deletePatient() {
        int id = Input::patientID("Patient ID to delete: ");
        int position = patientPosition(id);
        if (position < 0) { ConsoleUI::message("!", "Patient not found."); return; }
        if (state.history.hasPatient(id) || state.serviceQueue.hasPatient(id) || state.emergency.hasPatient(id)) {
            ConsoleUI::message("!", "Remove this patient's medical history and queue entries first.");
            return;
        }
        for (const auto& appointment : state.appointments)
            if (appointment.patientID == id) {
                ConsoleUI::message("!", "Patient has retained appointment records and cannot be deleted.");
                return;
            }
        for (const auto& prescription : state.prescriptions.records())
            if (prescription.patientID==id) { ConsoleUI::message("!", "Patient has retained prescription records and cannot be deleted."); return; }
        for (const auto& invoice : state.billing.records())
            if (invoice.patientID==id) { ConsoleUI::message("!", "Patient has retained invoice records and cannot be deleted."); return; }
        if (Input::number("Confirm deletion? 1. Yes  2. No: ", 1, 2) == 2) return;
        remember("Delete patient " + to_string(id));
        for (int i = position; i < state.count - 1; ++i) state.patients[i] = state.patients[i + 1];
        state.patients[--state.count] = Patient{};
        index.erase(id);
        if (!commitChange()) return;
        ConsoleUI::message("OK", "Patient deleted from array and BST.");
    }

// Check active doctor/patient bookings in the same slot, optionally excluding the edited booking.
bool HospitalSystem::conflict(const Appointment& proposed, int excludedID) const {
        for (const auto& existing : state.appointments)
            if (ScheduleBook::active(existing) && existing.id != excludedID && existing.date == proposed.date && existing.time == proposed.time &&
                (existing.doctorID == proposed.doctorID || existing.patientID == proposed.patientID))
                return true;
        return false;
    }

// Display the manually registered doctor directory.
void HospitalSystem::showDoctors(bool includeShifts) const {
    if (state.doctors.empty()) { ConsoleUI::message("INFO", "No doctors registered."); return; }
    ConsoleUI::banner("ALL DOCTORS");
    bool firstDoctor = true;
    for (const auto& doctor : state.doctors) {
        if (!firstDoctor) ConsoleUI::rule();
        firstDoctor = false;
        ConsoleUI::row(formatDoctorID(doctor.id) + " | " + doctor.name);
        ConsoleUI::field("Department", doctor.department);
        ConsoleUI::field("Consultation fee",doctor.consultationFeeCents ? BillingSystem::money(doctor.consultationFeeCents) : "Not set - configure before completing visits");
        if (!includeShifts) continue;
        vector<DoctorShift> shifts;
        for (const auto& shift : state.shifts)
            if (shift.doctorID == doctor.id) shifts.push_back(shift);
        sort(shifts.begin(), shifts.end(), [](const DoctorShift& a, const DoctorShift& b) {
            auto rank = [](const string& day) {
                for (int i=1;i<=7;++i) if (day==ScheduleBook::dayName(i)) return i;
                return 8;
            };
            if (rank(a.date) != rank(b.date)) return rank(a.date) < rank(b.date);
            if (a.date != b.date) return a.date < b.date;
            return a.start < b.start;
        });
        if (shifts.empty()) ConsoleUI::field("Shifts", "Not scheduled");
        else for (const auto& shift : shifts)
            ConsoleUI::field("Shift", shift.date + "  " + shift.start + " - " + shift.end);
        ConsoleUI::row();
    }
    ConsoleUI::rule();
    ConsoleUI::row("Total doctors: " + to_string(state.doctors.size()));
    ConsoleUI::rule();
}

// Locate a doctor by their unique ID rather than assuming consecutive IDs.
int HospitalSystem::doctorPosition(int id) const {
    for (size_t i = 0; i < state.doctors.size(); ++i)
        if (state.doctors[i].id == id) return static_cast<int>(i);
    return -1;
}

// Register a unique user-entered doctor and save the change as an undoable operation.
void HospitalSystem::addDoctor() {
    if (nextDoctorID > 999999) { ConsoleUI::message("!", "Doctor ID limit reached."); return; }
    int id = nextDoctorID;
    ConsoleUI::message("INFO", "Generated Doctor ID: " + formatDoctorID(id));
    string name = Input::name("Doctor name (0 to cancel): ", true);
    if (name == "0") { ConsoleUI::message("INFO", "Doctor registration cancelled."); return; }
    string department = Input::text("Department (0 to cancel): ", 100);
    if (department == "0") { ConsoleUI::message("INFO", "Doctor registration cancelled."); return; }
    long long fee=0;
    for(;;) {
        if(BillingSystem::parsePrice(Input::text("Consultation fee (RM): "),fee)) break;
        ConsoleUI::message("!","Enter RM 0.01 to RM 1000000.00 with at most two decimals.");
    }
    Doctor doctor{id, name, department, fee};
    remember("Add doctor " + to_string(id));
    state.doctors.push_back(doctor);
    ++nextDoctorID;
    if (!commitChange()) { --nextDoctorID; return; }
    ConsoleUI::message("OK", "Doctor registered. ID: " + formatDoctorID(id));
}

// Validate the whole CSV before changing records; save the batch as one undoable action.
void HospitalSystem::importDoctors() {
    string path = Input::text("Doctor CSV path (e.g. sample-doctors.csv): ");
    if (path.size() >= 2 && path.front() == '"' && path.back() == '"') path = path.substr(1, path.size()-2);
    ifstream file{filesystem::u8path(path)};
    if (!file) { ConsoleUI::message("!", "Cannot open doctor CSV file."); return; }
    auto trim = [](const string& value) {
        auto first = value.find_first_not_of(" \t\r");
        return first == string::npos ? string{} : value.substr(first, value.find_last_not_of(" \t\r")-first+1);
    };
    auto key = [](string value) {
        transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(tolower(c)); });
        return value;
    };
    map<pair<string,string>, int> known;
    for (const auto& doctor : state.doctors) known[{key(trim(doctor.name)),key(trim(doctor.department))}] = doctor.id;
    vector<Doctor> batch;
    vector<DoctorShift> shifts;
    set<int> weeklyDoctors;
    string line;
    size_t lineNumber = 0, skipped = 0;
    try {
        while (getline(file, line)) {
            ++lineNumber;
            if (lineNumber == 1 && line.rfind("\xEF\xBB\xBF",0) == 0) line.erase(0,3);
            if (trim(line).empty()) continue;
            if (line.size() > 1000) throw runtime_error("Line is too long");
            vector<string> fields;
            size_t pos = 0;
            for (;;) {
                while (pos < line.size() && line[pos] == ' ') ++pos;
                string value;
                if (pos < line.size() && line[pos] == '"') {
                    ++pos;
                    bool closed = false;
                    while (pos < line.size()) {
                        char c = line[pos++];
                        if (c != '"') value += c;
                        else if (pos < line.size() && line[pos] == '"') { value += '"'; ++pos; }
                        else { closed = true; break; }
                    }
                    if (!closed) throw runtime_error("Unclosed CSV quote");
                    while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\r')) ++pos;
                    if (pos < line.size() && line[pos] != ',') throw runtime_error("Unexpected text after quoted field");
                } else {
                    while (pos < line.size() && line[pos] != ',') {
                        if (line[pos] == '"') throw runtime_error("Unexpected CSV quote");
                        value += line[pos++];
                    }
                }
                fields.push_back(trim(value));
                if (pos == line.size()) break;
                ++pos;
            }
            if (fields.size() != 2 && fields.size() != 5) throw runtime_error("Expected name,department or name,department,date,start,end columns");
            if (batch.empty() && known.size() == state.doctors.size() && key(fields[0]) == "name" && key(fields[1]) == "department") continue;
            bool letter = false, digit = false;
            for (unsigned char c : fields[0]) {
                letter |= (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c >= 128;
                digit |= c >= '0' && c <= '9';
            }
            if (!Input::validText(fields[0],100) || !letter || digit || !Input::validText(fields[1],100))
                throw runtime_error("Invalid doctor name or department");
            auto identity = make_pair(key(fields[0]),key(fields[1]));
            int doctorID;
            auto found = known.find(identity);
            if (found != known.end()) { doctorID = found->second; ++skipped; }
            else {
                if (state.doctors.size()+batch.size() >= 100000 || nextDoctorID+static_cast<int>(batch.size()) > 999999)
                    throw runtime_error("Doctor capacity or ID limit reached");
                doctorID = nextDoctorID+static_cast<int>(batch.size());
                batch.push_back({doctorID,fields[0],fields[1]});
                known[identity] = doctorID;
            }
            if (fields.size() == 5) {
                bool weekly = !Input::validDate(fields[2]);
                if (weekly) weeklyDoctors.insert(doctorID);
                int start = ScheduleBook::slot(fields[3]), end = ScheduleBook::slot(fields[4],true);
                if (!ScheduleBook::validShiftDay(fields[2]) || start < 0 || end <= start)
                    throw runtime_error("Invalid shift date or times");
                bool duplicate = false;
                auto check = [&](const DoctorShift& shift) {
                    if (shift.doctorID != doctorID || shift.date != fields[2]) return;
                    if (shift.start == fields[3] && shift.end == fields[4]) { duplicate = true; return; }
                    if (start < ScheduleBook::slot(shift.end,true) && end > ScheduleBook::slot(shift.start))
                        throw runtime_error("Shift overlaps an existing shift");
                };
                if (!weekly) for (const auto& shift : state.shifts) check(shift);
                for (const auto& shift : shifts) check(shift);
                if (!duplicate) {
                    if (state.shifts.size()+shifts.size() >= 100000) throw runtime_error("Shift capacity reached");
                    shifts.push_back({doctorID,fields[2],fields[3],fields[4]});
                }
            }
        }
        if (file.bad()) throw runtime_error("Cannot finish reading file");
    } catch (const exception& error) {
        ConsoleUI::message("!", "CSV line " + to_string(lineNumber) + ": " + error.what() + ". Nothing imported.");
        return;
    }
    if (batch.empty() && shifts.empty()) { ConsoleUI::message("INFO", "No new doctors or shifts to import (empty file or duplicates)."); return; }
    vector<DoctorShift> updatedShifts;
    size_t replaced = 0;
    for (const auto& shift : state.shifts) {
        if (weeklyDoctors.count(shift.doctorID)) ++replaced;
        else updatedShifts.push_back(shift);
    }
    updatedShifts.insert(updatedShifts.end(),shifts.begin(),shifts.end());
    for (const auto& appointment : state.appointments) {
        if (weeklyDoctors.count(appointment.doctorID) && ScheduleBook::active(appointment) &&
            !ScheduleBook::working(updatedShifts,appointment.doctorID,appointment.date,appointment.time)) {
            ConsoleUI::message("!", "Replacement would leave active appointment #" + to_string(appointment.id) + " outside working hours. Reschedule or cancel it first. Nothing imported.");
            return;
        }
    }
    remember("Import " + to_string(batch.size()) + " doctors and " + to_string(shifts.size()) + " shifts from CSV");
    state.doctors.insert(state.doctors.end(),batch.begin(),batch.end());
    state.shifts = move(updatedShifts);
    nextDoctorID += static_cast<int>(batch.size());
    if (!commitChange()) { nextDoctorID -= static_cast<int>(batch.size()); return; }
    ConsoleUI::message("OK", "Imported " + to_string(batch.size()) + " doctors; skipped " + to_string(skipped) + " duplicate doctor rows. Added " + to_string(shifts.size()) + " shifts.");
    ConsoleUI::message("INFO", "Replaced " + to_string(replaced) + " old shifts for imported weekly schedules.");
    showDoctors();
}

// Calculate free slots from entered shifts and currently active appointments.
void HospitalSystem::showAvailableSlots() const {
    if (state.doctors.empty()) {
        ConsoleUI::panel([&](ostream& out) { out << "Register a doctor first in Doctor management.\n"; });
        return;
    }
    showDoctors();
    int id = Input::doctorID("Doctor ID: ");
    int position = doctorPosition(id);
    if (position < 0) { ConsoleUI::message("!", "Doctor not found."); return; }
    string date = Input::date();
    printAvailableSlots(id, date);
}

// Share the four-column grid between availability queries and booking.
bool HospitalSystem::printAvailableSlots(int id, const string& date, int patientID) const {
    int position = doctorPosition(id);
    array<bool, 48> working{}, occupied{};
    for (const auto& shift : state.shifts) {
        if (shift.doctorID != id || !ScheduleBook::matches(shift.date,date)) continue;
        for (int slot=ScheduleBook::slot(shift.start); slot<ScheduleBook::slot(shift.end,true); ++slot)
            working[slot]=true;
    }
    for (const auto& appointment : state.appointments) {
        if ((appointment.doctorID==id || (patientID && appointment.patientID==patientID)) && appointment.date==date && ScheduleBook::active(appointment))
            occupied[ScheduleBook::slot(appointment.time)]=true;
    }
    vector<string> available;
    int total=0;
    for (int slot=0;slot<48;++slot) {
        if (!working[slot]) continue;
        ++total;
        if (!occupied[slot]) available.push_back(ScheduleBook::time(slot));
    }
    ConsoleUI::RecordBox frame;
    ConsoleUI::row("AVAILABLE APPOINTMENT SLOTS", true);
    ConsoleUI::row();
    ConsoleUI::field("Doctor", formatDoctorID(id) + " | " + state.doctors[position].name);
    ConsoleUI::field("Date", date + " (" + ScheduleBook::dayName(ScheduleBook::weekday(date)) + ")");
    ConsoleUI::row();
    if (!total) ConsoleUI::row("No doctor shift on this date.");
    else if (available.empty()) ConsoleUI::row("No available slots.");
    else {
        // Four equal-width cells stay inside the shared content area.
        ConsoleUI::row(string(71, '-'));
        for (size_t i=0;i<available.size();i+=4) {
            string cells;
            for (size_t column=0;column<4;++column) {
                string value=i+column<available.size() ? available[i+column] : "";
                cells += string(6, ' ') + value + string(17-6-value.size(), ' ');
                if (column<3) cells += '|';
            }
            ConsoleUI::row(cells);
        }
        ConsoleUI::row(string(71, '-'));
    }
    ConsoleUI::row();
    ConsoleUI::row("Available slots: " + to_string(available.size()) + "/" + to_string(total));
    ConsoleUI::row(patientID ? "Only slots free for both this doctor and patient are shown." : "Booking also checks the patient's other appointments.");
    return !available.empty();
}

// Filter by doctor and date, then display matching bookings in time order.
void HospitalSystem::showDoctorAppointments(int selectedDoctorID) const {
    if (state.doctors.empty()) { ConsoleUI::message("OK", "No doctors registered."); return; }
    if(!selectedDoctorID) showDoctors();
    int id=selectedDoctorID ? selectedDoctorID : Input::doctorID("Doctor ID: ");
    int position = doctorPosition(id);
    if (position < 0) { ConsoleUI::message("!", "Doctor not found."); return; }
    string date = Input::date();
    vector<Appointment> appointments;
    for (const auto& appointment : state.appointments)
        if (appointment.doctorID == id && appointment.date == date)
            appointments.push_back(appointment);
    sort(appointments.begin(), appointments.end(), [](const Appointment& a, const Appointment& b) {
        return a.time < b.time;
    });
    ConsoleUI::panel([&](ostream& out) { out << "Appointments for " << state.doctors[position].name << " on " << date << ":\n"; });
    if (appointments.empty()) ConsoleUI::message("INFO", "No appointments for this doctor on this date.");
    for (const auto& appointment : appointments) printAppointment(appointment);

    ConsoleUI::panel([&](ostream& out) { out << "Total appointments: " << appointments.size() << '\n'; });
    if(selectedDoctorID) {
        printAvailableSlots(id,date);
        for(;;) {
            string time=Input::text("Time to check (HH:MM, 0 to return to main menu): ");
            if(!Input::validTime(time)) {ConsoleUI::message("!","Select a valid 30-minute time.");continue;}
            bool working=ScheduleBook::working(state.shifts,id,date,time),booked=false;
            for(const auto& visit:appointments) if(ScheduleBook::active(visit) && visit.time==time) booked=true;
            ConsoleUI::message("INFO",date+" "+time+": "+(!working ? "Not working" : booked ? "Already booked" : "Available"));
        }
    }
}

// Require an existing patient, doctor and shift; reject slot conflicts before booking.
void HospitalSystem::addAppointment() {
        if (nextAppointmentID > 999999) { ConsoleUI::message("!", "Appointment ID limit reached."); return; }
        if (state.doctors.empty()) { ConsoleUI::panel([&](ostream& out) { out << "Register a doctor first in Doctor management.\n"; }); return; }
        int patientID = Input::patientID("Patient ID: ");
        if (!index.contains(patientID)) { ConsoleUI::message("!", "Register the patient first."); return; }
        showDoctors();
        int doctorID = Input::doctorID("Doctor ID: ");
        if (doctorPosition(doctorID) < 0) { ConsoleUI::message("!", "Doctor not found."); return; }
        string date = Input::date();
        if (!printAvailableSlots(doctorID, date, patientID)) return;
        string time = Input::time();
        Appointment appointment{nextAppointmentID, patientID, doctorID, date, time, ""};
        if (!ScheduleBook::working(state.shifts,doctorID,date,time)) { ConsoleUI::message("!", "Doctor is not working at this time. Add a shift first."); return; }
        if (conflict(appointment)) { ConsoleUI::message("!", "Booking rejected: doctor or patient already booked in this slot."); return; }
        appointment.reason = Input::text("Appointment reason: ");
        remember("Book appointment " + to_string(appointment.id));
        state.appointments.push_back(appointment);
        ++nextAppointmentID;
        if (!commitChange()) return;
        ConsoleUI::panel([&](ostream& out) { out << "Appointment booked. ID: " << formatID("A",appointment.id) << '\n'; });
    }

// One appointment ID per card; each field gets its own line.
void HospitalSystem::printAppointment(const Appointment& appointment) const {
    ConsoleUI::RecordBox frame;
    ConsoleUI::row("Appointment " + formatID("A",appointment.id));
    ConsoleUI::row();
    ConsoleUI::field("Date", appointment.date);
    ConsoleUI::field("Time", appointment.time);
    ConsoleUI::field("Patient", formatPatientID(appointment.patientID) + " (" + state.patients[patientPosition(appointment.patientID)].name + ")");
    const auto& doctor = state.doctors[doctorPosition(appointment.doctorID)];
    ConsoleUI::field("Doctor", formatDoctorID(doctor.id) + " (" + doctor.name + ")");
    ConsoleUI::field("Department", doctor.department);
    ConsoleUI::field("Reason", appointment.reason);
    ConsoleUI::field("Status", appointment.status);
}

// Display current and retained appointments chronologically, with their statuses.
void HospitalSystem::showAppointments(int patientID) const {
        vector<Appointment> sorted;
        for (const auto& appointment : state.appointments)
            if (patientID == 0 || appointment.patientID == patientID) sorted.push_back(appointment);
        sort(sorted.begin(), sorted.end(), [](const Appointment& a, const Appointment& b) {
            if (a.date != b.date) return a.date < b.date;
            if (a.time != b.time) return a.time < b.time;
            return a.id < b.id;
        });
        if (sorted.empty()) { ConsoleUI::message("INFO", "No appointments found."); return; }
        for (const auto& appointment : sorted) printAppointment(appointment);
    }

// Cancel while retaining history, or reschedule a Scheduled appointment within a shift.
void HospitalSystem::changeAppointment(bool cancel) {
        int id = Input::entityID("Appointment ID: ", "A");
        auto found = find_if(state.appointments.begin(), state.appointments.end(),
                             [id](const Appointment& appointment) { return appointment.id == id; });
        if (found == state.appointments.end()) { ConsoleUI::message("!", "Appointment not found."); return; }
        if (found->status != "Scheduled") { ConsoleUI::message("!", "Only scheduled appointments can be cancelled or rescheduled."); return; }
        if (cancel) {
            if (Input::number("Confirm cancellation? 1. Yes  2. No: ", 1, 2) == 2) return;
            remember("Cancel appointment " + to_string(id));
            found->status="Cancelled";
            if (commitChange()) ConsoleUI::message("OK", "Appointment cancelled; record retained.");
        } else {
            Appointment updated = *found;
            updated.date = Input::date();
            updated.time = Input::time();
            if (!ScheduleBook::working(state.shifts,updated.doctorID,updated.date,updated.time)) { ConsoleUI::panel([&](ostream& out) { out << "Doctor is not working at this time.\n"; }); return; }
            if (conflict(updated, id)) { ConsoleUI::message("!", "Rescheduling rejected: doctor or patient has a conflicting appointment."); return; }
            remember("Reschedule appointment " + to_string(id));
            *found = updated;
            if (commitChange()) ConsoleUI::panel([&](ostream& out) { out << "Appointment rescheduled.\n"; });
        }
    }

// Dispatch array/BST operations until the user returns to the main menu.
void HospitalSystem::patientMenu() {
        for (;;) {
            try {
            ConsoleUI::menu("PATIENT RECORDS - TASK 1 ARRAY / TASK 5 BST", {"1. Add patient", "2. Display patient array (Array)", "3. Update patient", "4. Delete patient", "5. Search patient", "0. Back to main menu"});
            int choice = Input::number("Choose: ", 0, 5);
            if (!choice) throw MainMenuRequested{};
            if (choice == 2) {
                if (!state.count) ConsoleUI::message("INFO", "No patient records.");
                for (int i = 0; i < state.count; ++i) printPatient(state.patients[i]);
            } else if (choice == 1) addPatient();
            else if (choice == 3) editPatient();
            else if (choice == 4) deletePatient();
            else { for(;;) { searchDemo(); } }
                    } catch (const InputCancelled&) {
                ConsoleUI::message("INFO", "Operation cancelled. Nothing saved.");
            }
        }
    }

// Appointment tasks and doctor tasks have separate, focused menus.
void HospitalSystem::appointmentMenu() {
    for (;;) {
        try {
            ConsoleUI::menu("APPOINTMENT MANAGEMENT", {"1. Book appointment", "2. All appointments (chronological)", "3. Reschedule", "4. Appointment status / check-in", "5. Cancel appointment", "0. Back to main menu"});
            int choice = Input::number("Choose: ",0,5);
            if (!choice) throw MainMenuRequested{};
            if (choice == 1) addAppointment();
            else if (choice == 2) showAppointments();
            else if (choice == 3) changeAppointment(false);
            else if (choice == 4) updateAppointmentStatus();
            else changeAppointment(true);
        } catch (const InputCancelled&) { ConsoleUI::message("INFO", "Operation cancelled. Nothing saved."); }
    }
}

void HospitalSystem::doctorMenu() {
    for(;;) {
        try {
            ConsoleUI::menu("DOCTOR MANAGEMENT",{"1. Register doctor","2. View doctors","3. Update consultation fee","4. Manage shifts","0. Back to main menu"});
            int choice=Input::number("Choose: ",0,4);
            if (!choice) throw MainMenuRequested{};
            if(choice==1) addDoctor();
            else if(choice==2) doctorView();
            else if(choice==3) updateDoctorFee();else scheduleMenu();
        } catch(const InputCancelled&) {ConsoleUI::message("INFO","Operation cancelled. Nothing saved.");}
    }
}
void HospitalSystem::doctorView() {
    showDoctors(false);if(state.doctors.empty()) return;
    int id=Input::doctorID("Doctor ID to view (0 to return to main menu): ");int position=doctorPosition(id);
    if(position<0) {ConsoleUI::message("!","Doctor not found.");return;}
    {
        ConsoleUI::RecordBox frame;
        const auto& doctor=state.doctors[position];
        ConsoleUI::row(formatDoctorID(id)+" | "+doctor.name);
        ConsoleUI::field("Department",doctor.department);
        ConsoleUI::field("Consultation fee",doctor.consultationFeeCents ? BillingSystem::money(doctor.consultationFeeCents) : "Not set");
        for(int day=1;day<=7;++day) {
            string hours;
            for(const auto& shift:state.shifts) if(shift.doctorID==id && shift.date==ScheduleBook::dayName(day)) {if(!hours.empty()) hours+=" / ";hours+=shift.start+" - "+shift.end;}
            ConsoleUI::field(ScheduleBook::dayName(day),hours.empty() ? "Off" : hours);
        }
        for(const auto& shift:state.shifts) if(shift.doctorID==id && Input::validDate(shift.date))
            ConsoleUI::field("Dated shift",shift.date+" "+shift.start+" - "+shift.end);
    }
    // Continue directly to date/time availability for this selected doctor.
    try { showDoctorAppointments(id); }
    catch(const InputCancelled&) { /* Cancel an input and return to Doctor Management. */ }
}

// Shared complete-record form for Add and Update; mutation happens only after validation.
bool HospitalSystem::readMedicalDetails(string& details) {
    details.clear();
    const string labels[]={"Consultation","Diagnosis","Treatment","Prescription notes"};
    bool hasContent=false;
    for(const auto& label:labels) {
        string value=Input::text(label+": ");
        string normalized=value;
        transform(normalized.begin(),normalized.end(),normalized.begin(),[](unsigned char c){return static_cast<char>(tolower(c));});
        if(normalized!="n/a") hasContent=true;
        if(!details.empty()) details+='\n';
        details+=label+": "+value;
    }
    if(!hasContent) ConsoleUI::message("!","At least one visit section must contain details. Nothing saved.");
    return hasContent;
}

// Add, remove or traverse chronological linked-list medical records by patient.
void HospitalSystem::visitRecordsMenu() {
        for (;;) {
            try {
            ConsoleUI::menu("VISIT MEDICAL RECORDS - TASK 2 LINKED LIST", {"1. Add record", "2. View records", "3. Update record", "4. Delete record", "0. Back to main menu"});
            int choice = Input::number("Choose: ",0,4);
            if (!choice) throw MainMenuRequested{};
            if (choice == 1) {
                if (nextRecordID > 999999) { ConsoleUI::message("!", "Medical record ID limit reached."); continue; }
                int id = Input::patientID("Patient ID: ");
                if (!index.contains(id)) { ConsoleUI::message("!", "Patient not found."); continue; }
                vector<Appointment> visits;
                for (const auto& appointment : state.appointments)
                    if (appointment.patientID == id && appointment.status == "Checked-in") visits.push_back(appointment);
                if (visits.empty()) {
                    ConsoleUI::message("!", "No checked-in appointment for this patient. Check in an appointment before adding medical records.");
                    continue;
                }
                sort(visits.begin(),visits.end(),[](const Appointment& a,const Appointment& b) {
                    if(a.date!=b.date) return a.date<b.date;
                    if(a.time!=b.time) return a.time<b.time;
                    return a.id<b.id;
                });
                ConsoleUI::banner("SELECT CHECKED-IN VISIT");
                for (const auto& appointment : visits) printAppointment(appointment);
                Appointment visit=visits.front();
                for (;;) {
                    int appointmentID=Input::entityID("Appointment ID (0 to cancel): ", "A");
                    auto selected=find_if(visits.begin(),visits.end(),[&](const Appointment& appointment){return appointment.id==appointmentID;});
                    if(selected!=visits.end()) {visit=*selected;break;}
                    ConsoleUI::message("!", "Select one of this patient's checked-in appointments shown above.");
                }
                ConsoleUI::banner("SELECTED VISIT");
                printAppointment(visit);
                MedicalRecord record{nextRecordID,id,visit.date,"",""};
                record.appointmentID=visit.id;
                record.type = "Complete visit";
                ConsoleUI::message("INFO", "Complete one visit below. Enter N/A for a section that does not apply.");
                if (!readMedicalDetails(record.details)) continue;
                remember("Add medical record " + to_string(record.id));
                state.history.add(record);
                ++nextRecordID;
                if (!commitChange()) return;
                ConsoleUI::panel([&](ostream& out) { out << "Medical record added. ID: " << formatID("R",record.id) << '\n'; });
                printMedicalHistory(state.history.records(id),id);
            } else if (choice == 3 || choice == 4) {
                int recordID=Input::entityID("Medical record ID: ", "R");
                auto records=state.history.records();
                auto found=find_if(records.begin(),records.end(),[&](const MedicalRecord& record){return record.id==recordID;});
                if(found==records.end()) {ConsoleUI::message("!","Record not found.");continue;}
                MedicalRecord updated=*found;
                int patientID=updated.patientID;
                printMedicalHistory({updated},patientID);
                if(choice==3) {
                    ConsoleUI::message("INFO","Enter the corrected record details. The patient and linked visit stay the same; use N/A for unused sections.");
                    if(!readMedicalDetails(updated.details)) continue;
                    updated.type="Complete visit";
                    remember("Update medical record " + to_string(recordID));
                    state.history.remove(recordID);
                    state.history.add(updated);
                } else {
                    remember("Delete medical record " + to_string(recordID));
                    state.history.remove(recordID);
                }
                if(!commitChange()) return;
                ConsoleUI::message("OK",choice==3 ? "Medical record updated." : "Medical record deleted.");
                auto remaining=state.history.records(patientID);
                if(remaining.empty()) ConsoleUI::message("INFO","No medical history for this patient.");
                else printMedicalHistory(remaining,patientID);
            } else {
                int id = Input::patientID("Patient ID: ");
                if (!index.contains(id)) { ConsoleUI::message("!", "Patient not found."); continue; }
                auto records = state.history.records(id);
                if (records.empty()) ConsoleUI::message("INFO", "No medical history for this patient.");
                printMedicalHistory(records, id);
            }
                    } catch (const InputCancelled&) {
                ConsoleUI::message("INFO", "Operation cancelled. Nothing saved.");
            }
        }
    }

// One queue entry point for normal and emergency services.
void HospitalSystem::queueMenu() {
    for (;;) {
        try {
            ConsoleUI::menu("PATIENT SERVICE QUEUES - TASK 4 FIFO / TASK 9 PRIORITY", {"1. Normal (FIFO)", "2. Emergency (priority; ties: FIFO)", "0. Back to main menu"});
            int choice=Input::number("Choose: ",0,2);
            if (!choice) throw MainMenuRequested{};
            if (choice==1) normalQueueMenu();
            else emergencyMenu();
        } catch (const InputCancelled&) { ConsoleUI::message("INFO", "Operation cancelled. Nothing saved."); }
    }
}

// Demonstrate FIFO: enqueue at the rear, serve the front and display waiting patients.
void HospitalSystem::normalQueueMenu() {
        for (;;) {
            try {
            ConsoleUI::menu("SERVICE QUEUE - TASK 4 QUEUE / FIFO", {"1. Enqueue (insert at rear)", "2. Display waiting patients (FIFO order)", "3. Front patient (peek)", "4. Serve patients (one / all, FIFO)", "0. Back to main menu"});
            int choice = Input::number("Choose: ",0,4);
            if (!choice) throw MainMenuRequested{};
            if (choice == 1) {
                int id = Input::patientID("Patient ID: ");
                int position = patientPosition(id);
                if (position < 0) { ConsoleUI::message("!", "Patient not found."); continue; }
                if (showWaitingConflict(id)) continue;
                if (state.serviceQueue.size() == 100) { ConsoleUI::message("!", "Queue is full."); continue; }
                const auto& p = state.patients[position];
                int service = Input::number("1. Registration 2. Consultation 3. Laboratory 4. Pharmacy 5. General treatment: ",1,5);
                const string services[] = {"Registration","Consultation","Laboratory","Pharmacy","General treatment"};
                remember("Enqueue patient " + to_string(id));
                state.serviceQueue.enqueue({id,p.name,p.department,services[service-1],"Waiting"});
                if (!commitChange()) return;
        ConsoleUI::panel([&](ostream& out) { out << "Patient enqueued. Service order is FIFO.\n"; });
            } else if (choice == 4) {
                if (!state.serviceQueue.front()) { ConsoleUI::panel([&](ostream& out) { out << "Queue is empty.\n"; }); continue; }
                ConsoleUI::menu("SERVE PATIENTS - TASK 4 QUEUE / FIFO", {"1. Serve one patient (front)", "2. Serve all waiting patients (FIFO order)", "0. Cancel"});
                int mode = Input::number("Choose: ",0,2);
                if (!mode) continue;
                int count = mode == 1 ? 1 : state.serviceQueue.size();
                remember(mode == 1 ? "Serve front patient " + to_string(state.serviceQueue.front()->patientID)
                                   : "Serve all " + to_string(count) + " waiting patients (FIFO)");
                vector<QueueRecord> served;
                for (int i=0;i<count;++i) {
                    QueueRecord record;
                    state.serviceQueue.dequeue(record);
                    served.push_back(record);
                }
                if (!commitChange()) return;
                if (mode == 2) ConsoleUI::message("OK", "Served " + to_string(count) + " patients in FIFO order.");
                for (const auto& record : served) {
                    ConsoleUI::RecordBox frame;
                    ConsoleUI::panel([&](ostream& out) { out << "Serving patient " << formatPatientID(record.patientID) << " | " << record.name << '\n'; });
                    ConsoleUI::field("Department", record.department);
                    ConsoleUI::field("Service", record.service);
                }
            } else {
                vector<QueueRecord> records;
                if (choice == 3) {
                    if (auto* front = state.serviceQueue.front()) records.push_back(*front);
                } else records = state.serviceQueue.records();
                ConsoleUI::banner(choice == 3 ? "FRONT PATIENT (PEEK)" : "WAITING PATIENTS (FIFO ORDER)");
                if (records.empty()) ConsoleUI::message("INFO", "Queue is empty.");
                for (const auto& r : records) {
                    ConsoleUI::RecordBox frame;
                    ConsoleUI::row(formatPatientID(r.patientID) + " | " + r.name);
                    ConsoleUI::field("Queue", "Normal (FIFO)");
                    ConsoleUI::field("Department", r.department);
                    ConsoleUI::field("Service", r.service);
                    ConsoleUI::field("Status", r.status);
                }
            }
                    } catch (const InputCancelled&) {
                ConsoleUI::message("INFO", "Operation cancelled. Nothing saved.");
            }
        }
    }

// Run DFS and BFS from the selected department using the prescribed directed graph.
void HospitalSystem::graphMenu() {
    ConsoleUI::banner("DEPARTMENT REFERRALS - TASK 6 GRAPH / DFS / BFS");
    ConsoleUI::panel([&](ostream& out){graph.display(out);});
    int start=Input::number("Start department (1=A, 2=B, ... 6=F): ",1,6)-1;
    auto traversal=[](const vector<int>& order) {
        string result;
        for(int vertex:order) {if(!result.empty()) result+=" -> ";result+=char('A'+vertex);}
        return result;
    };
    ConsoleUI::RecordBox frame;
    ConsoleUI::row("TRAVERSAL RESULTS",true);
    ConsoleUI::divider();
    ConsoleUI::field("Start",string(1,char('A'+start)) + " | " + ReferralGraph::name(start));
    ConsoleUI::table({"Metric","DFS","BFS"},{
        {"Traversal order",traversal(graph.dfs(start)),traversal(graph.bfs(start))},
        {"Method","Explore each branch first","Explore each level first"},
        {"Uses","Recursion (call stack)","FIFO queue"},
        {"Time complexity","O(V + E)","O(V + E)"}
    });
    ConsoleUI::row("Neither is always better: DFS suits branch exploration; BFS finds fewest-edge paths.");
    ConsoleUI::row("Traversal lists reachable departments, not a single route.");
}

// Copy only manually entered patient records so demonstrations do not change stored data.
vector<Patient> HospitalSystem::algorithmData() const {
    return {state.patients.begin(), state.patients.begin() + state.count};
}

// Compare linear/binary searches for the same ID; binary search uses an ID-sorted copy.
void HospitalSystem::searchDemo() {
    auto data=algorithmData();
    ConsoleUI::menu("SEARCH PATIENT - TASK 5 BST / TASK 7 LINEAR / BINARY",{"1. BST search","2. Linear search","3. Binary search","4. Compare Linear / Binary","5. Benchmark Linear / Binary","0. Back to main menu"});
    int choice=Input::number("Choose: ",0,5);
    if (!choice) throw MainMenuRequested{};
    if(choice==5) {
        ConsoleUI::RecordBox frame;
        ConsoleUI::row("SEARCH BENCHMARK - TASK 7 LINEAR / BINARY");
        ConsoleUI::row("Target: Last ID in ascending data.");
        vector<vector<string>> rows;
        vector<string> conclusions;
        for(int n : {1000, 5000, 10000}) {
            vector<Patient> sample; sample.reserve(n);
            for(int i=1;i<=n;++i) { Patient p; p.id=i; p.name="Benchmark Patient"; sample.push_back(p); }
            auto linear=Algorithms::linearSearch(sample,n);
            auto binary=Algorithms::binarySearch(sample,n);
            rows.push_back({to_string(n),to_string(linear.comparisons),to_string(binary.comparisons)});
            conclusions.push_back("Dataset "+to_string(n)+" - "+lowerResult("Linear",linear.comparisons,"Binary",binary.comparisons,"ID comparisons"));
        }
        ConsoleUI::table({"Dataset","Linear comparisons","Binary comparisons"},rows);
        for(const auto& result:conclusions) ConsoleUI::row(result);
        ConsoleUI::row("Generated test records only; hospital data is unchanged.");
        ConsoleUI::row("Counts each executed ID == or < test; excludes loop conditions.");
        ConsoleUI::row("Linear: O(n). Binary: O(log n). Input is already sorted.");
        return;
    }
    if(data.empty()) {ConsoleUI::message("INFO","No patients. Add patient records first.");return;}
    int id=Input::patientID("Patient ID to search: ");bool found=false;
    {
        ConsoleUI::RecordBox frame;
        ConsoleUI::row("SEARCH RESULT - "+formatPatientID(id));
        if(choice==1) {
            found=index.contains(id);
            ConsoleUI::row(string("BST Search: ")+(found ? "Found" : "Not found"));
        }
        if(choice==4) {
            auto sorted=data; Algorithms::mergeSort(sorted,false,false);
            auto linear=Algorithms::linearSearch(data,id);
            auto binary=Algorithms::binarySearch(sorted,id);
            found=linear.position>=0;
            ConsoleUI::table({"Metric","Linear","Binary"},{
                {"Result",found ? "Found" : "Not found",binary.position>=0 ? "Found" : "Not found"},
                {"ID comparisons",to_string(linear.comparisons),to_string(binary.comparisons)},
                {"Complexity","O(n)","O(log n)"}
            });
            ConsoleUI::row(lowerResult("Linear",linear.comparisons,"Binary",binary.comparisons,"ID comparisons"));
            ConsoleUI::row("Binary search uses an ID-sorted copy; sorting cost is excluded.");
        }
        if(choice==2) {
            auto result=Algorithms::linearSearch(data,id);
            found=result.position>=0;
            ConsoleUI::row(string("Linear Search: ")+(found ? "Found" : "Not found")+" | comparisons: "+to_string(result.comparisons));
        }
        if(choice==3) {
            auto sorted=data;Algorithms::mergeSort(sorted,false,false);
            auto result=Algorithms::binarySearch(sorted,id);found=result.position>=0;
            ConsoleUI::row(string("Binary Search (ID ascending): ")+(found ? "Found" : "Not found")+" | comparisons: "+to_string(result.comparisons));
            ConsoleUI::row("Binary search uses an ID-sorted copy; sorting cost is excluded.");
        }
        if(found) printPatient(state.patients[patientPosition(id)]);
        if(choice!=1) {
            if(choice==2) ConsoleUI::row("Linear: O(n).");
            if(choice==3) ConsoleUI::row("Binary: O(log n).");
            ConsoleUI::row("Comparisons count executed ID == and < tests, not loop conditions.");
        }
    }
}

// Compare merge/bubble sorts on identical copies, with key/order selection and timing.
void HospitalSystem::sortingDemo() {
    ConsoleUI::menu("SORT & ORDERED DISPLAY - TASK 5 BST / TASK 8 MERGE / BUBBLE", {"1. Patient ID ascending - BST in-order", "2. Merge sort", "3. Bubble sort", "4. Compare Merge / Bubble", "5. Benchmark Merge / Bubble", "0. Back to main menu"});
    int choice=Input::number("Choose: ",0,5);
    if (!choice) throw MainMenuRequested{};
    auto data=algorithmData();
    if(choice==1) {
        if(data.empty()) {ConsoleUI::message("INFO","No patients. Add patient records first.");return;}
        ConsoleUI::message("INFO","BST in-order traversal: Left -> Root -> Right (ID ascending).");
        for(int id:index.sortedIDs()) printPatient(state.patients[patientPosition(id)]);
        return;
    }
    if(choice==5) {
        ConsoleUI::RecordBox frame;
        ConsoleUI::row("SORT BENCHMARK - TASK 8 MERGE / BUBBLE");
        ConsoleUI::row("Input: Descending IDs. Output: Ascending IDs.");
        vector<vector<string>> rows;
        vector<string> conclusions;
        auto timeText=[](double value) {ostringstream out;out<<fixed<<setprecision(1)<<value;return out.str();};
        for(int n : {1000, 5000, 10000}) {
            vector<Patient> base; base.reserve(n);
            for(int i=n;i>=1;--i) { Patient p; p.id=i; p.age=18+(i%70); p.name="Benchmark Patient"; base.push_back(p); }
            auto mergeData=base, bubbleData=base;
            auto merge=Algorithms::mergeSort(mergeData,false,false);
            auto bubble=Algorithms::bubbleSort(bubbleData,false,false);
            rows.push_back({to_string(n),to_string(merge.comparisons),timeText(merge.microseconds),
                            to_string(bubble.comparisons),timeText(bubble.microseconds)});
            conclusions.push_back("Dataset "+to_string(n)+" - "+lowerResult("Merge Sort",merge.comparisons,"Bubble Sort",bubble.comparisons,"Comparisons"));
            conclusions.push_back("Dataset "+to_string(n)+" - "+lowerResult("Merge Sort",merge.microseconds,"Bubble Sort",bubble.microseconds,"Measured time"));
        }
        ConsoleUI::row("Generated test records only; hospital data is unchanged.");
        ConsoleUI::table({"Dataset","Merge cmp","Merge us","Bubble cmp","Bubble us"},rows);
        for(const auto& result:conclusions) ConsoleUI::row(result);
        ConsoleUI::row("Both use identical copies; timings exclude copying and printing.");
        ConsoleUI::row("Timings vary between runs.");
        return;
    }
    if(data.empty()) {ConsoleUI::message("INFO","No patients. Add patient records first.");return;}
    bool age=Input::number("Sort key: 1. Patient ID 2. Age: ",1,2)==2;
    bool descending=Input::number("Order: 1. Ascending 2. Descending: ",1,2)==2;
    if(choice==4) {
        auto mergeData=data, bubbleData=data;
        auto merge=Algorithms::mergeSort(mergeData,age,descending);
        auto bubble=Algorithms::bubbleSort(bubbleData,age,descending);
        ConsoleUI::message("INFO","Unsorted records (original order):");
        for(const auto& patient:data) printPatient(patient);
        ConsoleUI::message("INFO","Sorted records - Merge Sort:");
        for(const auto& patient:mergeData) printPatient(patient);
        ConsoleUI::message("INFO","Sorted records - Bubble Sort:");
        for(const auto& patient:bubbleData) printPatient(patient);
        auto timeText=[](double value) {ostringstream out;out<<fixed<<setprecision(3)<<value;return out.str();};
        ConsoleUI::RecordBox frame;
        ConsoleUI::row("SORT COMPARISON - MERGE / BUBBLE");
        ConsoleUI::row("Records: "+to_string(data.size())+" | Key: "+(age ? "Age" : "Patient ID")+
                       " | Order: "+(descending ? "Descending" : "Ascending"));
        ConsoleUI::table({"Metric","Merge","Bubble"},{
            {"Key comparisons",to_string(merge.comparisons),to_string(bubble.comparisons)},
            {"Time (us)",timeText(merge.microseconds),timeText(bubble.microseconds)},
            {"Average / worst","O(n log n)","O(n^2)"}
        });
        ConsoleUI::row(lowerResult("Merge Sort",merge.comparisons,"Bubble Sort",bubble.comparisons,"Key comparisons"));
        ConsoleUI::row(lowerResult("Merge Sort",merge.microseconds,"Bubble Sort",bubble.microseconds,"Measured time"));
        ConsoleUI::row("Both use identical copies. Timings exclude printing and copying.");
        return;
    }
    ConsoleUI::message("INFO","Unsorted records (original order):");
    for(const auto& p:data) printPatient(p);
    auto show=[&](bool merge) {
        auto sorted=data;
        auto metrics=merge ? Algorithms::mergeSort(sorted,age,descending) : Algorithms::bubbleSort(sorted,age,descending);
        ConsoleUI::message("INFO",merge ? "Merge Sort records:" : "Bubble Sort records:");
        for(const auto& p:sorted) printPatient(p);
        ConsoleUI::panel([&](ostream& out) {
            out<<fixed<<setprecision(3)<<(merge ? "Merge Sort" : "Bubble Sort")<<" | comparisons: "<<metrics.comparisons<<" | time: "<<metrics.microseconds<<" us\n";
            out<<(merge ? "Merge: O(n log n), O(n) extra space." : "Bubble: O(n^2) average/worst; O(n) best with early exit.")<<'\n';
        });
        return metrics;
    };
    if(choice==2) show(true);
    if(choice==3) show(false);

}


// Category menus keep related tasks together and return to the dashboard with 0.
void HospitalSystem::categoryMenu(int category) {
    for (;;) {
            try {
        if (category == 1) {
            ConsoleUI::menu("PATIENT MANAGEMENT", {"1. Patient records (Array + BST)", "2. Patient profile", "0. Back to main menu"});
            int choice = Input::number("Choose: ", 0, 2);
            if (!choice) throw MainMenuRequested{};
            if (choice == 1) patientMenu();
            else showPatientProfile();
        } else if (category == 2) {
            ConsoleUI::menu("APPOINTMENTS & QUEUES", {"1. Appointment management", "2. Service queues (Normal / Emergency)", "0. Back to main menu"});
            int choice = Input::number("Choose: ", 0, 2);
            if (!choice) throw MainMenuRequested{};
            if (choice == 1) appointmentMenu();
            else queueMenu();
        } else if (category == 5) {
            ConsoleUI::menu("REPORTS & ALGORITHMS", {"1. Department referrals (DFS / BFS)", "2. Sort & ordered display", "0. Back to main menu"});
            int choice = Input::number("Choose: ", 0, 2);
            if (!choice) throw MainMenuRequested{};
            if (choice == 1) graphMenu();
            else { for(;;) { sortingDemo(); } }
        } else {
            ConsoleUI::menu("SESSION & LOGS - TASK 3 STACK / LIFO",{"1. View operations","2. Undo last change (Stack / LIFO)","0. Back to main menu"});
            int choice=Input::number("Choose: ",0,2);
            if (!choice) throw MainMenuRequested{};
            if(choice==2) undoLast();
            else showAuditLog();
        }
                } catch (const InputCancelled&) {
                ConsoleUI::message("INFO", "Operation cancelled. Nothing saved.");
            }
        }
}

// Central import menu; incomplete/invalid files never partially modify the hospital.
void HospitalSystem::loadMenu() {
    for (;;) {
        try {
            ConsoleUI::menu("DATA & SESSION", {"1. Export data", "2. Restore backup", "3. Clear data", "4. View operations", "5. Undo last change (Stack / LIFO)", "0. Back to main menu"});
            int choice=Input::number("Choose: ",0,5);
            if (!choice) throw MainMenuRequested{};
            if (choice==1) exportData();
            else if(choice==2) restoreBackup();
            else if(choice==3) clearData();
            else if(choice==4) showAuditLog();
            else undoLast();
        } catch (const InputCancelled&) { ConsoleUI::message("INFO", "Operation cancelled. Nothing saved."); }
    }
}
void HospitalSystem::importPatients() {
    string path=Input::text("Patient CSV path (e.g. sample-patients.csv): ");
    if (path.size()>=2 && path.front()=='"' && path.back()=='"') path=path.substr(1,path.size()-2);
    ifstream file{filesystem::u8path(path)};
    if (!file) {ConsoleUI::message("!", "Cannot open patient CSV file.");return;}
    auto identity=[](const Patient& p) {
        ostringstream out;
        out<<quoted(p.name)<<' '<<p.age<<' '<<quoted(p.gender)<<' '<<quoted(p.department)<<' '<<quoted(p.condition);
        return out.str();
    };
    set<string> known;
    for (int i=0;i<state.count;++i) known.insert(identity(state.patients[i]));
    vector<Patient> batch;
    string line;
    size_t lineNumber=0,skipped=0;
    try {
        while (getline(file,line)) {
            ++lineNumber;
            if (lineNumber==1 && line.rfind("\xEF\xBB\xBF",0)==0) line.erase(0,3);
            if (CsvReader::trim(line).empty()) continue;
            auto fields=CsvReader::fields(line);
            if (fields.size()!=5) throw runtime_error("Expected name,age,gender,department,condition columns");
            if (lineNumber==1 && fields==vector<string>{"name","age","gender","department","condition"}) continue;
            if (fields[1].empty() || fields[1].size()>3 || !all_of(fields[1].begin(),fields[1].end(),[](char c){return c>='0' && c<='9';})) throw runtime_error("Invalid age");
            Patient patient;
            patient.name=fields[0];patient.age=stoi(fields[1]);patient.gender=fields[2];patient.department=fields[3];patient.condition=fields[4];
            string gender=patient.gender;
            transform(gender.begin(),gender.end(),gender.begin(),[](unsigned char c){return static_cast<char>(tolower(c));});
            if (gender=="female" || gender=="1") patient.gender="Female";
            else if (gender=="male" || gender=="2") patient.gender="Male";
            else if (gender=="other" || gender=="other/unspecified" || gender=="3") patient.gender="Other/unspecified";
            else throw runtime_error("Invalid gender");
            bool letter=false,digit=false;
            for (unsigned char c : patient.name) {
                letter|=(c>='A' && c<='Z') || (c>='a' && c<='z') || c>=128;
                digit|=c>='0' && c<='9';
            }
            if (!Input::validText(patient.name,100) || !letter || digit || patient.age>130 || !Input::validText(patient.department,100) || !Input::validText(patient.condition)) throw runtime_error("Invalid patient fields");
            if (!known.insert(identity(patient)).second) {++skipped;continue;}
            if (state.count+static_cast<int>(batch.size())>=capacity || nextPatientID+static_cast<int>(batch.size())>999999) throw runtime_error("Patient capacity or ID limit reached");
            patient.id=nextPatientID+static_cast<int>(batch.size());batch.push_back(patient);
        }
        if (file.bad()) throw runtime_error("File read failed");
    } catch (const exception& error) {ConsoleUI::message("!", "CSV line " + to_string(lineNumber) + ": " + error.what() + ". Nothing imported.");return;}
    if (batch.empty()) {ConsoleUI::message("INFO", "No new patients to import (empty file or identical records).");return;}
    remember("Import " + to_string(batch.size()) + " patients from CSV");
    for (const auto& patient : batch) {state.patients[state.count++]=patient;index.insert(patient.id);}
    nextPatientID+=static_cast<int>(batch.size());
    if (!commitChange()) {nextPatientID-=static_cast<int>(batch.size());return;}
    ConsoleUI::message("OK", "Imported " + to_string(batch.size()) + " patients; skipped " + to_string(skipped) + " identical records.");
    for (const auto& patient : batch) printPatient(patient);
}

// The dashboard exposes eight categories; each opens its own submenu.
void HospitalSystem::run() {
    for (;;) {
            try {
        ConsoleUI::dashboard(state.count, capacity, state.doctors.size(),
                             state.appointments.size(), state.serviceQueue.size() + state.emergency.records().size());
        int choice = Input::number("Choose: ", 0, 8);
        if (!choice) { ConsoleUI::message("OK", "Session ended. Successful changes were autosaved."); return; }
        if(choice==1) categoryMenu(1);
        else if(choice==2) doctorMenu();
        else if(choice==3) appointmentMenu();
        else if(choice==4) queueMenu();
        else if(choice==5) clinicalMenu();
        else if(choice==6) billingMenu();
        else if(choice==7) categoryMenu(5);
        else loadMenu();
                } catch (const MainMenuRequested&) {
                    // Back from any depth resumes the main dashboard.
                } catch (const InputCancelled&) {
                ConsoleUI::message("INFO", "Operation cancelled. Nothing saved.");
            }
        }
}

#include "HospitalExchange.inc"

#include "HospitalServices.inc"
