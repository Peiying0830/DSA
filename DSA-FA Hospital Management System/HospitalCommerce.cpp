// HospitalCommerce.cpp
// Pharmacy, prescription dispensing and billing workflows.
// DSA Final Assessment - Hospital Management System.

#include "HospitalSystem.h"
#include "ConsoleUI.h"
#include <algorithm>
#include <iostream>
#include <set>
#include <fstream>
#include <sstream>
#include <cctype>
using namespace std;
namespace {
// Retry until the user enters a valid exact-cent selling price or service fee.
long long readPrice(const string& prompt) {
    for (;;) {
        long long cents=0;
        if (BillingSystem::parsePrice(Input::text(prompt),cents)) return cents;
        ConsoleUI::panel([&](ostream& out) { out << "Enter RM 0.01 to RM 1000000.00, with at most two decimal places.\n"; });
    }
}
}
// Display all prescriptions or only those belonging to a selected patient.
void HospitalSystem::showPrescriptions(int patientID) const {
    bool found=false;
    for (const auto& prescription : state.prescriptions.records()) {
        if (patientID && prescription.patientID!=patientID) continue;
        found=true;
        ConsoleUI::RecordBox frame;
        ConsoleUI::row("Prescription " + formatID("RX",prescription.id));
        ConsoleUI::field("Patient", formatPatientID(prescription.patientID));
        ConsoleUI::field("Doctor", formatDoctorID(prescription.doctorID));
        ConsoleUI::field("Date",prescription.date);
        if(prescription.appointmentID) ConsoleUI::field("Visit",formatID("A",prescription.appointmentID));
        if(prescription.invoiceID) ConsoleUI::field("Invoice",formatID("I",prescription.invoiceID));
        ConsoleUI::field("Status",prescription.status);
        for (const auto& item : prescription.items) {
            const auto* drug=state.pharmacy.find(item.drugID);
            ConsoleUI::row(string(60,'-'));
            ConsoleUI::field("Medicine",formatID("M",drug->id) + " | " + drug->name);
            ConsoleUI::field("Quantity",to_string(item.quantity) + " " + drug->unit);
            ConsoleUI::field("Instructions",item.instructions);
        }
    }
    if (!found) ConsoleUI::message("INFO", "No prescriptions found.");
}
// Read staff-entered medicines/quantities/instructions; do not reserve stock yet.
void HospitalSystem::createPrescription() {
    if (nextPrescriptionID>999999) { ConsoleUI::message("!", "Prescription ID limit reached."); return; }
    if (state.doctors.empty()) { ConsoleUI::message("!", "Register a doctor first."); return; }
    if (state.pharmacy.records().empty()) { ConsoleUI::message("!", "Register medicines first."); return; }
    vector<Appointment> visits;
    for(const auto& appointment:state.appointments) if(appointment.status=="Checked-in") visits.push_back(appointment);
    if(visits.empty()) { ConsoleUI::message("!","No checked-in visits. Check in an appointment before creating a prescription."); return; }
    sort(visits.begin(),visits.end(),[](const Appointment& left,const Appointment& right) {
        if(left.date!=right.date) return left.date<right.date;
        if(left.time!=right.time) return left.time<right.time;
        return left.id<right.id;
    });
    ConsoleUI::banner("SELECT CHECKED-IN VISIT");
    for(const auto& visit:visits) printAppointment(visit);
    int appointmentID=Input::entityID("Appointment ID for prescription: ","A");
    auto selectedVisit=find_if(visits.begin(),visits.end(),[&](const Appointment& appointment){return appointment.id==appointmentID;});
    if(selectedVisit==visits.end()) { ConsoleUI::message("!","Select one of the checked-in appointments shown above."); return; }
    Prescription prescription;
    prescription.id=nextPrescriptionID;
    prescription.appointmentID=appointmentID;
    prescription.patientID=selectedVisit->patientID;
    prescription.doctorID=selectedVisit->doctorID;
    prescription.date=selectedVisit->date;
    int maximum = min(20, static_cast<int>(state.pharmacy.records().size()));
    int count=Input::number("Number of different medicines (1-" + to_string(maximum) + "): ",1,maximum);
    set<int> selected;
    for (int i=0;i<count;++i) {
        int id=Input::entityID("Drug ID: ", "M");
        const auto* drug=state.pharmacy.find(id);
        if (!drug) { ConsoleUI::message("!", "Drug not found."); --i; continue; }
        if (!selected.insert(id).second) { ConsoleUI::message("!", "Drug already included; enter another drug."); --i; continue; }
        ConsoleUI::panel([&](ostream& out) { out << drug->name << " | Current stock: " << drug->stock << ' ' << drug->unit << '\n'; });
        int quantity=Input::number("Quantity: ",1,100000);
        string instructions=Input::text("Instructions entered by prescribing staff: ");
        prescription.items.push_back({id,quantity,instructions});
    }
    remember("Create prescription "+to_string(prescription.id)+" for appointment "+to_string(appointmentID));
    state.prescriptions.add(prescription);
    ++nextPrescriptionID;
    if (commitChange()) {
        ConsoleUI::panel([&](ostream& out) { out << "Prescription created. ID: " << formatID("RX",prescription.id) << " | Pending; stock is checked when dispensing.\n"; });
        if(Input::number("Dispense this prescription now? 1. Yes 2. Later: ",1,2)==1)
            dispensePrescription(prescription.id);
        if(Input::number("Add a service for this visit? 1. Yes 2. No: ",1,2)==1)
            addVisitService(appointmentID);
    }
}
// Validate all stock first, then consume it and create one invoice in a saved transaction.
void HospitalSystem::dispensePrescription(int prescriptionID) {
    int id=prescriptionID ? prescriptionID : Input::entityID("Prescription ID: ", "RX");
    const auto* found=state.prescriptions.find(id);
    if (!found) { ConsoleUI::message("!", "Prescription not found."); return; }
    if (found->status!="Pending") { ConsoleUI::message("!", "Only pending prescriptions can be dispensed."); return; }
    Prescription prescription=*found;
    // Older save files did not retain the appointment on prescriptions. Ask explicitly.
    if(!prescription.appointmentID) {
        vector<Appointment> candidates;
        for(const auto& visit:state.appointments)
            if(visit.patientID==prescription.patientID && visit.doctorID==prescription.doctorID && visit.date==prescription.date &&
               (visit.status=="Checked-in" || visit.status=="Completed")) candidates.push_back(visit);
        if(!candidates.empty()) {
            ConsoleUI::message("INFO","Select the visit for this prescription from an older save.");
            for(const auto& visit:candidates) printAppointment(visit);
            int selected=Input::entityID("Appointment ID for this prescription: ","A");
            if(none_of(candidates.begin(),candidates.end(),[&](const Appointment& visit){return visit.id==selected;})) {
                ConsoleUI::message("!","Select one of the visits shown. Nothing dispensed.");return;
            }
            prescription.appointmentID=selected;
        }
    }
    Invoice* visitInvoice=nullptr;
    for(const auto& bill:state.billing.records())
        if(prescription.appointmentID && bill.appointmentID==prescription.appointmentID && bill.status=="Unpaid") {
            visitInvoice=state.billing.findMutable(bill.id); break;
        }
    if(!visitInvoice && nextInvoiceID>999999) {ConsoleUI::message("!","Invoice ID limit reached.");return;}
    Invoice invoice;
    invoice.id=nextInvoiceID; invoice.patientID=prescription.patientID; invoice.prescriptionID=id;
    invoice.createdAt=Persistence::timestamp();
    // Validate ALL quantities before consuming any item: no partial dispensing.
    for (const auto& item : prescription.items) {
        const auto* drug=state.pharmacy.find(item.drugID);
        if (!drug) { ConsoleUI::message("OK", "Prescription refers to an unregistered medicine. Nothing dispensed."); return; }
        if (drug->stock<item.quantity) {
            ConsoleUI::panel([&](ostream& out) { out << "Insufficient stock for " << drug->name << ": need " << item.quantity
                 << ", available " << drug->stock << ". Nothing dispensed.\n"; });
            return;
        }
        invoice.items.push_back({drug->name+" ("+drug->unit+")",item.quantity,drug->priceCents});
    }
    Invoice combined=visitInvoice ? *visitInvoice : invoice;
    const int firstItem=visitInvoice ? static_cast<int>(combined.items.size()) : 0;
    if(visitInvoice) combined.items.insert(combined.items.end(),invoice.items.begin(),invoice.items.end());
    if(combined.items.size()>20 || BillingSystem::total(combined)>BillingSystem::maxInvoiceCents) {
        ConsoleUI::message("!","Invoice item/amount limit exceeded. Nothing dispensed."); return;
    }
    ConsoleUI::message("INFO","Medication charge: "+BillingSystem::money(BillingSystem::total(invoice)));
    if(visitInvoice) {
        ConsoleUI::message("INFO","Medication will be added to visit invoice "+formatID("I",combined.id)+".");
        showInvoice(combined);
    } else if(prescription.appointmentID) {
        ConsoleUI::message("INFO","No unpaid visit invoice is available; a separate medication invoice will be created.");
    }
    if (Input::number("Confirm dispensing and invoice? 1. Yes 2. No: ",1,2)==2) return;
    remember("Dispense prescription "+to_string(id)+"; "+(visitInvoice ? "update" : "create")+" invoice "+to_string(combined.id));
    if(!visitInvoice && !state.billing.add(combined)) {
        state=move(undo.top().previous);undo.pop();pendingAction.clear();
        ConsoleUI::message("!","Invoice could not be created. Nothing dispensed.");return;
    }
    if(visitInvoice) *visitInvoice=combined;
    for (const auto& item : prescription.items) state.pharmacy.consume(item.drugID,item.quantity);
    state.prescriptions.markDispensed(id,combined.id,firstItem,prescription.appointmentID);
    if(!visitInvoice) ++nextInvoiceID;
    if (commitChange()) {
        ConsoleUI::message("OK","Prescription dispensed. Invoice "+formatID("I",combined.id)+(visitInvoice ? " updated (Unpaid)." : " created (Unpaid)."));
        showInvoice(combined);
    }
}
// Import a validated batch without replacing existing stock or prices.
void HospitalSystem::importMedicines() {
    string path=Input::text("Medicine CSV path (e.g. sample-medicines.csv): ");
    if (path.size()>=2 && path.front()=='"' && path.back()=='"') path=path.substr(1,path.size()-2);
    ifstream file{filesystem::u8path(path)};
    if (!file) { ConsoleUI::message("!", "Cannot open medicine CSV file."); return; }
    auto trim=[](const string& value) {
        auto first=value.find_first_not_of(" \t\r");
        return first==string::npos ? string{} : value.substr(first,value.find_last_not_of(" \t\r")-first+1);
    };
    auto normalized=[](string value) {
        transform(value.begin(),value.end(),value.begin(),[](unsigned char c){return static_cast<char>(tolower(c));});
        return value;
    };
    PharmacyInventory proposed=state.pharmacy;
    set<pair<string,string>> known;
    int nextID=1;
    for (const auto& drug : proposed.records()) {
        nextID=max(nextID,drug.id+1);
        known.insert({normalized(trim(drug.name)),normalized(trim(drug.unit))});
    }
    size_t lineNumber=0,added=0,skipped=0;
    string line;
    try {
        while (getline(file,line)) {
            ++lineNumber;
            if (lineNumber==1 && line.rfind("\xEF\xBB\xBF",0)==0) line.erase(0,3);
            if (trim(line).empty()) continue;
            if (line.size()>1000) throw runtime_error("Line too long");
            vector<string> fields;
            size_t pos=0;
            for (;;) {
                while (pos<line.size() && line[pos]==' ') ++pos;
                string value;
                if (pos<line.size() && line[pos]=='"') {
                    ++pos; bool closed=false;
                    while (pos<line.size()) {
                        char c=line[pos++];
                        if (c!='"') value+=c;
                        else if (pos<line.size() && line[pos]=='"') {value+='"';++pos;}
                        else {closed=true;break;}
                    }
                    if (!closed) throw runtime_error("Unclosed CSV quote");
                    while (pos<line.size() && (line[pos]==' ' || line[pos]=='\r')) ++pos;
                    if (pos<line.size() && line[pos]!=',') throw runtime_error("Invalid quoted field");
                } else {
                    while (pos<line.size() && line[pos]!=',') {
                        if (line[pos]=='"') throw runtime_error("Invalid CSV quote");
                        value+=line[pos++];
                    }
                }
                fields.push_back(trim(value));
                if (pos==line.size()) break;
                ++pos;
            }
            if (fields.size()!=5) throw runtime_error("Expected name,unit,stock,reorder_level,price columns");
            if (lineNumber==1 && fields==vector<string>{"name","unit","stock","reorder_level","price"}) continue;
            auto quantity=[](const string& value) {
                if (value.empty() || value.size()>7 || !all_of(value.begin(),value.end(),[](char c){return c>='0' && c<='9';}))
                    throw runtime_error("Invalid stock or threshold");
                int number=stoi(value);
                if (number>1000000) throw runtime_error("Stock or threshold exceeds 1000000");
                return number;
            };
            Drug drug;
            drug.id=nextID; drug.name=fields[0]; drug.unit=fields[1];
            drug.stock=quantity(fields[2]); drug.reorderLevel=quantity(fields[3]);
            if (!Input::validText(drug.name,100) || !Input::validText(drug.unit,50) || !BillingSystem::parsePrice(fields[4],drug.priceCents))
                throw runtime_error("Invalid medicine name, unit or price");
            if (!known.insert({normalized(drug.name),normalized(drug.unit)}).second) {++skipped;continue;}
            if (proposed.records().size()>=100000 || !proposed.add(drug)) throw runtime_error("Medicine capacity or ID limit reached");
            ++nextID; ++added;
        }
        if (file.bad()) throw runtime_error("File read failed");
    } catch (const exception& error) {
        ConsoleUI::message("!", "CSV line " + to_string(lineNumber) + ": " + error.what() + ". Nothing imported."); return;
    }
    if (!added) {ConsoleUI::message("INFO", "No new medicines to import (empty file or duplicates).");return;}
    remember("Import " + to_string(added) + " medicines from CSV");
    state.pharmacy=move(proposed);
    if (commitChange()) ConsoleUI::message("OK", "Imported " + to_string(added) + " medicines; skipped " + to_string(skipped) + " duplicates. View Inventory for IDs.");
}

// Manage user-entered medicines, restocking, price updates and prescriptions.
void HospitalSystem::pharmacyMenu() {
    for (;;) {
        try {
            ConsoleUI::menu("PHARMACY & DISPENSING", {"1. Medicine management", "2. Dispense prescription", "3. Cancel pending prescription", "0. Back to main menu"});
            int choice=Input::number("Choose: ",0,3);
            if (!choice) throw MainMenuRequested{};
            if (choice==1) medicineMenu();
            else if(choice==2) dispensePrescription();
            else pharmacyAction(8);
        } catch (const InputCancelled&) { ConsoleUI::message("INFO", "Operation cancelled. Nothing saved."); }
    }
}
void HospitalSystem::medicineMenu() {
    for (;;) {
        try {
            ConsoleUI::menu("MANAGE MEDICINE", {"1. Add new medicine", "2. View all", "3. Update medicine", "4. Low-stock medicines", "0. Back to main menu"});
            int choice=Input::number("Choose: ",0,4);
            if (!choice) throw MainMenuRequested{};
            const int actions[]={1,2,3,9};
            pharmacyAction(actions[choice-1]);
        } catch (const InputCancelled&) { ConsoleUI::message("INFO", "Operation cancelled. Nothing saved."); }
    }
}
void HospitalSystem::prescriptionMenu() {
    for (;;) {
        try {
            ConsoleUI::menu("PRESCRIPTIONS", {"1. Create prescription for a visit", "2. View prescriptions", "0. Back to main menu"});
            int choice=Input::number("Choose: ",0,2);
            if (!choice) throw MainMenuRequested{};
            if(choice==1) createPrescription();
            else pharmacyAction(6);
        } catch (const InputCancelled&) { ConsoleUI::message("INFO", "Operation cancelled. Nothing saved."); }
    }
}
void HospitalSystem::pharmacyAction(int choice) {
        if (choice==1) {
            int id=1;
            for(const auto& existing:state.pharmacy.records()) id=max(id,existing.id+1);
            if(id>999999 || state.pharmacy.records().size()>=100000) {
                ConsoleUI::message("!","Medicine catalog limit reached.");return;
            }
            ConsoleUI::message("INFO","New medicine ID: "+formatID("M",id)+" (automatically assigned).");
            Drug drug;
            drug.id=id; drug.name=Input::text("Medicine name: ", 100); drug.unit=Input::text("Stock unit (e.g. tablet/bottle): ", 50);
            drug.stock=Input::number("Opening stock: ",0,1000000);
            drug.reorderLevel=Input::number("Low-stock threshold: ",0,1000000);
            drug.priceCents=readPrice("Unit selling price (RM): ");
            remember("Register drug "+to_string(id));
            state.pharmacy.add(drug);
            if (commitChange()) ConsoleUI::message("OK", "Medicine "+formatID("M",id)+" registered.");
        } else if (choice==2 || choice==9) {
            bool found=false;
            for (const auto& drug : state.pharmacy.records()) {
                if (choice==9 && drug.stock>drug.reorderLevel) continue;
                found=true;
                ConsoleUI::RecordBox frame;
                ConsoleUI::row("Medicine " + formatID("M",drug.id));
                ConsoleUI::field("Medicine", drug.name);
                ConsoleUI::field("Stock", to_string(drug.stock) + " " + drug.unit);
                ConsoleUI::field("Unit price", BillingSystem::money(drug.priceCents));
                ConsoleUI::field("Low-stock threshold", to_string(drug.reorderLevel));
                ConsoleUI::field("Stock status", drug.stock<=drug.reorderLevel ? "LOW STOCK" : "In stock");
            }
            if (!found) ConsoleUI::panel([&](ostream& out) { out << (choice==9 ? "No low-stock medicines.\n" : "No medicines registered.\n"); });
        } else if (choice==3) {
            pharmacyAction(2);
            int id=Input::entityID("Drug ID: ", "M");
            const auto* drug=state.pharmacy.find(id);
            if(!drug) {ConsoleUI::message("!","Drug not found.");return;}
            ConsoleUI::panel([&](ostream& out) {out << "Medicine " << formatID("M",drug->id) << " | " << drug->name << "\nStock: " << drug->stock << ' ' << drug->unit << "\nUnit price: " << BillingSystem::money(drug->priceCents) << '\n';});
            int quantity=0;
            for(;;) {
                string value=Input::text("Quantity to add (- to keep, 0 to cancel): ");
                if(value=="-") break;
                if(!value.empty() && value.size()<=7 && all_of(value.begin(),value.end(),[](char c){return c>='0' && c<='9';})) {
                    int parsed=stoi(value);
                    if(parsed>=1 && parsed<=1000000-drug->stock) {quantity=parsed;break;}
                }
                ConsoleUI::message("!","Enter a positive quantity within the stock limit, or - to keep current stock.");
            }
            long long price=drug->priceCents;
            for(;;) {
                string value=Input::text("New unit price (RM, - to keep, 0 to cancel): ");
                if(value=="-") break;
                if(BillingSystem::parsePrice(value,price)) break;
                ConsoleUI::message("!","Enter RM 0.01 to RM 1000000.00 with at most two decimals, or - to keep.");
                price=drug->priceCents;
            }
            if(!quantity && price==drug->priceCents) {ConsoleUI::message("INFO","No changes made.");return;}
            remember("Update medicine " + to_string(id));
            if(quantity) state.pharmacy.restock(id,quantity);
            state.pharmacy.setPrice(id,price);
            if(commitChange()) {
                ConsoleUI::RecordBox frame;
                ConsoleUI::message("OK","Medicine updated.");
                drug=state.pharmacy.find(id);
                ConsoleUI::row("Medicine " + formatID("M",id));
                ConsoleUI::field("Medicine",drug->name);
                ConsoleUI::field("Stock",to_string(drug->stock)+" "+drug->unit);
                ConsoleUI::field("Unit price",BillingSystem::money(drug->priceCents));
            }
        } else if (choice==5) createPrescription();
        else if (choice==6) showPrescriptions();
        else if (choice==7) dispensePrescription();
        else {
            int id=Input::entityID("Prescription ID: ", "RX");
            const auto* prescription=state.prescriptions.find(id);
            if (!prescription) { ConsoleUI::message("!", "Prescription not found."); return; }
            if (prescription->status!="Pending") { ConsoleUI::message("!", "Only pending prescriptions can be cancelled."); return; }
            if (Input::number("Confirm cancellation? 1. Yes 2. No: ",1,2)==2) return;
            remember("Cancel prescription "+to_string(id));
            state.prescriptions.setStatus(id,"Cancelled");
            if (commitChange()) ConsoleUI::message("OK", "Prescription cancelled; record retained.");
        }
}
// List invoices and sum unpaid balances for all patients or one selected patient.
void HospitalSystem::showInvoices(int patientID) const {
    bool found=false;
    long long outstanding=0;
    for (const auto& invoice : state.billing.records()) {
        if (patientID && invoice.patientID!=patientID) continue;
        found=true;
        long long total=BillingSystem::total(invoice);
        ConsoleUI::RecordBox frame;
        ConsoleUI::row("Invoice " + formatID("I",invoice.id) + " | Patient " + formatPatientID(invoice.patientID) + " | " + invoice.status + " | " + BillingSystem::money(total));
        if(invoice.appointmentID) ConsoleUI::field("Visit",formatID("A",invoice.appointmentID));
        if(invoice.prescriptionID) ConsoleUI::field("Prescription",formatID("RX",invoice.prescriptionID));
        for(const auto& rx:state.prescriptions.records()) {
            if(rx.invoiceID==invoice.id && rx.id!=invoice.prescriptionID)
                ConsoleUI::field("Prescription",formatID("RX",rx.id));
        }
        if (invoice.status=="Unpaid") outstanding+=total;
    }
    if (!found) ConsoleUI::message("INFO", "No invoices found.");
    ConsoleUI::panel([&](ostream& out) { out << "Outstanding balance: " << BillingSystem::money(outstanding) << '\n'; });
}
// Display fixed charge details; paid invoices are shown as payment receipts.
void HospitalSystem::showInvoice(const Invoice& invoice) const {
    ConsoleUI::RecordBox frame;
    ConsoleUI::panel([&](ostream& out) { out << (invoice.status=="Paid" ? "\n--- PAYMENT RECEIPT ---\n" : "\n--- UNPAID INVOICE ---\n")
         << "Invoice " << formatID("I",invoice.id) << " | Created: " << invoice.createdAt << '\n'; });
    if(invoice.appointmentID) ConsoleUI::field("Visit",formatID("A",invoice.appointmentID));
    const auto& patient=state.patients[patientPosition(invoice.patientID)];
    ConsoleUI::panel([&](ostream& out) { out << "Patient " << formatPatientID(patient.id) << " | " << patient.name << '\n'; });
    if (invoice.prescriptionID) ConsoleUI::panel([&](ostream& out) { out << "Prescription " << formatID("RX",invoice.prescriptionID) << '\n'; });
    for(const auto& rx:state.prescriptions.records()) {
        if(rx.invoiceID==invoice.id && rx.id!=invoice.prescriptionID)
            ConsoleUI::field("Prescription",formatID("RX",rx.id));
    }
    for (const auto& item : invoice.items)
        ConsoleUI::panel([&](ostream& out) { out << item.description << " | " << item.quantity << " x " << BillingSystem::money(item.unitPriceCents)
             << " = " << BillingSystem::money(item.quantity*item.unitPriceCents) << '\n'; });
    ConsoleUI::panel([&](ostream& out) { out << "Total: " << BillingSystem::money(BillingSystem::total(invoice)) << " | " << invoice.status << '\n'; });
    if (invoice.status=="Paid") ConsoleUI::panel([&](ostream& out) { out << "Recorded payment: " << invoice.paymentMethod << " | " << invoice.paidAt << '\n'; });
}
// Automatic invoices are generated at check-in and dispensing.
void HospitalSystem::billingMenu() {
    for(;;) {
        try {
            ConsoleUI::menu("BILLING & PAYMENTS", {"1. View invoices", "2. Record payment", "0. Back to main menu"});
            int choice=Input::number("Choose: ",0,2);
            if (!choice) throw MainMenuRequested{};
            if(choice==1) invoiceViewMenu();else payInvoice();
        } catch(const InputCancelled&) {ConsoleUI::message("INFO","Operation cancelled. Nothing saved.");}
    }
}
void HospitalSystem::invoiceViewMenu() {
    for(;;) {
        try {
            ConsoleUI::menu("VIEW INVOICES", {"1. All invoices", "2. By patient", "0. Back to main menu"});
            int choice=Input::number("Choose: ",0,2);
            if (!choice) throw MainMenuRequested{};
            int patientID=0;
            if(choice==2) {
                patientID=Input::patientID("Patient ID: ");
                if(!index.contains(patientID)) {ConsoleUI::message("!","Patient not found.");continue;}
            }
            showInvoices(patientID);
            bool any=false;for(const auto& invoice:state.billing.records()) if(!patientID || invoice.patientID==patientID) any=true;
            if(!any) continue;
            for(;;) {
                int id=Input::entityID("Invoice ID to view details (0 to return to main menu): ","I");
                const auto* invoice=state.billing.find(id);
                if(!invoice || (patientID && invoice->patientID!=patientID)) {ConsoleUI::message("!","Select an invoice from this list.");continue;}
                showInvoice(*invoice);
            }
        } catch(const InputCancelled&) { /* Cancel an input and return to invoice filters. */ }
    }
}
void HospitalSystem::payInvoice() {
    showInvoices();
    if(state.billing.records().empty()) return;
    int id=Input::entityID("Invoice ID: ","I");
    const auto* found=state.billing.find(id);
    if(!found) {ConsoleUI::message("!","Invoice not found.");return;}
    if(found->status=="Paid") {ConsoleUI::message("!","Invoice already paid.");return;}
    showInvoice(*found);
    int method=Input::number("Payment method: 1. Cash 2. Card 3. Transfer: ",1,3);
    const string methods[]={"Cash","Card","Transfer"};
    if(Input::number("Confirm full payment received? 1. Yes 2. No: ",1,2)==2) return;
    remember("Record payment for invoice "+to_string(id)+" via "+methods[method-1]);
    state.billing.pay(id,methods[method-1],Persistence::timestamp());
    if(commitChange()) showInvoice(*state.billing.find(id));
}
