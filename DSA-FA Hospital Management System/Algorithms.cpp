// Algorithms.cpp
// Tasks 7 and 8: manual searching and sorting with comparison metrics.
// DSA Final Assessment - Hospital Management System.

#include "Algorithms.h"
#include <chrono>
#include <utility>
using namespace std;

namespace {
// Compare the selected key in the requested order; shared by both assessed sorts.
bool before(const Patient& a, const Patient& b, bool byAge, bool descending) {
    int x = byAge ? a.age : a.id, y = byAge ? b.age : b.id;
    return descending ? x > y : x < y;
}
}
// Check records sequentially, counting ID equality tests; return -1 if not found.
SearchResult Algorithms::linearSearch(const vector<Patient>& data, int id) {
    SearchResult result;
    for (size_t i = 0; i < data.size(); ++i) {
        ++result.comparisons;
        if (data[i].id == id) { result.position = static_cast<int>(i); break; }
    }
    return result;
}
// Require ascending IDs; halve the remaining range after each midpoint comparison.
SearchResult Algorithms::binarySearch(const vector<Patient>& sorted, int id) {
    SearchResult result;
    int low = 0, high = static_cast<int>(sorted.size()) - 1;
    while (low <= high) {
        // This midpoint expression avoids adding two potentially large indices.
        int mid = low + (high - low) / 2;
        ++result.comparisons; // Count each actual ID relational/equality comparison.
        if (sorted[mid].id == id) { result.position = mid; break; }
        ++result.comparisons;
        if (sorted[mid].id < id) low = mid + 1;
        else high = mid - 1;
    }
    return result;
}
// Recursively divide a half-open range and stably merge its sorted halves.
void Algorithms::mergeRange(vector<Patient>& data, vector<Patient>& buffer,
                            int begin, int end, bool byAge, bool descending, SortMetrics& metrics) {
    if (end - begin <= 1) return;
    int mid = begin + (end - begin) / 2;
    // Divide: recursively sort each half before combining them.
    mergeRange(data, buffer, begin, mid, byAge, descending, metrics);
    mergeRange(data, buffer, mid, end, byAge, descending, metrics);
    int left = begin, right = mid, out = begin;
    // Merge: choose the next ordered record while both halves still have records.
    while (left < mid && right < end) {
        ++metrics.comparisons;
        if (before(data[right], data[left], byAge, descending)) buffer[out++] = data[right++];
        else buffer[out++] = data[left++]; // Stable: keep left record on equal keys.
    }
    // The unconsumed half is already sorted, so copy its remaining records directly.
    while (left < mid) buffer[out++] = data[left++];
    while (right < end) buffer[out++] = data[right++];
    for (int i = begin; i < end; ++i) data[i] = buffer[i];
}
// Sort with divide-and-conquer and an O(n) buffer; record comparisons and elapsed time.
SortMetrics Algorithms::mergeSort(vector<Patient>& data, bool byAge, bool descending) {
    SortMetrics result;
    auto start = chrono::steady_clock::now();
    vector<Patient> buffer(data.size());
    mergeRange(data, buffer, 0, static_cast<int>(data.size()), byAge, descending, result);
    result.microseconds = chrono::duration<double,micro>(chrono::steady_clock::now()-start).count();
    return result;
}
// Compare adjacent keys, move out-of-order records and stop early on a no-swap pass.
SortMetrics Algorithms::bubbleSort(vector<Patient>& data, bool byAge, bool descending) {
    SortMetrics result;
    auto start = chrono::steady_clock::now();
    for (size_t end = data.size(); end > 1; --end) {
        bool swapped = false;
        // Each pass places the final record for the selected order at end - 1.
        for (size_t i = 1; i < end; ++i) {
            ++result.comparisons;
            if (before(data[i], data[i-1], byAge, descending)) {
                swap(data[i],data[i-1]); swapped = true;
            }
        }
        // No swaps means the whole remaining range is already ordered.
        if (!swapped) break;
    }
    result.microseconds = chrono::duration<double,micro>(chrono::steady_clock::now()-start).count();
    return result;
}
