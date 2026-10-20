#ifndef FINDER_H
#define FINDER_H

#include "Ticket.h"

using namespace std;

// Assignment Part C and Part D: Searching algorithms and comparison.
// Finder groups the searching algorithms used by the system.
// It compares linear search with binary search for DSA demonstration.
class Finder {
public:
    static int linearFind(const Ticket tickets[], int ticketCount, int ticketID, int& comparisons);
    static int binaryFind(const Ticket tickets[], int ticketCount, int ticketID, int& comparisons);
    static void printComparisonTable(int linearComparisons, int binaryComparisons);
};

#endif // FINDER_H
