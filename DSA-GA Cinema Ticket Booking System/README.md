# Cinema Ticket Booking System

Console-based cinema ticket booking system for the BCS2153 Data Structure group assignment.

## Features

- Customer movie browsing and ticket booking
- Seat map display with normal, twin, and wheelchair seats
- Waiting line queue using a linked list
- Cancellation log stack using an array
- Linear search and binary search for ticket records
- Insertion sort for ticket record display
- Movie management with array insert, update, and delete operations
- Text file save, load, autosave, and report export

## Main Files

- `main.cpp` - menu flow and user interface
- `Registry.cpp` / `Registry.h` - main controller for tickets, booking, search, sort, and files
- `WaitingLine.cpp` / `WaitingLine.h` - queue implementation
- `CancelLog.cpp` / `CancelLog.h` - stack implementation
- `Finder.cpp` / `Finder.h` - searching algorithms
- `MovieCatalog.cpp` / `MovieCatalog.h` - movie screening catalog
- `Ticket.cpp` / `Ticket.h` - ticket data and validation helpers

## Compile

```bash
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp Registry.cpp WaitingLine.cpp CancelLog.cpp Finder.cpp MovieCatalog.cpp Ticket.cpp -o CinemaTicketBooking.exe
```

## Run

```bash
./CinemaTicketBooking.exe
```

Staff/Admin password:

```text
admin123
```
