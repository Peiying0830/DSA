#include "Finder.h"

#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

// Print one aligned table row with fixed column widths.
static void printTableRow(const string& metric, const string& linearValue, const string& binaryValue) {
    // setw keeps every comparison table column aligned.
    cout << "| " << left << setw(26) << metric
         << " | " << left << setw(28) << linearValue
         << " | " << left << setw(28) << binaryValue
         << " |" << endl;
}

// Linear search checks tickets one by one from the start of the array.
// Example: to find Ticket ID 1005, it may check 1001, 1002, 1003, 1004, then 1005.
// It does not need sorted data, but it can become slow when the ticket array grows.
int Finder::linearFind(const Ticket tickets[], int ticketCount, int ticketID, int& comparisons) {
    // comparisons counts how many ticket IDs are checked during this search.
    comparisons = 0;
    for (int index = 0; index < ticketCount; ++index) {
        comparisons++;
        if (tickets[index].ticketID == ticketID) {
            return index;
        }
    }
    return -1;
}

// Sort a copy of tickets by ticketID using insertion sort before binary search.
static void insertionSortByTicketID(Ticket sortedTickets[], int ticketCount) {
    // Binary search needs sorted data, so this helper sorts the copied array first.
    for (int i = 1; i < ticketCount; ++i) {
        Ticket key = sortedTickets[i];
        int j = i;

        // Move elements that are greater than key.ticketID one position ahead.
        while (j > 0 && sortedTickets[j - 1].ticketID > key.ticketID) {
            sortedTickets[j] = sortedTickets[j - 1];
            --j;
        }
        sortedTickets[j] = key;
    }
}

// Binary search works on a sorted copy of the ticket array.
// It is not related to the "base 2" value of the Ticket ID number.
// It works by using the middle position of the sorted array.
// If the target Ticket ID is larger than the middle Ticket ID, the left half is ignored.
// If the target Ticket ID is smaller than the middle Ticket ID, the right half is ignored.
// This repeats until the target is found or the search range becomes empty.
// Example sorted IDs: 1001, 1002, 1003, 1004, 1005.
// Searching 1005 checks middle 1003, then 1004, then 1005, so it uses 3 comparisons.
int Finder::binaryFind(const Ticket tickets[], int ticketCount, int ticketID, int& comparisons) {
    // Work on a copy so the original ticket registry order is not changed.
    Ticket* sortedTickets = new Ticket[ticketCount];
    for (int index = 0; index < ticketCount; ++index) {
        sortedTickets[index] = tickets[index];
    }
    insertionSortByTicketID(sortedTickets, ticketCount);

    int left = 0;
    int right = ticketCount - 1;
    comparisons = 0;

    while (left <= right) {
        // Check the middle ticket, then discard half of the remaining search range.
        int mid = left + (right - left) / 2;
        comparisons++;
        if (sortedTickets[mid].ticketID == ticketID) {
            delete[] sortedTickets;
            return mid;
        }
        if (sortedTickets[mid].ticketID < ticketID) {
            // Target is larger, so search only the right half.
            left = mid + 1;
        } else {
            // Target is smaller, so search only the left half.
            right = mid - 1;
        }
    }

    delete[] sortedTickets;
    return -1;
}

// Print Big-O comparison table for presentation/demo.
void Finder::printComparisonTable(int linearComparisons, int binaryComparisons) {
    // This table supports Part D by comparing complexity and this-run comparisons.
    const string border = "+----------------------------+------------------------------+------------------------------+";

    cout << border << endl;
    printTableRow("Metric", "Linear Search", "Binary Search");
    cout << border << endl;
    printTableRow("Comparisons (this run)", to_string(linearComparisons), to_string(binaryComparisons));
    printTableRow("Best Case", "O(1)", "O(1)");
    printTableRow("Worst Case", "O(n)", "O(log n)");
    printTableRow("Needs Sorted Data First", "No", "Yes, sort first");
    printTableRow("Advantage", "Simple, no sorting needed", "Fast on sorted data");
    printTableRow("Disadvantage", "Slow for large datasets", "Must sort data first");
    cout << border << endl;
}
