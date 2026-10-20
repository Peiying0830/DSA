// BillingSystem.h
// Invoice snapshots and local payment records using exact integer cents.
// DSA Final Assessment - Hospital Management System.

#pragma once
#include <string>
#include <vector>
struct BillItem {
    std::string description;
    int quantity=0;
    long long unitPriceCents=0;
};
struct Invoice {
    int id=0, patientID=0, prescriptionID=0; // Zero means a manually entered service invoice.
    std::string createdAt, status="Unpaid", paidAt, paymentMethod;
    std::vector<BillItem> items;
    int appointmentID=0;
};
class BillingSystem {
    std::vector<Invoice> invoices;
public:
    static constexpr long long maxInvoiceCents=100000000; // RM 1,000,000 per invoice.
    // Parse at most two decimal places directly into cents; never use floating-point money.
    static bool parsePrice(const std::string& input, long long& cents);
    // Format integer cents as an RM amount with exactly two decimal places.
    static std::string money(long long cents);
    // Sum quantity times captured unit price for each invoice line.
    static long long total(const Invoice& invoice);
    // Validate line amounts/payment fields and prevent duplicate billing of a prescription.
    bool add(const Invoice& invoice);
    // Read an invoice by its persistent ID.
    const Invoice* find(int id) const;
    // Return a pending bill owned by this system so its service lines can be amended.
    Invoice* findMutable(int id);
    // Mark an Unpaid invoice Paid with a method/time; reject repeated payment recording.
    bool pay(int id, const std::string& method, const std::string& timestamp);
    // Expose read-only invoices for balances, patient profiles and saving.
    const std::vector<Invoice>& records() const;
};
