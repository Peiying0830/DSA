// Binary Search Using list<int> in C++
// This program builds four sorted lists (10, 100, 1,000 and 10,000 elements),
// searches each list with binary search, and shows the number of comparisons.

#include <iostream>   // for cin and cout
#include <list>       // for list<int>
#include <iterator>   // for next()
using namespace std;

// Create a sorted list of "size" elements.
// Each element is (index * step), so the values are in ascending order.
// Example: size = 10, step = 10 gives 10, 20, 30, ... 100
list<int> createList(int size, int step)
{
    list<int> numbers;
    for (int i = 1; i <= size; i++)
    {
        numbers.push_back(i * step);   // add to the back to keep ascending order
    }
    return numbers;
}

// Binary search on a sorted list.
// Returns true if the target is found, otherwise false.
// "comparisons" is updated with the number of comparisons made.
bool binarySearch(const list<int>& numbers, int target, int& comparisons)
{
    int low = 0;                          // first position
    int high = numbers.size() - 1;        // last position
    comparisons = 0;                      // reset the counter

    while (low <= high)
    {
        int mid = (low + high) / 2;       // middle position

        // A list has no direct access by index, so we move the iterator
        // from the beginning to the middle position.
        int midValue = *next(numbers.begin(), mid);

        comparisons++;                    // count one comparison per loop

        if (midValue == target)
        {
            return true;                  // value found
        }
        else if (midValue < target)
        {
            low = mid + 1;                // search the right half
        }
        else
        {
            high = mid - 1;               // search the left half
        }
    }
    return false;                         // value not found
}

// Search one list and display the result in a suitable format.
void displayResult(const list<int>& numbers, int target)
{
    int comparisons = 0;
    bool found = binarySearch(numbers, target, comparisons);

    cout << "List Size: " << numbers.size() << endl;
    cout << "Search Value: " << target << endl;
    cout << "Result: " << (found ? "Element Found" : "Element Not Found") << endl;
    cout << "Comparisons: " << comparisons << endl << endl;
}

int main()
{
    // Create the four sorted lists
    list<int> list1 = createList(10, 10);      // 10, 20, ..., 100
    list<int> list2 = createList(100, 1);      // 1, 2, ..., 100
    list<int> list3 = createList(1000, 1);     // 1, 2, ..., 1000
    list<int> list4 = createList(10000, 1);    // 1, 2, ..., 10000

    // Ask the user for a value to search
    int input;
    cout << "Enter value to search: ";
    cin >> input;

    cout << endl << "========== BINARY SEARCH ==========" << endl << endl;

    // Test the same user input on all four list sizes.
    // This follows the requirement to test the Binary Search algorithm
    // using lists with 10, 100, 1,000 and 10,000 elements.
    displayResult(list1, input);
    displayResult(list2, input);
    displayResult(list3, input);
    displayResult(list4, input);

    return 0;
}
