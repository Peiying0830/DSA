// PharmacyInventory.h
// Encapsulated medicine catalog, stock counts and selling prices.
// DSA Final Assessment - Hospital Management System.

#pragma once
#include <map>
#include <string>
#include <vector>
struct Drug {
    int id = 0, stock = 0, reorderLevel = 0;
    std::string name, unit;
    long long priceCents = 0;
};
class PharmacyInventory {
    std::map<int,Drug> drugs;
public:
    // Validate medicine data and insert a unique ID into the encapsulated catalog.
    bool add(const Drug& drug);
    // Read a medicine by ID; return nullptr when it has not been registered.
    const Drug* find(int id) const;
    // Increase stock only for a positive quantity that stays within the storage limit.
    bool restock(int id, int quantity);
    // Change the current selling price; previously created invoices retain their snapshots.
    bool setPrice(int id, long long priceCents);
    // Reject insufficient stock before subtracting the dispensing quantity.
    bool consume(int id, int quantity);
    // Return medicine records in ID order for display and serialisation.
    std::vector<Drug> records() const;
};
