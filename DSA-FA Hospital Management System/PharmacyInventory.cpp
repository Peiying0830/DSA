// PharmacyInventory.cpp
// Encapsulated medicine catalog, stock counts and selling prices.
// DSA Final Assessment - Hospital Management System.

#include "PharmacyInventory.h"
#include "Input.h"
using namespace std;

// Validate medicine data and insert a unique ID into the encapsulated catalog.
bool PharmacyInventory::add(const Drug& drug) {
    if (drug.id<1 || drug.id>999999 || drug.stock<0 || drug.stock>1000000 ||
        drug.reorderLevel<0 || drug.reorderLevel>1000000 || drug.priceCents<1 ||
        drug.priceCents>100000000 || !Input::validText(drug.name,100) || !Input::validText(drug.unit,50)) return false;
    return drugs.emplace(drug.id,drug).second;
}
// Read a medicine by ID; return nullptr when it has not been registered.
const Drug* PharmacyInventory::find(int id) const {
    auto found=drugs.find(id);
    return found==drugs.end() ? nullptr : &found->second;
}
// Increase stock only for a positive quantity that stays within the storage limit.
bool PharmacyInventory::restock(int id, int quantity) {
    auto found=drugs.find(id);
    if (found==drugs.end() || quantity<1 || quantity>1000000-found->second.stock) return false;
    found->second.stock+=quantity; return true;
}
// Change the current selling price; previously created invoices retain their snapshots.
bool PharmacyInventory::setPrice(int id, long long priceCents) {
    auto found=drugs.find(id);
    if (found==drugs.end() || priceCents<1 || priceCents>100000000) return false;
    found->second.priceCents=priceCents; return true;
}
// Reject insufficient stock before subtracting the dispensing quantity.
bool PharmacyInventory::consume(int id, int quantity) {
    auto found=drugs.find(id);
    if (found==drugs.end() || quantity<1 || quantity>found->second.stock) return false;
    found->second.stock-=quantity; return true;
}
// Return medicine records in ID order for display and serialisation.
vector<Drug> PharmacyInventory::records() const {
    vector<Drug> result;
    for (const auto& entry : drugs) result.push_back(entry.second);
    return result;
}
