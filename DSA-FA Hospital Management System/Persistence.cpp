// Persistence.cpp
// Validated, versioned autosave with a temporary file and previous-state backup.
// DSA Final Assessment - Hospital Management System.

#include "Persistence.h"
#include <algorithm>
#include "Input.h"
#include "ScheduleBook.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <set>
#include <map>
#include <sstream>
#include <stdexcept>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
using namespace std;
namespace {
// Stop loading with a clear validation error instead of accepting a damaged save.
void require(bool condition, const string& message) {
    if (!condition) throw runtime_error("Invalid save file: " + message);
}
// Read an expected section label and bounded record count before allocating/looping.
size_t section(istream& in, const string& expected, size_t maximum=100000) {
    string tag; size_t count=0;
    in >> tag >> count;
// Stop loading with a clear validation error instead of accepting a damaged save.
    require(bool(in) && tag==expected && count<=maximum, expected+" section");
    return count;
}
// Check the common positive hospital identifier range.
bool idValid(int id) { return id>=1 && id<=999999; }
}
// Keep the selected data directory; no records are written by construction.
Persistence::Persistence(filesystem::path directory, filesystem::path filename) : directory(move(directory)), filename(move(filename)) {}
// Check whether the main hospital database already exists.
bool Persistence::exists() const {
    const auto target=directory/filename;
    // Clear/new-system state may leave an empty placeholder file. It contains no
    // database header, so treat it exactly like no save file instead of rejecting it.
    return filesystem::exists(target) && filesystem::is_regular_file(target) && filesystem::file_size(target)>0;
}
// Return the main save path for startup messages and error reporting.
string Persistence::path() const { return (directory/filename).string(); }
// Format current time as Malaysia time (UTC+8), independent of host timezone.
string Persistence::timestamp() {
    // Malaysia uses UTC+8 year-round, independent of the computer's timezone.
    auto now=chrono::system_clock::now()+chrono::hours(8);
    time_t seconds=chrono::system_clock::to_time_t(now);
    tm value{};
#ifdef _WIN32
    gmtime_s(&value,&seconds);
#else
    gmtime_r(&seconds,&value);
#endif
    ostringstream out;
    out << put_time(&value,"%Y-%m-%d %H:%M:%S") << " MYT";
    return out.str();
}
// Parse into temporary state; validate IDs, references, statuses and sections before returning it.
SavedHospital Persistence::load() const {
    ifstream in(directory/filename);
    if (!in) throw runtime_error("Cannot read save file: "+path());
    SavedHospital saved;
    string header;
    int version=0;
    if(in.peek()=='+') {
        string line;bool payload=false;
        while(getline(in,line)) if(line=="=== RESTORABLE DATABASE ===") {payload=true;break;}
        require(payload,"backup data section");
    }
    in >> header >> version >> saved.nextAppointmentID >> saved.nextRecordID >> saved.nextEmergencySequence;
// Stop loading with a clear validation error instead of accepting a damaged save.
    require(bool(in) && header=="DSA_HOSPITAL" && (version>=1 && version<=8),"header/version");
    if (version>=2) in >> saved.nextPrescriptionID >> saved.nextInvoiceID;
// Stop loading with a clear validation error instead of accepting a damaged save.
    require(saved.nextAppointmentID>0 && saved.nextAppointmentID<=1000000 && saved.nextRecordID>0 && saved.nextRecordID<=1000000 && saved.nextEmergencySequence>0,"ID counters");
// Stop loading with a clear validation error instead of accepting a damaged save.
    require(bool(in) && saved.nextPrescriptionID>=1 && saved.nextPrescriptionID<=1000000 && saved.nextInvoiceID>=1 && saved.nextInvoiceID<=1000000,"commerce ID counters");
    if (version>=3) in >> saved.nextPatientID;
    require(bool(in) && saved.nextPatientID>=1 && saved.nextPatientID<=1000000,"patient ID counter");
    if (version>=4) in >> saved.nextDoctorID;
    require(bool(in) && saved.nextDoctorID>=1 && saved.nextDoctorID<=1000000,"doctor ID counter");
    if(version>=7) in >> saved.nextServiceOrderID;
    require(bool(in) && saved.nextServiceOrderID>=1 && saved.nextServiceOrderID<=1000000,"service order counter");
    auto& data=saved.data;
    set<int> patients,doctors,appointments,medical,queued,emergency;
    int maxAppointment=0,maxMedical=0;
    unsigned long long maxSequence=0;
    size_t count=section(in,"PATIENTS",100);
    for (size_t i=0;i<count;++i) {
        Patient p;
        in >> p.id >> quoted(p.name) >> p.age >> quoted(p.gender) >> quoted(p.department) >> quoted(p.condition);
// Stop loading with a clear validation error instead of accepting a damaged save.
        require(bool(in) && idValid(p.id) && patients.insert(p.id).second && p.age>=0 && p.age<=130 && !p.name.empty() && !p.department.empty() && !p.condition.empty(),"patient");
        data.patients[data.count++]=p;
    }
    count=section(in,"DOCTORS");
    for (size_t i=0;i<count;++i) {
        Doctor d; in >> d.id >> quoted(d.name) >> quoted(d.department);
        if(version>=7) in >> d.consultationFeeCents;
        require(bool(in) && d.consultationFeeCents>=0 && d.consultationFeeCents<=100000000,"consultation fee");
// Stop loading with a clear validation error instead of accepting a damaged save.
        require(bool(in) && idValid(d.id) && doctors.insert(d.id).second && !d.name.empty() && !d.department.empty(),"doctor");
        data.doctors.push_back(d);
    }
    count=section(in,"SHIFTS");
    for (size_t i=0;i<count;++i) {
        DoctorShift s; in >> s.doctorID >> quoted(s.date) >> quoted(s.start) >> quoted(s.end);
        int start=ScheduleBook::slot(s.start),end=ScheduleBook::slot(s.end,true);
// Stop loading with a clear validation error instead of accepting a damaged save.
        require(bool(in) && doctors.count(s.doctorID) && ScheduleBook::validShiftDay(s.date) && start>=0 && end>start,"doctor shift");
        for (const auto& old : data.shifts)
// Stop loading with a clear validation error instead of accepting a damaged save.
            require(old.doctorID!=s.doctorID || old.date!=s.date || end<=ScheduleBook::slot(old.start) || start>=ScheduleBook::slot(old.end,true),"overlapping shifts");
        data.shifts.push_back(s);
    }
    count=section(in,"APPOINTMENTS");
    const set<string> statuses={"Scheduled","Checked-in","Completed","Cancelled","No-show"};
    for (size_t i=0;i<count;++i) {
        Appointment a; in >> a.id >> a.patientID >> a.doctorID >> quoted(a.date) >> quoted(a.time) >> quoted(a.reason) >> quoted(a.status);
// Stop loading with a clear validation error instead of accepting a damaged save.
        require(bool(in) && idValid(a.id) && appointments.insert(a.id).second && patients.count(a.patientID) && doctors.count(a.doctorID) && Input::validDate(a.date) && ScheduleBook::slot(a.time)>=0 && !a.reason.empty() && statuses.count(a.status),"appointment");
        if (ScheduleBook::active(a)) {
// Stop loading with a clear validation error instead of accepting a damaged save.
            require(ScheduleBook::working(data.shifts,a.doctorID,a.date,a.time),"appointment outside shift");
            for (const auto& old : data.appointments)
// Stop loading with a clear validation error instead of accepting a damaged save.
                require(!ScheduleBook::active(old) || old.date!=a.date || old.time!=a.time || (old.doctorID!=a.doctorID && old.patientID!=a.patientID),"appointment conflict");
        }
        maxAppointment=max(maxAppointment,a.id);
        data.appointments.push_back(a);
    }
    count=section(in,"HISTORY");
    for (size_t i=0;i<count;++i) {
        MedicalRecord r; in >> r.id >> r.patientID >> quoted(r.date) >> quoted(r.type) >> quoted(r.details);
        if(version>=6) in >> r.appointmentID;
        require(bool(in) && r.appointmentID>=0,"medical visit ID");
        if(r.appointmentID) {
            auto visit=find_if(data.appointments.begin(),data.appointments.end(),[&](const Appointment& appointment){return appointment.id==r.appointmentID;});
            require(visit!=data.appointments.end() && visit->patientID==r.patientID && visit->date==r.date,"medical visit reference");
        }
// Stop loading with a clear validation error instead of accepting a damaged save.
        require(bool(in) && idValid(r.id) && medical.insert(r.id).second && patients.count(r.patientID) && Input::validDate(r.date) && !r.type.empty() && !r.details.empty(),"medical record");
        maxMedical=max(maxMedical,r.id); data.history.add(r);
    }
    count=section(in,"QUEUE",100);
    for (size_t i=0;i<count;++i) {
        QueueRecord r; in >> r.patientID >> quoted(r.name) >> quoted(r.department) >> quoted(r.service) >> quoted(r.status);
// Stop loading with a clear validation error instead of accepting a damaged save.
        require(bool(in) && patients.count(r.patientID) && queued.insert(r.patientID).second && !r.service.empty(),"service queue");
        data.serviceQueue.enqueue(r);
    }
    count=section(in,"EMERGENCY",100);
    set<unsigned long long> sequences;
    for (size_t i=0;i<count;++i) {
        EmergencyRecord r; in >> r.patientID >> r.priority >> r.sequence >> quoted(r.reason);
// Stop loading with a clear validation error instead of accepting a damaged save.
        require(bool(in) && patients.count(r.patientID) && emergency.insert(r.patientID).second && !queued.count(r.patientID) && r.priority>=1 && r.priority<=3 && r.sequence>0 && sequences.insert(r.sequence).second && !r.reason.empty(),"emergency queue");
        maxSequence=max(maxSequence,r.sequence); data.emergency.enqueue(r);
    }
    // Older version 1 databases have no pharmacy/billing sections; keep them readable.
    if (version>=2) {
        count=section(in,"DRUGS");
        for (size_t i=0;i<count;++i) {
            Drug drug;
            in >> drug.id >> quoted(drug.name) >> quoted(drug.unit) >> drug.stock >> drug.reorderLevel >> drug.priceCents;
// Stop loading with a clear validation error instead of accepting a damaged save.
            require(bool(in) && data.pharmacy.add(drug),"medicine");
        }
        count=section(in,"PRESCRIPTIONS");
        int maxPrescription=0,maxInvoice=0;
        for (size_t i=0;i<count;++i) {
            Prescription r;
            size_t itemCount=0;
            in >> r.id >> r.patientID >> r.doctorID >> quoted(r.date) >> quoted(r.status) >> itemCount;
            if(version>=8) in >> r.appointmentID >> r.invoiceID >> r.invoiceItemOffset;
            if(r.appointmentID) {
                auto visit=find_if(data.appointments.begin(),data.appointments.end(),[&](const Appointment& a){return a.id==r.appointmentID;});
                require(visit!=data.appointments.end() && visit->patientID==r.patientID && visit->doctorID==r.doctorID && visit->date==r.date,"prescription visit reference");
            }
// Stop loading with a clear validation error instead of accepting a damaged save.
            require(bool(in) && patients.count(r.patientID) && doctors.count(r.doctorID) && Input::validDate(r.date) && itemCount>=1 && itemCount<=20,"prescription header");
            for (size_t item=0;item<itemCount;++item) {
                PrescriptionItem line;
                in >> line.drugID >> line.quantity >> quoted(line.instructions);
// Stop loading with a clear validation error instead of accepting a damaged save.
                require(bool(in) && data.pharmacy.find(line.drugID),"prescription medicine reference");
                r.items.push_back(line);
            }
// Stop loading with a clear validation error instead of accepting a damaged save.
            require(data.prescriptions.add(r),"prescription items/status/ID");
            maxPrescription=max(maxPrescription,r.id);
        }
        count=section(in,"INVOICES");
        set<int> billedPrescriptions;
        for (size_t i=0;i<count;++i) {
            Invoice invoice;
            size_t itemCount=0;
            in >> invoice.id >> invoice.patientID >> invoice.prescriptionID >> quoted(invoice.createdAt)
               >> quoted(invoice.status) >> quoted(invoice.paidAt) >> quoted(invoice.paymentMethod) >> itemCount;
            if(version>=7) in >> invoice.appointmentID;
            if(invoice.appointmentID) {
                auto visit=find_if(data.appointments.begin(),data.appointments.end(),[&](const Appointment& a){return a.id==invoice.appointmentID;});
                // Consultation invoices are issued at check-in and retained after completion.
                require(visit!=data.appointments.end() && visit->patientID==invoice.patientID &&
                        (visit->status=="Checked-in" || visit->status=="Completed") && invoice.prescriptionID==0,"invoice visit reference");
            }
// Stop loading with a clear validation error instead of accepting a damaged save.
            require(bool(in) && patients.count(invoice.patientID) && invoice.prescriptionID>=0 && itemCount>=1 && itemCount<=20,"invoice header");
            const Prescription* prescription=nullptr;
            if (invoice.prescriptionID) {
                prescription=data.prescriptions.find(invoice.prescriptionID);
// Stop loading with a clear validation error instead of accepting a damaged save.
                require(prescription && prescription->status=="Dispensed" && prescription->patientID==invoice.patientID && prescription->items.size()==itemCount && billedPrescriptions.insert(invoice.prescriptionID).second,"invoice prescription reference");
            }
            for (size_t item=0;item<itemCount;++item) {
                BillItem line;
                in >> quoted(line.description) >> line.quantity >> line.unitPriceCents;
// Stop loading with a clear validation error instead of accepting a damaged save.
                require(bool(in),"invoice item");
                if (prescription) require(line.quantity==prescription->items[item].quantity,"dispensed invoice quantity");
                invoice.items.push_back(line);
            }
// Stop loading with a clear validation error instead of accepting a damaged save.
            require(data.billing.add(invoice),"invoice amounts/status/payment/ID");
            maxInvoice=max(maxInvoice,invoice.id);
        }
        set<pair<int,int>> chargedLines;
        for (const auto& prescription : data.prescriptions.records()) {
            if(prescription.invoiceID) {
                const auto* bill=data.billing.find(prescription.invoiceID);
                require(bill && bill->patientID==prescription.patientID && prescription.status=="Dispensed","prescription invoice reference");
                if(bill->prescriptionID) {
                    require(bill->prescriptionID==prescription.id && prescription.invoiceItemOffset==0,"prescription standalone invoice");
                } else {
                    require(prescription.appointmentID && bill->appointmentID==prescription.appointmentID && prescription.invoiceItemOffset>=1 && billedPrescriptions.insert(prescription.id).second,"combined prescription visit invoice");
                }
                require(static_cast<size_t>(prescription.invoiceItemOffset)+prescription.items.size()<=bill->items.size(),"prescription invoice lines");
                for(size_t i=0;i<prescription.items.size();++i) {
                    int index=prescription.invoiceItemOffset+static_cast<int>(i);
                    require(chargedLines.insert({bill->id,index}).second && bill->items[index].quantity==prescription.items[i].quantity,"duplicate/mismatched prescription charge");
                }
            }
            require(prescription.status!="Dispensed" || billedPrescriptions.count(prescription.id),"dispensed prescription missing invoice");
        }
// Stop loading with a clear validation error instead of accepting a damaged save.
        require(saved.nextPrescriptionID>maxPrescription && saved.nextInvoiceID>maxInvoice,"commerce ID reuse");
    }
    if(version>=7) {
        data.services.clear();set<int> serviceIDs,orderIDs;int maxOrder=0;
        count=section(in,"SERVICES");
        for(size_t i=0;i<count;++i) {
            HospitalService service;in>>service.id>>quoted(service.name)>>service.priceCents;
            require(bool(in) && idValid(service.id) && serviceIDs.insert(service.id).second && Input::validText(service.name,100) && service.priceCents>=1 && service.priceCents<=100000000,"service catalog");
            data.services.push_back(service);
        }
        count=section(in,"VISIT_SERVICES");
        map<int,set<int>> perVisit;
        for(size_t i=0;i<count;++i) {
            VisitService order;in>>order.id>>order.appointmentID>>order.serviceID>>order.quantity>>quoted(order.description)>>order.unitPriceCents;
            auto visit=find_if(data.appointments.begin(),data.appointments.end(),[&](const Appointment& a){return a.id==order.appointmentID;});
            require(bool(in) && idValid(order.id) && orderIDs.insert(order.id).second && serviceIDs.count(order.serviceID) && visit!=data.appointments.end() && (visit->status=="Checked-in" || visit->status=="Completed") && order.quantity>=1 && order.quantity<=100000 && Input::validText(order.description,100) && order.unitPriceCents>=1 && order.unitPriceCents<=100000000,"visit service order");
            require(perVisit[order.appointmentID].insert(order.serviceID).second && perVisit[order.appointmentID].size()<=19,"duplicate/excessive visit services");
            data.visitServices.push_back(order);maxOrder=max(maxOrder,order.id);
        }
        require(saved.nextServiceOrderID>maxOrder,"service order ID reuse");
    }
    count=section(in,"AUDIT");
    for (size_t i=0;i<count;++i) {
        AuditEntry entry; in >> quoted(entry.timestamp) >> quoted(entry.action);
// Stop loading with a clear validation error instead of accepting a damaged save.
        require(bool(in) && !entry.timestamp.empty() && !entry.action.empty(),"audit entry");
        saved.audit.push_back(entry);
    }
    string end; in >> end;
// Stop loading with a clear validation error instead of accepting a damaged save.
    require(bool(in) && end=="END","incomplete file");
    in >> ws;
// Stop loading with a clear validation error instead of accepting a damaged save.
    require(in.eof(),"unexpected trailing data");
// Stop loading with a clear validation error instead of accepting a damaged save.
    require(saved.nextAppointmentID>maxAppointment && saved.nextRecordID>maxMedical && saved.nextEmergencySequence>maxSequence,"reused ID counters");
    int maxPatient=patients.empty() ? 0 : *patients.rbegin();
    if (version<3) saved.nextPatientID=maxPatient+1;
    require(saved.nextPatientID>maxPatient,"reused patient ID counter");
    int maxDoctor=doctors.empty() ? 0 : *doctors.rbegin();
    if (version<4) saved.nextDoctorID=maxDoctor+1;
    require(saved.nextDoctorID>maxDoctor,"reused doctor ID counter");
    return saved;
}
// Write and close a temporary database, back up the old file, then replace the main file.
void Persistence::save(const SavedHospital& saved, const string& report) const {
    filesystem::create_directories(directory);
    // Do not truncate the working database while writing a new snapshot.
    auto temp=directory/(filename.string()+".tmp"), target=directory/filename;
    ofstream out(temp,ios::trunc);
    if (!out) throw runtime_error("Cannot create autosave file in "+directory.string());
    if(!report.empty()) out << report << "\n=== RESTORABLE DATABASE ===\n";
    out << "DSA_HOSPITAL 8 " << saved.nextAppointmentID << ' ' << saved.nextRecordID << ' ' << saved.nextEmergencySequence
        << ' ' << saved.nextPrescriptionID << ' ' << saved.nextInvoiceID << ' ' << saved.nextPatientID << ' ' << saved.nextDoctorID << ' ' << saved.nextServiceOrderID << '\n';
    const auto& d=saved.data;
    out << "PATIENTS " << d.count << '\n';
    for (int i=0;i<d.count;++i) {
        const auto& p=d.patients[i];
        out << p.id << ' ' << quoted(p.name) << ' ' << p.age << ' ' << quoted(p.gender) << ' ' << quoted(p.department) << ' ' << quoted(p.condition) << '\n';
    }
    out << "DOCTORS " << d.doctors.size() << '\n';
    for (const auto& r : d.doctors) out << r.id << ' ' << quoted(r.name) << ' ' << quoted(r.department) << ' ' << r.consultationFeeCents << '\n';
    out << "SHIFTS " << d.shifts.size() << '\n';
    for (const auto& r : d.shifts) out << r.doctorID << ' ' << quoted(r.date) << ' ' << quoted(r.start) << ' ' << quoted(r.end) << '\n';
    out << "APPOINTMENTS " << d.appointments.size() << '\n';
    for (const auto& r : d.appointments) out << r.id << ' ' << r.patientID << ' ' << r.doctorID << ' ' << quoted(r.date) << ' ' << quoted(r.time) << ' ' << quoted(r.reason) << ' ' << quoted(r.status) << '\n';
    auto history=d.history.records(); out << "HISTORY " << history.size() << '\n';
    for (const auto& r : history) out << r.id << ' ' << r.patientID << ' ' << quoted(r.date) << ' ' << quoted(r.type) << ' ' << quoted(r.details) << ' ' << r.appointmentID << '\n';
    auto queue=d.serviceQueue.records(); out << "QUEUE " << queue.size() << '\n';
    for (const auto& r : queue) out << r.patientID << ' ' << quoted(r.name) << ' ' << quoted(r.department) << ' ' << quoted(r.service) << ' ' << quoted(r.status) << '\n';
    auto emergency=d.emergency.records(); out << "EMERGENCY " << emergency.size() << '\n';
    for (const auto& r : emergency) out << r.patientID << ' ' << r.priority << ' ' << r.sequence << ' ' << quoted(r.reason) << '\n';
    auto drugs=d.pharmacy.records(); out << "DRUGS " << drugs.size() << '\n';
    for (const auto& r : drugs) out << r.id << ' ' << quoted(r.name) << ' ' << quoted(r.unit) << ' ' << r.stock << ' ' << r.reorderLevel << ' ' << r.priceCents << '\n';
    out << "PRESCRIPTIONS " << d.prescriptions.records().size() << '\n';
    for (const auto& r : d.prescriptions.records()) {
        out << r.id << ' ' << r.patientID << ' ' << r.doctorID << ' ' << quoted(r.date) << ' ' << quoted(r.status) << ' ' << r.items.size() << ' ' << r.appointmentID << ' ' << r.invoiceID << ' ' << r.invoiceItemOffset << '\n';
        for (const auto& item : r.items) out << item.drugID << ' ' << item.quantity << ' ' << quoted(item.instructions) << '\n';
    }
    out << "INVOICES " << d.billing.records().size() << '\n';
    for (const auto& r : d.billing.records()) {
        out << r.id << ' ' << r.patientID << ' ' << r.prescriptionID << ' ' << quoted(r.createdAt) << ' ' << quoted(r.status)
            << ' ' << quoted(r.paidAt) << ' ' << quoted(r.paymentMethod) << ' ' << r.items.size() << ' ' << r.appointmentID << '\n';
        for (const auto& item : r.items) out << quoted(item.description) << ' ' << item.quantity << ' ' << item.unitPriceCents << '\n';
    }
    out<<"SERVICES "<<d.services.size()<<'\n';
    for(const auto& service:d.services) out<<service.id<<' '<<quoted(service.name)<<' '<<service.priceCents<<'\n';
    out<<"VISIT_SERVICES "<<d.visitServices.size()<<'\n';
    for(const auto& order:d.visitServices) out<<order.id<<' '<<order.appointmentID<<' '<<order.serviceID<<' '<<order.quantity<<' '<<quoted(order.description)<<' '<<order.unitPriceCents<<'\n';
    out << "AUDIT " << saved.audit.size() << '\n';
    for (const auto& r : saved.audit) out << quoted(r.timestamp) << ' ' << quoted(r.action) << '\n';
    out << "END\n"; out.flush();
    if (!out) throw runtime_error("Autosave write failed; previous save retained.");
    out.close();
    if (!out) throw runtime_error("Autosave close failed; previous save retained.");
    // Keep the previous complete state as a recoverable backup before replacement.
    if (filesystem::exists(target))
        filesystem::copy_file(target,directory/(filename.string()+".bak"),filesystem::copy_options::overwrite_existing);
#ifdef _WIN32
    if (!MoveFileExW(temp.c_str(),target.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
        throw runtime_error("Cannot replace autosave; previous save retained.");
#else
    filesystem::rename(temp,target);
#endif
}
