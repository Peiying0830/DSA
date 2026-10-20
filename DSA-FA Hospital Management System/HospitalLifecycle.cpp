// HospitalLifecycle.cpp
// Load/save transactions, LIFO Undo, schedules and patient lifecycle.
// DSA Final Assessment - Hospital Management System.

#include "HospitalSystem.h"
#include "ConsoleUI.h"
#include <algorithm>
#include <iostream>
#include <limits>
using namespace std;

// Keep a complete, ready-to-demo baseline in the program so Clear Data always
// returns to the same usable hospital without requiring CSV files.
void HospitalSystem::loadDefaultData() {
    state=HospitalData{};
    const array<Patient,10> patients{{
        {1,"Alex Tan",22,"Male","Cardiology","Chest pain"},
        {2,"Brenda Lim",30,"Female","General Medicine","Fever"},
        {3,"Chris Wong",12,"Male","Pediatrics","Cough"},
        {4,"Diana Lee",40,"Female","Orthopedics","Arm pain"},
        {5,"Ethan Goh",25,"Male","Emergency","Minor injury"},
        {6,"Farah Ali",28,"Female","Dermatology","Skin rash"},
        {7,"Grace Ng",55,"Female","Neurology","Headache"},
        {8,"Henry Koh",60,"Male","Ophthalmology","Blurred vision"},
        {9,"Irene Teo",35,"Female","ENT","Sore throat"},
        {10,"Jason Low",45,"Male","General Surgery","Follow-up review"}
    }};
    for(const auto& patient:patients) state.patients[state.count++]=patient;
    state.doctors={
        {1,"Alice Tan","Cardiology",6000}, {2,"Ben Lim","General Medicine",4000},
        {3,"Carol Wong","Pediatrics",4500}, {4,"David Lee","Orthopedics",6500},
        {5,"Emma Chan","Emergency",5000}, {6,"Farah Ahmad","Dermatology",5500},
        {7,"Grace Ng","Neurology",7500}, {8,"Henry Goh","Ophthalmology",6000},
        {9,"Irene Teo","ENT",4500}, {10,"Jason Low","General Surgery",8000}
    };
    const array<string,6> weekdays{{"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"}};
    for(const auto& doctor:state.doctors) for(size_t day=0;day<weekdays.size();++day)
        state.shifts.push_back({doctor.id,weekdays[day],"09:00",day==5 ? "13:00" : "17:00"});
    const array<Drug,10> medicines{{
        {1,200,30,"Paracetamol 500mg","tablet",50}, {2,120,20,"Ibuprofen 200mg","tablet",80},
        {3,100,15,"Cetirizine 10mg","tablet",100}, {4,80,10,"Oral Rehydration Salts","sachet",250},
        {5,40,8,"Antacid Suspension","bottle",850}, {6,35,5,"Saline Nasal Spray","bottle",1200},
        {7,150,20,"Vitamin C 500mg","tablet",60}, {8,100,15,"Adhesive Dressing","piece",150},
        {9,100,15,"Amoxicillin 500mg","capsule",120}, {10,30,6,"Cough Syrup","bottle",900}
    }};
    for(const auto& medicine:medicines) state.pharmacy.add(medicine);
    state.services={{1,"Chest X-ray",6000},{2,"Full blood count (FBC)",3000},{3,"ECG",4000}};
    nextPatientID=11; nextDoctorID=11; nextAppointmentID=1; nextRecordID=1;
    nextPrescriptionID=1; nextInvoiceID=1; nextServiceOrderID=1; nextEmergencySequence=1;
    rebuildIndex();
}

// Load saved data if present, restore ID counters and rebuild the BST index.
HospitalSystem::HospitalSystem(const filesystem::path& dataDirectory) : storage(dataDirectory) {
    if (storage.exists()) {
        auto saved=storage.load(); // Reject damaged files without overwriting them.
        state=saved.data;
        nextServiceOrderID=saved.nextServiceOrderID;
        nextDoctorID=saved.nextDoctorID;
        nextPatientID=saved.nextPatientID;
        nextAppointmentID=saved.nextAppointmentID;
        nextRecordID=saved.nextRecordID;
        nextEmergencySequence=saved.nextEmergencySequence;
        nextPrescriptionID=saved.nextPrescriptionID;
        nextInvoiceID=saved.nextInvoiceID;
        audit=move(saved.audit);
        rebuildIndex();
        ConsoleUI::panel([&](ostream& out) { out << "Saved hospital data loaded.\n"; });
    } else {
        loadDefaultData();
        storage.save(snapshot());
        ConsoleUI::panel([&](ostream& out) { out << "Default demo data loaded: 10 patients, 10 doctors and 10 medicines.\n"; });
    }
    ConsoleUI::panel([&](ostream& out) { out << "Autosave: " << storage.path() << '\n'; });
}
// Collect current records, counters and audit entries into one consistent save object.
SavedHospital HospitalSystem::snapshot() const {
    return {state,nextAppointmentID,nextRecordID,nextEmergencySequence,audit,nextPrescriptionID,nextInvoiceID,nextPatientID,nextDoctorID,nextServiceOrderID};
}
// LIFO: push the pre-change state onto the Undo stack before modifying records.
void HospitalSystem::remember(const string& description) {
    // Stack top is the most recent operation; older operations remain underneath.
    undo.push({state,description});
    pendingAction=description;
}
// Autosave the completed change; restore its previous state if saving fails.
bool HospitalSystem::commitChange() {
    audit.push_back({Persistence::timestamp(),pendingAction});
    try {
        storage.save(snapshot());
        pendingAction.clear();
        ConsoleUI::savedNoticePending = true;
        return true;
    } catch (const exception& error) {
        audit.pop_back();
        // An unsaved change must not remain visible as a successful modification.
        state=move(undo.top().previous);
        undo.pop();
        rebuildIndex();
        pendingAction.clear();
        ConsoleUI::panel([&](ostream& out) { out << "Change rolled back because autosave failed: " << error.what() << '\n'; });
        return false;
    }
}
// LIFO: restore the most recent snapshot, autosave it, then pop that Undo entry.
void HospitalSystem::undoLast() {
    if (undo.empty()) { ConsoleUI::message("INFO", "No operations available to undo."); return; }
    HospitalData current=state;
    string description=undo.top().description;
    // Restore before saving, but keep the Undo entry until the save succeeds.
    state=undo.top().previous;
    audit.push_back({Persistence::timestamp(),"Undo: "+description});
    try {
        storage.save(snapshot());
        // LIFO removal: discard exactly the operation that was just undone.
        undo.pop();
        rebuildIndex();
        ConsoleUI::panel([&](ostream& out) { out << "Undone: " << description << "\nAutosaved.\n"; });
    } catch (const exception& error) {
        state=move(current);
        audit.pop_back();
        ConsoleUI::panel([&](ostream& out) { out << "Undo rejected because autosave failed: " << error.what() << '\n'; });
    }
}

void HospitalSystem::showDoctorShifts() const {
            for (const auto& doctor : state.doctors) {
                vector<size_t> positions;
                for (size_t i=0;i<state.shifts.size();++i)
                    if (state.shifts[i].doctorID == doctor.id) positions.push_back(i);
                if (positions.empty()) continue;
                auto rank = [](const string& day) {
                    for (int i=1;i<=7;++i) if (day == ScheduleBook::dayName(i)) return i;
                    return 8;
                };
                sort(positions.begin(), positions.end(), [&](size_t a, size_t b) {
                    const auto& left=state.shifts[a]; const auto& right=state.shifts[b];
                    if (rank(left.date)!=rank(right.date)) return rank(left.date)<rank(right.date);
                    if (left.date!=right.date) return left.date<right.date;
                    return left.start<right.start;
                });
                ConsoleUI::rule();
                ConsoleUI::row(formatDoctorID(doctor.id) + " | " + doctor.name);
                ConsoleUI::field("Department", doctor.department);
                ConsoleUI::row();
                for (size_t position : positions) {
                    const auto& shift=state.shifts[position];
                    ConsoleUI::row("[" + to_string(position+1) + "] " + shift.date + " | " + shift.start + "-" + shift.end);
                }
                ConsoleUI::rule();
                cout << '\n';
            }
}

// Add/view/remove dated doctor shifts, rejecting overlap and removal of booked periods.
void HospitalSystem::scheduleMenu() {
    for (;;) {
            try {
        ConsoleUI::menu("DOCTOR SHIFTS", {"1. Add weekly shift", "2. Remove shift", "0. Back to main menu"});
        int choice=Input::number("Choose: ",0,2);
        if (!choice) throw MainMenuRequested{};
        if (choice==1) {
            if (state.doctors.empty()) { ConsoleUI::message("!", "Register a doctor first."); continue; }
            showDoctors();
            int id=Input::doctorID("Doctor ID: ");
            if (doctorPosition(id)<0) { ConsoleUI::message("!", "Doctor not found."); continue; }
            ConsoleUI::menu("WEEKDAY", {"1. Monday", "2. Tuesday", "3. Wednesday", "4. Thursday", "5. Friday", "6. Saturday", "7. Sunday", "0. Cancel"});
            int day=Input::number("Weekday (1-7, 0 to cancel): ",1,7);
            string date=ScheduleBook::dayName(day),start,end;
            for (;;) {
                start=Input::text("Shift start (HH:MM, 00/30 minutes): ");
                end=Input::text("Shift end (HH:MM; 24:00 allowed): ");
                if (ScheduleBook::slot(start)>=0 && ScheduleBook::slot(end,true)>ScheduleBook::slot(start)) break;
                ConsoleUI::panel([&](ostream& out) { out << "Enter a valid same-day shift with end after start.\n"; });
            }
            bool overlaps=false;
            for (const auto& s : state.shifts)
                if (s.doctorID==id && s.date==date && ScheduleBook::slot(start)<ScheduleBook::slot(s.end,true) && ScheduleBook::slot(end,true)>ScheduleBook::slot(s.start)) overlaps=true;
            if (overlaps) { ConsoleUI::message("!", "Shift overlaps an existing shift."); continue; }
            remember("Add shift for doctor "+to_string(id)+" on "+date);
            state.shifts.push_back({id,date,start,end});
            if (commitChange()) ConsoleUI::message("OK", "Doctor shift added.");
        } else {
            if (state.shifts.empty()) { ConsoleUI::message("INFO", "No shifts entered."); continue; }
            showDoctorShifts();
            int position=Input::number("Shift number to remove: ",1,static_cast<int>(state.shifts.size()))-1;
            const auto shift=state.shifts[position];
            bool booked=false;
            for (const auto& a : state.appointments)
                if (ScheduleBook::active(a) && a.doctorID==shift.doctorID && ScheduleBook::matches(shift.date,a.date) && ScheduleBook::slot(a.time)>=ScheduleBook::slot(shift.start) && ScheduleBook::slot(a.time)<ScheduleBook::slot(shift.end,true)) booked=true;
            if (booked) { ConsoleUI::message("!", "Cancel or reschedule active appointments before removing this shift."); continue; }
            remember("Remove shift for doctor "+to_string(shift.doctorID)+" on "+shift.date);
            state.shifts.erase(state.shifts.begin()+position);
            if (commitChange()) ConsoleUI::message("OK", "Shift removed.");
        }
                } catch (const InputCancelled&) {
                ConsoleUI::message("INFO", "Operation cancelled. Nothing saved.");
            }
        }
}

// Show the exact queue entry; only identify a visit when its stored label matches.
bool HospitalSystem::showWaitingConflict(int patientID, int appointmentID) const {
    auto normal=state.serviceQueue.records();
    auto entry=find_if(normal.begin(),normal.end(),[&](const QueueRecord& q){return q.patientID==patientID;});
    auto emergency=state.emergency.records();
    auto urgent=find_if(emergency.begin(),emergency.end(),[&](const EmergencyRecord& q){return q.patientID==patientID;});
    if(entry==normal.end() && urgent==emergency.end()) return false;
    ConsoleUI::RecordBox box;
    ConsoleUI::row("[!] Patient already waiting in a queue.");
    ConsoleUI::field("Patient",formatPatientID(patientID));
    if(appointmentID) ConsoleUI::field("Requested check-in",formatID("A",appointmentID));
    if(entry!=normal.end()) {
        ConsoleUI::field("Queue","Normal (FIFO)");
        ConsoleUI::field("Position",to_string(distance(normal.begin(),entry)+1)+" of "+to_string(normal.size()));
        const Appointment* visit=nullptr;
        for(const auto& a:state.appointments)
            if(a.patientID==patientID && entry->service=="Consultation (appointment #"+to_string(a.id)+")") {visit=&a;break;}
        if(visit) {
            ConsoleUI::field("Waiting appointment",formatID("A",visit->id));
            ConsoleUI::field("Visit",visit->date+" "+visit->time);
            int position=doctorPosition(visit->doctorID);
            ConsoleUI::field("Doctor",formatDoctorID(visit->doctorID)+(position>=0 ? " ("+state.doctors[position].name+")" : ""));
        } else {
            ConsoleUI::field("Appointment","Not linked to this queue entry.");
            ConsoleUI::field("Service",entry->service);
        }
        ConsoleUI::row("Next: Appointments & queues > Service queues > Normal.");
        ConsoleUI::row("Use Serve patients; FIFO serves earlier patients first.");
        if(visit) ConsoleUI::row("Then complete "+formatID("A",visit->id)+" in Appointment status / check-in.");
        ConsoleUI::row("After that, retry this operation.");
    } else {
        ConsoleUI::field("Queue","Emergency (priority; equal priority FIFO)");
        ConsoleUI::field("Priority",to_string(urgent->priority)+" (1 = highest)");
        ConsoleUI::field("Reason",urgent->reason);
        ConsoleUI::field("Appointment","Not linked to this emergency queue entry.");
        ConsoleUI::row("Next: Appointments & queues > Service queues > Emergency.");
        ConsoleUI::row("Use Serve highest priority until this patient is served.");
        ConsoleUI::row("Then retry this operation.");
    }
    return true;
}

// Check in into the FIFO queue, record a no-show or complete a served appointment.
void HospitalSystem::updateAppointmentStatus() {
    int id=Input::entityID("Appointment ID: ", "A");
    auto found=find_if(state.appointments.begin(),state.appointments.end(),[id](const Appointment& a){return a.id==id;});
    if (found==state.appointments.end()) { ConsoleUI::message("!", "Appointment not found."); return; }
    ConsoleUI::panel([&](ostream& out) { out << "Current status: " << found->status << '\n'; });
    if (!ScheduleBook::active(*found)) { ConsoleUI::message("!", "This appointment is already closed."); return; }
    if (found->status=="Scheduled") {
        int choice=Input::number("1. Check in 2. Mark no-show 0. Back to main menu: ",0,2);
        if (!choice) throw MainMenuRequested{};
        if (choice==1) {
            if (showWaitingConflict(found->patientID,found->id)) return;
            if (state.serviceQueue.size()==100) { ConsoleUI::message("!", "Service queue is full."); return; }
            int doctorIndex=doctorPosition(found->doctorID);
            if (doctorIndex<0 || !state.doctors[doctorIndex].consultationFeeCents) {
                ConsoleUI::message("!", "Set this doctor's consultation fee before check-in."); return;
            }
            if (nextInvoiceID>999999) { ConsoleUI::message("!", "Invoice ID limit reached."); return; }
            remember("Check in appointment "+to_string(id));
            const auto& p=state.patients[patientPosition(found->patientID)];
            state.serviceQueue.enqueue({p.id,p.name,p.department,"Consultation (appointment #"+to_string(id)+")","Waiting"});
            found->status="Checked-in";
            Invoice invoice;
            invoice.id=nextInvoiceID;
            invoice.patientID=found->patientID;
            invoice.appointmentID=found->id;
            invoice.createdAt=Persistence::timestamp();
            const auto& doctor=state.doctors[doctorIndex];
            invoice.items.push_back({"Consultation - "+formatDoctorID(doctor.id)+" "+doctor.name,1,doctor.consultationFeeCents});
            if (!state.billing.add(invoice)) {
                state=move(undo.top().previous); undo.pop(); pendingAction.clear(); rebuildIndex();
                ConsoleUI::message("!", "Consultation invoice could not be created."); return;
            }
            ++nextInvoiceID;
        } else {
            remember("Mark no-show appointment "+to_string(id));
            found->status="No-show";
        }
    } else {
        if (state.serviceQueue.hasPatient(found->patientID) || state.emergency.hasPatient(found->patientID)) {
            ConsoleUI::message("!", "Serve this patient from the queue before completing the appointment."); return;
        }
        completeVisit(*found);
        return;
    }
    if (commitChange()) {
        if (found->status=="Checked-in") ConsoleUI::message("OK", "Checked in. Consultation invoice "+formatID("I",nextInvoiceID-1)+" is ready for payment; selected services will be added to it.");
        else ConsoleUI::message("OK", "Appointment status updated.");
    }
}

// Combine patient basics, history, appointments, queues, prescriptions and invoices.
void HospitalSystem::showPatientProfile() const {
    int id=Input::patientID("Patient ID: ");
    int position=patientPosition(id);
    if (position<0) { ConsoleUI::message("!", "Patient not found."); return; }
    ConsoleUI::RecordBox frame;
    ConsoleUI::banner("COMPLETE PATIENT PROFILE");
    printPatient(state.patients[position]);
    ConsoleUI::panel([&](ostream& out) { out << "Medical history (Task 2 - Linked List):\n"; });
    auto records=state.history.records(id);
    if (records.empty()) ConsoleUI::message("INFO", "No medical history.");
    printMedicalHistory(records, id);
    ConsoleUI::panel([&](ostream& out) { out << "Appointments:\n"; });
    showAppointments(id);
    ConsoleUI::panel([&](ostream& out) { out << "Queue status:\n"; });
    bool waiting=false;
    for (const auto& r : state.serviceQueue.records()) if (r.patientID==id) { ConsoleUI::panel([&](ostream& out) { out << "Service queue | " << r.service << " | " << r.status << '\n'; }); waiting=true; }
    for (const auto& r : state.emergency.records()) if (r.patientID==id) { ConsoleUI::panel([&](ostream& out) { out << "Emergency queue | Priority " << r.priority << " | " << r.reason << '\n'; }); waiting=true; }
    if (!waiting) ConsoleUI::panel([&](ostream& out) { out << "Not waiting in a queue.\n"; });
    ConsoleUI::panel([&](ostream& out) { out << "Prescriptions:\n"; });
    showPrescriptions(id);
    ConsoleUI::panel([&](ostream& out) { out << "Invoices:\n"; });
    showInvoices(id);
}
// Display persisted operation descriptions and Malaysia-time timestamps.
void HospitalSystem::showAuditLog() const {
    ConsoleUI::RecordBox box;
    ConsoleUI::row("OPERATION LOG (Malaysia time)", true);
    ConsoleUI::row(undo.empty() ? "Undo stack is empty." : "Latest operation (stack top): " + undo.top().description);
    if (audit.empty()) ConsoleUI::row("No recorded changes.");
    for (const auto& entry : audit)
        ConsoleUI::row(entry.timestamp + " | " + entry.action);
}
// Serve highest-priority cases first; equal priorities retain arrival order.
void HospitalSystem::emergencyMenu() {
    for (;;) {
            try {
        ConsoleUI::menu("EMERGENCY PRIORITY QUEUE - TASK 9 HEAP / FIFO TIES", {"1. Enqueue (staff-assigned priority)", "2. Display waiting (priority order)", "3. Serve highest priority (ties: FIFO)", "0. Back to main menu"});
        int choice=Input::number("Choose: ",0,3);
        if (!choice) throw MainMenuRequested{};
        if (choice==1) {
            int id=Input::patientID("Patient ID: ");
            if (!index.contains(id)) { ConsoleUI::message("!", "Patient not found."); continue; }
            if (showWaitingConflict(id)) continue;
            if (state.emergency.records().size()==100) { ConsoleUI::message("!", "Emergency queue is full."); continue; }
            int priority=Input::number("Priority assigned by staff (1=highest, 2=medium, 3=lowest): ",1,3);
            string reason=Input::text("Reason: ");
            if (nextEmergencySequence==numeric_limits<unsigned long long>::max()) { ConsoleUI::message("!", "Sequence limit reached."); continue; }
            remember("Emergency enqueue patient "+to_string(id));
            state.emergency.enqueue({id,priority,nextEmergencySequence++,reason});
            if (commitChange()) ConsoleUI::panel([&](ostream& out) { out << "Emergency patient enqueued. Equal priority uses arrival order.\n"; });
        } else if (choice==3) {
            if (state.emergency.records().empty()) { ConsoleUI::panel([&](ostream& out) { out << "Emergency queue is empty.\n"; }); continue; }
            remember("Serve emergency patient "+to_string(state.emergency.records().front().patientID));
            EmergencyRecord record;
            state.emergency.dequeue(record);
            if (commitChange()) ConsoleUI::panel([&](ostream& out) { out << "Serving emergency patient " << formatPatientID(record.patientID) << " | Priority " << record.priority << '\n'; });
        } else {
            auto records=state.emergency.records();
            ConsoleUI::banner("WAITING PATIENTS (EMERGENCY PRIORITY ORDER)");
            if (records.empty()) ConsoleUI::message("INFO", "Emergency queue is empty.");
            for (const auto& r : records) {
                const auto& patient=state.patients[patientPosition(r.patientID)];
                ConsoleUI::RecordBox frame;
                ConsoleUI::row(formatPatientID(r.patientID) + " | " + patient.name);
                ConsoleUI::field("Queue", "Emergency (priority; equal priority FIFO)");
                ConsoleUI::field("Department", patient.department);
                ConsoleUI::field("Priority", to_string(r.priority));
                ConsoleUI::field("Reason", r.reason);
                ConsoleUI::field("Status", "Waiting");
            }
        }
                } catch (const InputCancelled&) {
                ConsoleUI::message("INFO", "Operation cancelled. Nothing saved.");
            }
        }
}
