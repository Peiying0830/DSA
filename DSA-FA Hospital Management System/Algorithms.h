// Algorithms.h
// Tasks 7 and 8: manual searching and sorting with comparison metrics.
// DSA Final Assessment - Hospital Management System.

#pragma once
#include "Models.h"
#include <vector>
struct SearchResult { int position = -1; long long comparisons = 0; };
struct SortMetrics { long long comparisons = 0; double microseconds = 0; };
class Algorithms {
    // Recursively divide a half-open range and stably merge its sorted halves.
    static void mergeRange(std::vector<Patient>& data, std::vector<Patient>& buffer,
                           int begin, int end, bool byAge, bool descending, SortMetrics& metrics);
public:
    // Check records sequentially, counting ID equality tests; return -1 if not found.
    static SearchResult linearSearch(const std::vector<Patient>& data, int id);
    // Require ascending IDs; halve the remaining range after each midpoint comparison.
    static SearchResult binarySearch(const std::vector<Patient>& sorted, int id);
    // Sort with divide-and-conquer and an O(n) buffer; record comparisons and elapsed time.
    static SortMetrics mergeSort(std::vector<Patient>& data, bool byAge, bool descending);
    // Compare adjacent keys, move out-of-order records and stop early on a no-swap pass.
    static SortMetrics bubbleSort(std::vector<Patient>& data, bool byAge, bool descending);
};
