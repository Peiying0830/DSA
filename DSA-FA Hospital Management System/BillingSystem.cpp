// BillingSystem.cpp
// Invoice snapshots and local payment records using exact integer cents.
// DSA Final Assessment - Hospital Management System.

#include "BillingSystem.h"
#include "Input.h"
#include <iomanip>
#include <sstream>
using namespace std;

// Parse at most two decimal places directly into cents; never use floating-point money.
bool BillingSystem::parsePrice(const string& input, long long& cents) {
    if (input.empty()) return false;
    auto point=input.find('.');
    string whole=input.substr(0,point), fraction=point==string::npos ? "" : input.substr(point+1);
    if (whole.empty() || whole.size()>7 || fraction.size()>2 || (point!=string::npos && fraction.empty())) return false;
    for (char c : whole+fraction) if (c<'0' || c>'9') return false;
    while (fraction.size()<2) fraction+='0';
    cents=stoll(whole)*100+stoll(fraction);
    return cents>=1 && cents<=100000000;
}
// Format integer cents as an RM amount with exactly two decimal places.
string BillingSystem::money(long long cents) {
    ostringstream out;
    out << "RM " << cents/100 << '.' << setfill('0') << setw(2) << cents%100;
    return out.str();
}
// Sum quantity times captured unit price for each invoice line.
long long BillingSystem::total(const Invoice& invoice) {
    long long sum=0;
    for (const auto& item : invoice.items) sum+=item.quantity*item.unitPriceCents;
    return sum;
}
// Validate line amounts/payment fields and prevent duplicate billing of a prescription.
bool BillingSystem::add(const Invoice& invoice) {
    if (invoice.id<1 || invoice.id>999999 || find(invoice.id) || invoice.items.empty() || invoice.items.size()>20 || invoice.createdAt.empty()) return false;
    if (invoice.patientID<1 || invoice.patientID>999999 || invoice.prescriptionID<0 || invoice.prescriptionID>999999) return false;
    if(invoice.appointmentID<0 || invoice.appointmentID>999999 || (invoice.appointmentID && invoice.prescriptionID)) return false;
    if(invoice.appointmentID) for(const auto& existing:invoices) if(existing.appointmentID==invoice.appointmentID) return false;
    if (invoice.status!="Unpaid" && invoice.status!="Paid") return false;
    if (invoice.status=="Paid" && (invoice.paidAt.empty() || (invoice.paymentMethod!="Cash" && invoice.paymentMethod!="Card" && invoice.paymentMethod!="Transfer"))) return false;
    if (invoice.status=="Unpaid" && (!invoice.paidAt.empty() || !invoice.paymentMethod.empty())) return false;
    for (const auto& item : invoice.items)
        if (!Input::validText(item.description) || item.quantity<1 || item.quantity>100000 || item.unitPriceCents<1 || item.unitPriceCents>100000000) return false;
    if (total(invoice)>maxInvoiceCents) return false;
    if (invoice.prescriptionID)
        for (const auto& existing : invoices) if (existing.prescriptionID==invoice.prescriptionID) return false;
    invoices.push_back(invoice); return true;
}
// Read an invoice by its persistent ID.
const Invoice* BillingSystem::find(int id) const {
    for (const auto& invoice : invoices) if (invoice.id==id) return &invoice;
    return nullptr;
}
Invoice* BillingSystem::findMutable(int id) {
    for (auto& invoice : invoices) if (invoice.id==id) return &invoice;
    return nullptr;
}
// Mark an Unpaid invoice Paid with a method/time; reject repeated payment recording.
bool BillingSystem::pay(int id, const string& method, const string& timestamp) {
    if (timestamp.empty() || (method!="Cash" && method!="Card" && method!="Transfer")) return false;
    for (auto& invoice : invoices)
        if (invoice.id==id && invoice.status=="Unpaid") {
            invoice.status="Paid"; invoice.paymentMethod=method; invoice.paidAt=timestamp; return true;
        }
    return false;
}
// Expose read-only invoices for balances, patient profiles and saving.
const vector<Invoice>& BillingSystem::records() const { return invoices; }
