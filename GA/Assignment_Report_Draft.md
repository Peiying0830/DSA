# Cinema Ticket Booking System Using Stack, Queue, and Searching Algorithms

## Group Assignment Report

Course: BCS2153 / BIT2123 Data Structure and Algorithm  
Study Intake: 202607  
Group Members:

| No. | Name | Student ID | Contribution |
| --- | --- | --- | --- |
| 1 | [Name] | [Student ID] | Queue, booking flow, and ticket records |
| 2 | [Name] | [Student ID] | Stack cancellation log and undo cancellation |
| 3 | [Name] | [Student ID] | Searching, sorting, and comparison report |
| 4 | [Name] | [Student ID] | Testing, validation, documentation, and presentation |

---

## 1. Introduction

This project is a menu-driven Cinema Ticket Booking System developed in C++. The system helps manage cinema ticket bookings, waiting customers, cancellation records, searching, and reporting. It applies important data structure concepts including Queue, Stack, Array, Linked List, Linear Search, Binary Search, and Insertion Sort.

The system provides separate menus for Customer and Staff/Admin users. Customers can browse movies, book tickets, search tickets, and cancel bookings. Staff/Admin users can manage movie records, serve customers from the waiting queue, view records, search tickets, undo cancellations, and export reports.

---

## 2. Objectives

The objectives of this system are:

- To apply Queue FIFO concept for managing customers waiting to purchase tickets.
- To apply Stack LIFO concept for storing booking cancellation history.
- To implement Linear Search and Binary Search for ticket searching.
- To compare the performance of Linear Search and Binary Search.
- To use arrays, linked lists, structures/classes, functions, and validation in a C++ application.
- To develop a complete menu-driven system with records display and report export.

---

## 3. System Features

### 3.1 Customer Features

- Browse available movies and showtimes.
- View movie lists in a table format with wrapped long titles.
- View seat maps.
- Book one or more tickets.
- Automatically generate Ticket ID.
- Search ticket details.
- Cancel a ticket with cancellation reason, or enter 0 to cancel the action.

### 3.2 Staff/Admin Features

- Insert, view, edit, and delete movie screenings.
- Serve next customer from the waiting queue.
- Serve all waiting customers.
- View waiting line details.
- View front and rear waiting customers.
- Check queue status with waiting count, capacity view, and usage percentage.
- View cancellation records.
- Peek latest cancellation.
- Undo latest cancellation after viewing the latest cancellation details and confirming with Y/N.
- View full ticket registry.
- View system summary.
- Sort ticket records by Ticket ID, movie, hall, ticket type, or status.
- Save and load data from text files.
- Export system report.

---

## 4. Data Structures Used

| Data Structure | Implementation | Purpose |
| --- | --- | --- |
| Queue | Linked List | Stores waiting customers in FIFO order |
| Stack | Array | Stores cancellation records in LIFO order |
| Array | Fixed-size array | Stores ticket records and movie records |
| Struct/Class | C++ classes and structs | Organizes ticket, cancellation, queue, and catalog data |

### 4.1 Queue FIFO

The queue represents customers waiting to purchase or process tickets. It follows the First In, First Out principle. The first customer added to the queue will be the first customer served.

Queue operations implemented:

- Add customer to booking queue
- Serve/remove next customer
- Display all waiting customers
- Display front customer
- Display rear customer
- Check whether queue is empty or full
- Display queue capacity view and usage percentage

The queue is implemented using a linked list, so customers can be added and removed dynamically.

### 4.2 Stack LIFO

The stack stores booking cancellation history. It follows the Last In, First Out principle. The most recent cancellation is placed at the top of the stack and can be undone first.

Stack operations implemented:

- Push cancellation record
- Pop latest cancellation for undo
- Display cancellation history
- Peek latest cancellation
- Confirm undo before restoring the latest cancelled ticket

The stack is implemented using an array.

---

## 5. Searching Algorithms

### 5.1 Linear Search

Linear Search checks ticket records one by one until the target Ticket ID is found or all records have been checked.

Advantages:

- Simple to implement.
- Does not require sorted data.

Disadvantages:

- Slower for large datasets.
- Worst-case time complexity is O(n).

### 5.2 Binary Search

Binary Search searches the ticket records by repeatedly dividing the sorted data into half. Before Binary Search is applied, a sorted copy of the ticket records is created based on Ticket ID.

Advantages:

- Faster for large sorted datasets.
- Time complexity is O(log n).

Disadvantages:

- Requires sorted data before searching.
- Sorting adds extra processing if the original data is unsorted.

### 5.3 Searching Comparison

| Algorithm | Time Complexity | Sorted Data Required | Description |
| --- | --- | --- | --- |
| Linear Search | O(n) | No | Checks each ticket record one by one |
| Binary Search | O(log n) | Yes | Searches by repeatedly dividing sorted records |

The system displays the number of comparisons made by both algorithms. For small datasets, Linear Search may be simpler. For larger sorted datasets, Binary Search performs better.

---

## 6. Sorting Algorithm

Insertion Sort is used to sort ticket records before displaying sorted records. It is also used to sort a copy of ticket records by Ticket ID before Binary Search is applied.

Sorting options implemented:

- Sort by Ticket ID
- Sort by movie title
- Sort by hall and showtime
- Sort by ticket type
- Sort by ticket status

The system also displays:

- Number of insertion sort comparisons
- Number of insertion sort shifts

If ticket records are already in ascending order, the number of shifts may be 0 because no elements need to be moved.

---

## 7. File Handling

The system supports saving and loading records using text files.

Data files used:

- `tickets_autosave.txt`
- `movies_autosave.txt`
- `cancellations_autosave.txt`

The system can also manually save and load:

- Ticket records
- Movie catalog
- Cancellation log

This allows data to be reused after the program is closed and reopened.

---

## 8. Validation

Input validation is included to reduce invalid data entry. The system validates:

- Menu option range
- Ticket ID input
- 0 cancel/back option for ticket ID input
- 0 cancel/back option for movie and showtime selection
- Movie details
- Hall name
- Showtime
- Seat code
- Ticket type
- File name
- Empty text input

Invalid input will display an error message and ask the user to enter the value again.

Several actions also support cancellation by entering 0, such as selecting a movie, selecting a showtime, entering a Ticket ID, editing a movie, deleting a movie, searching a ticket, and editing ticket information.

---

## 9. Sample Output Screenshots

Paste screenshots of your program output in this section.

### 9.1 Main Menu

[Insert screenshot here]

### 9.2 Customer Booking

[Insert screenshot here]

### 9.3 Queue Status

[Insert screenshot here]

### 9.4 Cancellation Log

[Insert screenshot here]

### 9.5 Linear and Binary Search Comparison

[Insert screenshot here]

### 9.6 Sorted Ticket Records

[Insert screenshot here]

### 9.7 Movie List Table

[Insert screenshot here]

### 9.8 Exported Report

[Insert screenshot here]

---

## 10. Testing

| Test Case | Input / Action | Expected Result | Status |
| --- | --- | --- | --- |
| Book ticket | Select movie, showtime, seat, and ticket type | Ticket is created and added to waiting queue | Pass |
| Serve next customer | Select Serve Next Customer | First waiting ticket is served | Pass |
| View waiting line | Select Waiting Line Details | All waiting tickets are displayed | Pass |
| View queue status | Select Queue Status | Waiting count, capacity view, usage, empty status, and full status are displayed | Pass |
| View front customer | Select Front Waiting Customer | First waiting ticket and queue status are displayed | Pass |
| View rear customer | Select Rear Waiting Customer | Last waiting ticket and queue status are displayed | Pass |
| Cancel ticket | Enter valid waiting Ticket ID and reason | Ticket status becomes CANCELLED and cancellation is pushed to stack | Pass |
| Cancel ticket action | Enter 0 at Ticket ID prompt | Action is cancelled and returns to menu | Pass |
| Undo cancellation | Select Undo Last Cancellation and confirm Y | Latest cancelled ticket returns to WAITING | Pass |
| Cancel undo | Select Undo Last Cancellation and enter N | Cancellation remains unchanged | Pass |
| Peek latest cancellation | Select Latest Cancellation | Most recent cancellation record is displayed | Pass |
| Linear search | Enter existing Ticket ID | Ticket details are displayed with comparison count | Pass |
| Binary search | Enter existing Ticket ID | Ticket details are displayed after searching sorted copy | Pass |
| Search comparison | Enter Ticket ID | Linear and binary comparison results are displayed | Pass |
| Sort by Ticket ID | Select Sort by Ticket ID | Tickets are displayed in ascending Ticket ID order | Pass |
| Sort by movie | Select Sort by Movie | Tickets are displayed by movie title | Pass |
| Sort by hall | Select Sort by Hall | Tickets are displayed by hall and showtime | Pass |
| Sort by ticket type | Select Sort by Ticket Type | Tickets are displayed by ticket type | Pass |
| Sort by status | Select Sort by Status | Tickets are displayed by ticket status | Pass |
| Edit movie cancel | Enter 0 during movie or showtime selection | Edit action is cancelled | Pass |
| Delete movie cancel | Enter 0 during movie or showtime selection | Delete action is cancelled | Pass |
| Invalid menu input | Enter invalid option | Error message is displayed | Pass |
| Export report | Enter report file name | Report file is generated | Pass |

---

## 11. Limitations and Future Improvements

Current limitations:

- Data is stored in text files instead of a database.
- Staff/Admin login uses a simple password.
- The program runs in a console interface.
- Cancellation records recovered from older ticket files may not contain the original reason and timestamp.

Future improvements:

- Add database storage for stronger data management.
- Add user account login and role permissions.
- Add graphical user interface.
- Add payment processing simulation.
- Add more detailed sales and revenue analytics.

---

## 12. Conclusion

The Cinema Ticket Booking System successfully applies the required data structure and algorithm concepts in a real-world booking scenario. The system uses a linked list queue to manage waiting customers, an array stack to manage cancellation history, arrays to store ticket records, and searching algorithms to find booking records.

Linear Search and Binary Search are implemented and compared using comparison counts and time complexity. The system also includes sorting, validation, file handling, report export, and a complete menu-driven interface. Overall, the project meets the functional and programming requirements of the assignment.
