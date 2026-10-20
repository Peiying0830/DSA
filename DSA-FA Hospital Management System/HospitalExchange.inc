#include <tuple>
using namespace std;

// Included once by HospitalSystem.cpp: transactional CSV upsert and data tools.
namespace {
int exchangeNumber(string value, int minimum, int maximum, char prefix=0) {
    if (prefix && !value.empty() && (value[0]==prefix || value[0]==static_cast<char>(tolower(prefix)))) value.erase(0,1);
    if (value.empty() || value.size()>7 || !all_of(value.begin(),value.end(),[](char c){return c>='0' && c<='9';})) throw runtime_error("Invalid number or ID");
    int result=stoi(value);
    if (result<minimum || result>maximum) throw runtime_error("Number or ID outside allowed range");
    return result;
}
vector<vector<string>> exchangeRows(const filesystem::path& path, const vector<string>& header) {
    ifstream file(path);
    if (!file) throw runtime_error("Cannot open " + path.filename().string());
    vector<vector<string>> rows;
    string line; size_t number=0; bool seenHeader=false; size_t expectedColumns=header.size();
    while (getline(file,line)) {
        ++number;
        if (number==1 && line.rfind("\xEF\xBB\xBF",0)==0) line.erase(0,3);
        if (CsvReader::trim(line).empty()) continue;
        try {
            auto fields=CsvReader::fields(line);
            if (!seenHeader) {
                auto extended=header;extended.push_back("consultation_fee");
                if(fields!=header && !(header.size()==6 && header[0]=="id" && header[3]=="day" && fields==extended)) throw runtime_error("Unexpected CSV header");
                expectedColumns=fields.size();seenHeader=true;continue;
            }
            if (fields.size()!=expectedColumns) throw runtime_error("Wrong column count");
            if (rows.size()>=100000) throw runtime_error("Too many rows");
            rows.push_back(fields);
        } catch (const exception& error) {throw runtime_error(path.filename().string()+" line "+to_string(number)+": "+error.what());}
    }
    if (!seenHeader || file.bad()) throw runtime_error("Empty or unreadable " + path.filename().string());
    return rows;
}
filesystem::path exchangePath(const string& prompt) {
    string value=Input::text(prompt);
    if(value.size()>=2 && value.front()=='"' && value.back()=='"') value=value.substr(1,value.size()-2);
    return filesystem::u8path(value);
}
void exchangeText(const string& value, size_t limit) {
    if (!Input::validText(value,limit)) throw runtime_error("Blank, invalid or overlong text");
}
void exchangeName(const string& value) {
    exchangeText(value,100);bool letter=false;
    for(unsigned char c:value) {
        if(c>='0' && c<='9') throw runtime_error("Names cannot contain digits");
        letter|=(c>='A' && c<='Z') || (c>='a' && c<='z') || c>=128;
    }
    if(!letter) throw runtime_error("Name must contain letters");
}
}
void HospitalSystem::loadDataBundle(int category) {
    auto source=exchangePath(category==0 ? "Data folder (e.g. import-data): " : "CSV file or folder (e.g. import-data): ");
    auto filePath=[&](const string& name) {
        return category!=0 && source.extension()==".csv" ? source : source/name;
    };
    HospitalData proposed=state;
    int patientCounter=nextPatientID,doctorCounter=nextDoctorID;
    size_t patientRows=0,doctorRows=0,medicineRows=0;
    try {
        vector<vector<string>> patients,doctors,medicines,services;
        if(category==0 || category==2) patients=exchangeRows(filePath("patients.csv"),{"id","name","age","gender","department","condition"});
        if(category==0 || category==1) doctors=exchangeRows(filePath("doctors.csv"),{"id","name","department","day","start","end"});
        if(category==0 || category==3) medicines=exchangeRows(filePath("medicines.csv"),{"id","name","unit","stock","reorder_level","price"});
        if(category==4 || (category==0 && filesystem::exists(filePath("services.csv")))) services=exchangeRows(filePath("services.csv"),{"id","name","price"});
        set<int> patientIDs,doctorIDs,drugIDs;
        map<int,Doctor> importedDoctors;
        vector<DoctorShift> importedShifts;
        for(const auto& f:patients) {
            Patient patient;patient.id=exchangeNumber(f[0],1,999999,'P');patient.name=f[1];patient.age=exchangeNumber(f[2],0,130);patient.gender=f[3];patient.department=f[4];patient.condition=f[5];
            exchangeName(patient.name);exchangeText(patient.department,100);exchangeText(patient.condition,500);
            if(patient.gender!="Female" && patient.gender!="Male" && patient.gender!="Other/unspecified") throw runtime_error("Invalid patient gender");
            if(!patientIDs.insert(patient.id).second) throw runtime_error("Repeated patient ID in CSV");
            int position=-1;
            for(int i=0;i<proposed.count;++i) if(proposed.patients[i].id==patient.id) position=i;
            if(position<0) {if(proposed.count>=capacity) throw runtime_error("Patient capacity exceeded");position=proposed.count++;}
            proposed.patients[position]=patient;patientCounter=max(patientCounter,patient.id+1);++patientRows;
        }
        for(const auto& f:doctors) {
            Doctor doctor{exchangeNumber(f[0],1,999999,'D'),f[1],f[2]};exchangeName(doctor.name);exchangeText(doctor.department,100);
            for(const auto& existingDoctor:state.doctors) if(existingDoctor.id==doctor.id) doctor.consultationFeeCents=existingDoctor.consultationFeeCents;
            if(f.size()==7) {
                if(f[6]=="0" || f[6]=="0.00") doctor.consultationFeeCents=0;
                else if(!BillingSystem::parsePrice(f[6],doctor.consultationFeeCents)) throw runtime_error("Invalid consultation fee");
            }
            auto existing=importedDoctors.find(doctor.id);
            if(existing!=importedDoctors.end() && (existing->second.name!=doctor.name || existing->second.department!=doctor.department || existing->second.consultationFeeCents!=doctor.consultationFeeCents)) throw runtime_error("Conflicting doctor details for the same ID");
            importedDoctors[doctor.id]=doctor;doctorIDs.insert(doctor.id);doctorCounter=max(doctorCounter,doctor.id+1);
            if(f[3].empty() && f[4].empty() && f[5].empty()) continue;
            if(!ScheduleBook::validShiftDay(f[3]) || ScheduleBook::slot(f[4])<0 || ScheduleBook::slot(f[5],true)<=ScheduleBook::slot(f[4])) throw runtime_error("Invalid doctor shift");
            for(const auto& shift:importedShifts)
                if(shift.doctorID==doctor.id && shift.date==f[3] && ScheduleBook::slot(f[4])<ScheduleBook::slot(shift.end,true) && ScheduleBook::slot(f[5],true)>ScheduleBook::slot(shift.start)) throw runtime_error("Duplicate or overlapping shifts");
            importedShifts.push_back({doctor.id,f[3],f[4],f[5]});
        }
        for(const auto& entry:importedDoctors) {
            bool replaced=false;
            for(auto& doctor:proposed.doctors) if(doctor.id==entry.first) {doctor=entry.second;replaced=true;}
            if(!replaced) {if(proposed.doctors.size()>=100000) throw runtime_error("Doctor capacity exceeded");proposed.doctors.push_back(entry.second);}
        }
        doctorRows=importedDoctors.size();
        proposed.shifts.erase(remove_if(proposed.shifts.begin(),proposed.shifts.end(),[&](const DoctorShift& shift){return doctorIDs.count(shift.doctorID)!=0;}),proposed.shifts.end());
        proposed.shifts.insert(proposed.shifts.end(),importedShifts.begin(),importedShifts.end());
        if(proposed.shifts.size()>100000) throw runtime_error("Shift capacity exceeded");
        for(const auto& appointment:proposed.appointments)
            if(ScheduleBook::active(appointment) && !ScheduleBook::working(proposed.shifts,appointment.doctorID,appointment.date,appointment.time)) throw runtime_error("New shifts do not cover active appointment #"+to_string(appointment.id));
        map<int,Drug> drugs;
        for(const auto& drug:proposed.pharmacy.records()) drugs[drug.id]=drug;
        for(const auto& f:medicines) {
            Drug drug;drug.id=exchangeNumber(f[0],1,999999,'M');drug.name=f[1];drug.unit=f[2];drug.stock=exchangeNumber(f[3],0,1000000);drug.reorderLevel=exchangeNumber(f[4],0,1000000);
            exchangeText(drug.name,100);exchangeText(drug.unit,50);
            if(!BillingSystem::parsePrice(f[5],drug.priceCents) || !drugIDs.insert(drug.id).second) throw runtime_error("Invalid price or repeated medicine ID");
            drugs[drug.id]=drug;++medicineRows;
        }
        if(drugs.size()>100000) throw runtime_error("Medicine capacity exceeded");
        proposed.pharmacy=PharmacyInventory{};
        for(const auto& entry:drugs) if(!proposed.pharmacy.add(entry.second)) throw runtime_error("Invalid medicine");
        set<int> seenServices;
        for(const auto& f:services) {
            HospitalService service{exchangeNumber(f[0],1,999999,'S'),f[1],0};exchangeText(service.name,100);
            if(!seenServices.insert(service.id).second || !BillingSystem::parsePrice(f[2],service.priceCents)) throw runtime_error("Invalid or duplicate service");
            bool updated=false;for(auto& existing:proposed.services) if(existing.id==service.id) {existing=service;updated=true;}
            if(!updated) proposed.services.push_back(service);
        }
        auto waiting=proposed.serviceQueue.records();proposed.serviceQueue=ServiceQueue{};
        for(auto record:waiting) {
            for(int i=0;i<proposed.count;++i) if(proposed.patients[i].id==record.patientID) {record.name=proposed.patients[i].name;record.department=proposed.patients[i].department;}
            proposed.serviceQueue.enqueue(record);
        }
    } catch(const exception& error) {ConsoleUI::message("!",string(error.what())+". Nothing loaded.");return;}
    ConsoleUI::message("INFO","Only selected categories are loaded. Matching IDs update; medicine stock and listed doctor shifts are replaced.");
    if(Input::number("Apply selected data? 1. Yes 2. No: ",1,2)==2) return;
    remember("Load/update all data from CSV");
    state=move(proposed);int oldPatient=nextPatientID,oldDoctor=nextDoctorID;nextPatientID=patientCounter;nextDoctorID=doctorCounter;rebuildIndex();
    if(!commitChange()) {nextPatientID=oldPatient;nextDoctorID=oldDoctor;return;}
    ConsoleUI::message("OK","Loaded/updated "+to_string(doctorRows)+" doctors, "+to_string(patientRows)+" patients and "+to_string(medicineRows)+" medicines.");
}
void HospitalSystem::exportData() const {
    auto target=exchangePath("Backup file (e.g. hospital-backup.db): ");
    if(filesystem::exists(target)) {ConsoleUI::message("!","File already exists. Choose a new backup filename.");return;}
    try {
        auto parent=target.parent_path();if(parent.empty()) parent=".";
        ostringstream report;
        auto rule=[&] {report<<'+'<<string(74,'-')<<"+\n";};
        auto row=[&](const string& text) {
            istringstream lines(text);string value;
            do {
                if(!getline(lines,value)) value.clear();
                string part;
                auto emit=[&] {report<<"| "<<part<<string(72-ConsoleUI::displayWidth(part),' ')<<" |\n";part.clear();};
                for(size_t i=0;i<value.size();) {
                    unsigned char c=static_cast<unsigned char>(value[i]);
                    size_t n=c<128?1:(c&0xe0)==0xc0?2:(c&0xf0)==0xe0?3:4;
                    n=min(n,value.size()-i);auto ch=value.substr(i,n);
                    if(ConsoleUI::displayWidth(part+ch)>72) emit();
                    part+=ch;i+=n;
                }
                emit();
            } while(lines.peek()!=char_traits<char>::eof());
        };
        auto heading=[&](const string& title) {rule();row(title);rule();};
        auto money=[](long long cents) {return BillingSystem::money(cents);};
        auto doctorName=[&](int id) {for(const auto& d:state.doctors) if(d.id==id) return formatDoctorID(id)+" ("+d.name+")";return formatDoctorID(id);};
        heading("HOSPITAL COMPLETE BACKUP");row("Exported: "+Persistence::timestamp());
        row("All records are included. Restore this same file using Restore backup.");rule();
        heading("PATIENTS / APPOINTMENTS / MEDICAL RECORDS");
        if(!state.count) {row("No patients.");rule();}
        for(int i=0;i<state.count;++i) {
            const auto& p=state.patients[i];
            row(formatPatientID(p.id)+" | "+p.name);row("Age: "+to_string(p.age)+" | Gender: "+p.gender);
            row("Department: "+p.department);row("Medical condition: "+p.condition);rule();
            row("Appointments for "+formatPatientID(p.id));
            vector<Appointment> appointments;
            for(const auto& a:state.appointments) if(a.patientID==p.id) appointments.push_back(a);
            sort(appointments.begin(),appointments.end(),[](const Appointment& a,const Appointment& b){return tie(a.date,a.time,a.id)<tie(b.date,b.time,b.id);});
            if(appointments.empty()) {row("No appointments.");rule();}
            for(const auto& a:appointments) {
                row(formatID("A",a.id)+" | "+a.date+" "+a.time);row("Patient: "+formatPatientID(p.id));
                row("Doctor: "+doctorName(a.doctorID));row("Reason: "+a.reason);row("Status: "+a.status);rule();
            }
            row("Medical records for "+formatPatientID(p.id));
            auto history=state.history.records(p.id);
            if(history.empty()) {row("No medical records.");rule();}
            for(const auto& r:history) {row(formatID("R",r.id)+" | "+r.date+" | "+r.type);if(r.appointmentID) row("Appointment: "+formatID("A",r.appointmentID));row(r.details);rule();}
        }
        heading("DOCTORS & SHIFTS");
        if(state.doctors.empty()) {row("No doctors.");rule();}
        for(const auto& d:state.doctors) {
            row(doctorName(d.id));row("Department: "+d.department);row("Consultation fee: "+money(d.consultationFeeCents));
            bool any=false;for(const auto& shift:state.shifts) if(shift.doctorID==d.id) {row("Shift: "+shift.date+" "+shift.start+" - "+shift.end);any=true;}
            if(!any) row("No shifts assigned.");
            rule();
        }
        heading("MEDICINES");
        if(state.pharmacy.records().empty()) {row("No medicines.");rule();}
        for(const auto& m:state.pharmacy.records()) {row(formatID("M",m.id)+" | "+m.name);row("Stock: "+to_string(m.stock)+" "+m.unit);row("Unit price: "+money(m.priceCents));row("Low-stock threshold: "+to_string(m.reorderLevel));rule();}
        heading("PRESCRIPTIONS");
        if(state.prescriptions.records().empty()) {row("No prescriptions.");rule();}
        for(const auto& r:state.prescriptions.records()) {
            row(formatID("RX",r.id)+" | "+r.date+" | "+r.status);row("Patient: "+formatPatientID(r.patientID));row("Doctor: "+doctorName(r.doctorID));
            if(r.appointmentID) row("Appointment: "+formatID("A",r.appointmentID));
            if(r.invoiceID) row("Invoice: "+formatID("I",r.invoiceID));
            for(const auto& item:r.items) {row("Medicine: "+formatID("M",item.drugID)+" | Quantity: "+to_string(item.quantity));row("Instructions: "+item.instructions);}
            rule();
        }
        heading("INVOICES & PAYMENTS");
        if(state.billing.records().empty()) {row("No invoices.");rule();}
        for(const auto& inv:state.billing.records()) {
            row(formatID("I",inv.id)+" | "+inv.createdAt+" | "+inv.status);row("Patient: "+formatPatientID(inv.patientID));
            if(inv.appointmentID) row("Appointment: "+formatID("A",inv.appointmentID));
            if(inv.prescriptionID) row("Prescription: "+formatID("RX",inv.prescriptionID));
            for(const auto& item:inv.items) row(item.description+" | Qty: "+to_string(item.quantity)+" | Unit: "+money(item.unitPriceCents)+" | Amount: "+money(item.quantity*item.unitPriceCents));
            row("Total: "+money(BillingSystem::total(inv)));
            if(!inv.paidAt.empty()) row("Paid: "+inv.paidAt+" | Method: "+inv.paymentMethod);
            rule();
        }
        heading("SERVICES & VISIT CHARGES");
        for(const auto& service:state.services) {row(formatID("S",service.id)+" | "+service.name);row("Price: "+money(service.priceCents));rule();}
        for(const auto& v:state.visitServices) {row(formatID("V",v.id)+" | Appointment: "+formatID("A",v.appointmentID));row(formatID("S",v.serviceID)+" | "+v.description);row("Quantity: "+to_string(v.quantity)+" | Unit price: "+money(v.unitPriceCents));rule();}
        heading("WAITING QUEUES");
        for(const auto& q:state.serviceQueue.records()) {row("Normal: "+formatPatientID(q.patientID)+" | "+q.name);row(q.department+" | "+q.service+" | "+q.status);rule();}
        for(const auto& q:state.emergency.records()) {row("Emergency: "+formatPatientID(q.patientID)+" | Priority: "+to_string(q.priority));row("Arrival sequence: "+to_string(q.sequence)+" | "+q.reason);rule();}
        heading("OPERATION LOG");
        if(audit.empty()) row("No recorded operations.");
        for(const auto& entry:audit) row(entry.timestamp+" | "+entry.action);
        rule();
        heading("SYSTEM RESTORE DATA - KEEP THE FOLLOWING SECTION UNCHANGED");
        Persistence(parent,target.filename()).save(snapshot(),report.str());
        ConsoleUI::message("OK","Complete backup exported to "+target.string());
    } catch(const exception& error) {ConsoleUI::message("!",string("Backup export failed: ")+error.what());}
}
void HospitalSystem::restoreBackup() {
    auto source=exchangePath("Backup file to restore (e.g. hospital-backup.db): ");
    try {
        auto parent=source.parent_path();if(parent.empty()) parent=".";
        auto saved=Persistence(parent,source.filename()).load();
        ConsoleUI::panel([&](ostream& out) {
            out<<"Backup: "<<source.string()<<"\nPatients: "<<saved.data.count
               <<" | Doctors: "<<saved.data.doctors.size()<<" | Appointments: "<<saved.data.appointments.size()
               <<"\nThis replaces all current hospital records with the backup.\n";
        });
        if(Input::number("Restore backup? 1. Yes 2. No: ",1,2)!=1) return;
        // Keep counters high enough for Undo to restore the previous hospital safely.
        saved.nextPatientID=max(saved.nextPatientID,nextPatientID);
        saved.nextDoctorID=max(saved.nextDoctorID,nextDoctorID);
        saved.nextAppointmentID=max(saved.nextAppointmentID,nextAppointmentID);
        saved.nextRecordID=max(saved.nextRecordID,nextRecordID);
        saved.nextPrescriptionID=max(saved.nextPrescriptionID,nextPrescriptionID);
        saved.nextInvoiceID=max(saved.nextInvoiceID,nextInvoiceID);
        saved.nextServiceOrderID=max(saved.nextServiceOrderID,nextServiceOrderID);
        saved.nextEmergencySequence=max(saved.nextEmergencySequence,nextEmergencySequence);
        saved.audit.push_back({Persistence::timestamp(),"Restore complete backup"});
        // Validate and save before changing live state; a failed restore changes nothing.
        storage.save(saved);
        undo.push({state,"Restore complete backup"});
        state=move(saved.data);audit=move(saved.audit);
        nextPatientID=saved.nextPatientID;nextDoctorID=saved.nextDoctorID;
        nextAppointmentID=saved.nextAppointmentID;nextRecordID=saved.nextRecordID;
        nextPrescriptionID=saved.nextPrescriptionID;nextInvoiceID=saved.nextInvoiceID;
        nextServiceOrderID=saved.nextServiceOrderID;nextEmergencySequence=saved.nextEmergencySequence;
        rebuildIndex();
        ConsoleUI::message("OK","Complete backup restored and autosaved.");
    } catch(const InputCancelled&) { throw; }
    catch(const exception& error) {ConsoleUI::message("!",string("Backup restore failed: ")+error.what());}
}
void HospitalSystem::clearData() {
    ConsoleUI::message("!","Reset to the default demo data: P001-P010, D001-D010 and M001-M010. Appointments, queues, history, prescriptions, invoices and prior changes will be removed.");
    if(Input::text("Type CLEAR to confirm (0 cancels): ")!="CLEAR") {ConsoleUI::message("INFO","Clear cancelled.");return;}
    SavedHospital previous=snapshot();
    loadDefaultData();
    audit={{Persistence::timestamp(),"Reset hospital to default demo data"}};
    try {
        storage.save(snapshot());
        undo.clear();
        ConsoleUI::savedNoticePending=true;
        ConsoleUI::message("OK","Default data restored. The next new patient ID is P011; next appointment: A001.");
    } catch(const exception& error) {
        state=move(previous.data); audit=move(previous.audit);
        nextAppointmentID=previous.nextAppointmentID; nextRecordID=previous.nextRecordID;
        nextEmergencySequence=previous.nextEmergencySequence; nextPrescriptionID=previous.nextPrescriptionID;
        nextInvoiceID=previous.nextInvoiceID; nextPatientID=previous.nextPatientID;
        nextDoctorID=previous.nextDoctorID; nextServiceOrderID=previous.nextServiceOrderID;
        rebuildIndex();
        ConsoleUI::message("!",string("Reset failed; previous data was kept: ")+error.what());
    }
}
