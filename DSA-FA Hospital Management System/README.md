# DSA-FA Hospital Management System

C++17 console application for the DSA Final Assessment. Patient records use an
array and BST, medical history a singly linked list, Undo a manually implemented linked stack, normal service
a circular FIFO queue, and referrals a directed graph. Searching and sorting
algorithms are implemented manually in Algorithms.cpp.

## Build and run

Compile the entire project, not only the active .cpp file. In CMD:

```bat
cd /d "C:\DSA\DSA-FA Hospital Management System"
build.cmd
HospitalManagement.exe
```

In PowerShell:

```powershell
cd "C:\DSA\DSA-FA Hospital Management System"
.\build.ps1
.\HospitalManagement.exe
```

VS Code Ctrl+Shift+B builds the project; F5 builds/debugs it. All normal builds
produce the single HospitalManagement.exe. Exit running hospital instances before
replacing it. The application opens data/hospital.db beside its executable,
regardless of the terminal's working directory.

All submenu `0. Back to main menu` options return directly to the main dashboard,
including Back prompts in detail views. Main-menu `0` exits the application.

## Current menus

| Main option | Contents |
| --- | --- |
| 1 Patient management | Patient records; one patient profile containing medical history, appointments, queues, prescriptions and invoices |
| 2 Doctor management | Register doctor, view doctors, update consultation fee, manage shifts |
| 3 Appointment management | Booking, appointment list, rescheduling, check-in, cancellation and visit records |
| 4 Service queues | Normal FIFO and emergency priority queues |
| 5 Clinical visits & pharmacy | Medical records, treatment orders, medicine inventory and dispensing |
| 6 Billing & payments | View invoices (all/by patient and invoice details), record payment |
| 7 Reports & algorithms | Department referrals; Sort & ordered display |
| 8 Data & session | Export/restore/clear data; operation log and Stack/LIFO Undo |
| 0 Exit | End session; successful changes are already autosaved |

Patient management contains 1 Patient Records and 2 Patient Profile.
Profile asks for the Patient ID once, then displays basic details, chronological
medical records, appointments, queue status, prescriptions and invoices. Separate
medical-history and appointment view entries are removed from this category.

Patient management > Patient records:
1 Add patient; 2 Display patient array; 3 Update patient; 4 Delete patient;
5 Search patient. Search patient contains BST, Linear, Binary, Compare
Linear/Binary, and Benchmark Linear/Binary (options 1-5).

Reports & algorithms > Sort & ordered display:
1 BST in-order (Patient ID ascending); 2 Merge sort; 3 Bubble sort;
4 Compare Merge/Bubble; 5 Benchmark Merge/Bubble.
BST in-order traverses a tree; it is not an array sorting algorithm.
There are no duplicate search or in-order entries elsewhere.

Appointment management:
1 Book; 2 All appointments chronologically; 3 Reschedule;
4 Appointment status/check-in; 5 Cancel; 6 Visit medical records.
Available half-hour slots appear inside booking after patient, doctor and date.
They exclude both doctor and patient conflicts. Patient-specific appointments
are viewed in Patient Profile. Cancel appointment is inside Appointment Management;
there is one genuine LIFO Undo entry, under Session & logs.

Doctor management > View doctors shows a concise directory. Select a doctor ID
once to display their fee, department and full Monday-Sunday schedule (plus any
legacy dated shifts). There is no availability submenu: enter the
date directly to see its weekday, appointments and four-column available slots,
then optionally enter a time to check whether it is available/booked/off duty.
The selected doctor ID is reused. Enter 0 at the date/time prompt to return to
Doctor Management. Manage Shifts adds/removes weekly shifts. No shift means off duty; overlapping periods
are rejected, and the end time is exclusive. 24:00 is allowed as an end time.
Active appointments must remain covered when shifts change.

## CRUD and workflow menu order

Record menus follow Create / Read / Update / Delete, followed by search or
specialist actions. Menus that only query data retain their query choices;
unsupported delete/edit operations are not added just to fill CRUD positions.

| Submenu | Numbered options |
| --- | --- |
| Patient Records | 1 Add; 2 Display; 3 Update; 4 Delete; 5 Search |
| Doctor Management | 1 Register; 2 View; 3 Update Consultation Fee; 4 Manage Shifts |
| Visit Medical Records | 1 Add; 2 View; 3 Update; 4 Delete |
| Services | 1 Services and Prices (select services for a visit); 2 Update Service Prices |
| Selected Visit Services | Select appointment once; 1 Add; 2 View; 3 Remove |
| Update Service Prices | Catalogue displayed automatically; enter Service ID and new price directly; 0 returns |
| Pharmacy & Prescriptions | 1 Medicine Management; 2 Prescription Management |
| Manage Medicine | 1 Add; 2 View All; 3 Update; 4 Low-stock |
| Manage Prescriptions | 1 Create; 2 List; 3 Dispense; 4 Cancel Pending |
| Normal FIFO Queue | 1 Enqueue; 2 Display Waiting; 3 Front/Peek; 4 Serve One/All |
| Emergency Queue | 1 Enqueue; 2 Display Waiting; 3 Serve Highest Priority |
| Data Management | 1 Export; 2 Restore; 3 Clear |
| Session & Logs | 1 View Operations; 2 Undo |

Doctor shift editing supports Add then Remove. Billing keeps View Invoices then
Record Payment. Appointment management keeps Book, View All, Reschedule, Status/
Check-in, Cancel and Visit Medical Records. Weekdays and algorithm choices retain their natural order.
Existing demonstration scripts should use these current option numbers.

## Records and workflow

All new records receive IDs automatically, including medicines (M011, M012...) and
services (S004, S005...). IDs are entered only when selecting existing records.
Patient and doctor registration generate P001 / D001 style IDs. Other display
prefixes are A (appointment), R (medical record), M (medicine), RX (prescription),
I (invoice), S (service) and V (visit service). Matching prefixed IDs are accepted
case-insensitively; legacy numeric IDs also work. Counters persist across restarts
and are not reused after ordinary deletion or Undo.

Enter exactly 0 to cancel a form or return from a menu. Unfinished forms are not
saved. If a numeric field legitimately accepts zero (age/stock/threshold), enter
00 because exact 0 is reserved for cancellation. Names, IDs, ages, dates, text
lengths, money, quantities, references, statuses, shift overlap, appointment
conflicts, queue capacity/duplicates and stock limits are validated.
Past dates are allowed for historical assessment demonstrations.

Appointment status flow:
- Scheduled -> Checked-in: automatically joins the normal consultation FIFO.
- Scheduled -> Cancelled or No-show: retains the record and frees the slot.
- Checked-in -> Completed: first serve the patient from FIFO; a configured
  consultation fee and invoice confirmation are required.
Closed appointments cannot be edited through normal status changes. Checked-in
appointments cannot be cancelled/rescheduled. Undo reverses the latest change.

Appointment Management > 6 Visit Medical Records offers Add / View / Update / Delete. Adding lists the
patient's checked-in appointments and asks which visit to link. Date, time and
doctor come from that appointment. Enter Consultation, Diagnosis, Treatment and
Prescription notes together. N/A is allowed, but all four cannot be N/A. Mutations
show the updated history. Legacy unlinked records remain marked as such.

Normal queue: enqueue, serve one/all in FIFO order, peek front, display waiting.
Emergency queue: staff-assigned priority 1?3, highest priority first, FIFO ties.
A patient cannot be waiting in both queues simultaneously. Duplicate checks
display the existing queue and next steps. Linked consultation entries also show
the waiting appointment ID, doctor, date/time and FIFO position. Manual/emergency
entries are explicitly unlinked; the system does not guess an appointment. Emergency priority is
entered by staff; the program does not derive it from symptoms.
Deleting a patient is blocked while retained linked records reference them.

## Services, pharmacy and automated bills

Services > Services and Prices shows the catalogue, then lists checked-in/completed
appointments. Select the
appointment once, then view, add or remove its services without entering the ID
again. Completed/billed visits are view-only; mutation attempts are rejected.
Services > Update Service Prices displays the catalogue and asks directly for the
Service ID and new price. There is no extra Update submenu; 0 returns to Services.

Check-in automatically creates one unpaid invoice containing the doctor's captured
consultation fee. Services can be attached to that checked-in visit before payment;
each one updates the same invoice using its captured price. Completing a visit does
not create another invoice. Billing & Payments records the payment, so no amount is
calculated manually.

Clinical Visits & Pharmacy > Add New Service registers additional service types
with a name and price, assigns the next S-prefixed ID, and autosaves the catalog.
Use Manage Visit Services to charge the new service to a checked-in visit.
Service names are checked for duplicates without regard to letter case.

Pharmacy > Medicine Management: Add New Medicine, View All, Update Medicine,
Low-stock Medicines. Inventory is viewed here, without a duplicate outer entry.
Pharmacy > Prescription Management contains Create, List, Dispense and Cancel
Pending prescriptions together. Update shows the inventory first, then lets you restock and
change unit price in one transaction; '-' keeps a value. Prescriptions use
registered patients, doctors and medicines. No medication dosage is generated.
Pending prescriptions do not reserve stock. Creating a prescription offers immediate
dispensing or later processing. Dispensing validates every item before consuming
stock and adds the medication charges to that appointment's unpaid invoice, alongside
consultation and service charges. Multiple prescriptions can share the visit invoice.
If the visit invoice is already paid, medication gets a separate supplementary invoice;
the paid receipt stays unchanged. Older prescriptions without a stored appointment
ask staff to select the matching visit when available. Pending prescriptions can be
cancelled; repeated dispensing is rejected. Save format 8 retains appointment and
invoice-line references and can load earlier save formats.

Invoice lists can be filtered by patient. Select an invoice ID for details or a
receipt. Payment asks Cash/Card/Transfer and confirmation of full payment, then
records a local Malaysia-time payment. It does not process a bank transaction.
Existing invoices retain their original prices when later prices change.
Undo completion/dispensing reverses the linked invoice and associated changes.
Partial payments and refunds are not implemented.

Default built-in prices and doctor consultation fees are demonstration configuration.

## Autosave and complete backups

Every successful mutation, including Undo, autosaves to data/hospital.db.
The save format is version 7; versions 1?6 remain readable. Saving uses a temporary
file, retains the previous database as hospital.db.bak, then replaces the main
file. Failed saves roll back visible changes. A damaged startup database is
rejected without silently overwriting it. Only use one instance per data folder.

Data management > Export data creates ONE new file, e.g. Backup.db. It contains
an ASCII-framed readable overview (patients with appointments/history, doctors,
shifts, medicines, prescriptions, invoices/payments, services, queues and logs)
followed by its complete restore payload. Do not edit the restore payload.
Existing export filenames are rejected to avoid accidental overwriting.

Data management > Restore backup accepts the same exported file, a plain
hospital.db autosave or hospital.db.bak. It validates all records, previews counts
and requires confirmation before replacing current data. Failed/cancelled restores
leave current data unchanged. ID counters retain the larger current/backup values
so same-session Undo remains safe. Restore can be undone during that session.
If the normal startup file is damaged, preserve it and launch with a new data
folder using --data-dir, then restore a valid backup there.

Clear data requires typing CLEAR. It restores the fixed default data: P001-P010,
D001-D010 and M001-M010, and resets new record IDs to P011, D011 and M011.
Appointments, queues, history, prescriptions, invoices and operation logs are
removed. The Undo stack is cleared because this is a deliberate system reset.

### Price references

Service prices reference Klinik NurCahaya's public private-clinic self-pay fees,
checked 2026-10-15: Chest X-ray RM60; FBC from RM30; ECG from RM40.
Source: https://www.kliniknurcahaya.com/fees/
These are one clinic's reference prices, not a national/private-hospital tariff.
Doctor consultation fees RM40-80 and medicine prices remain demonstration data,
not verified specialist or pharmacy quotations. Change prices as appropriate.
Captured visit-service prices and issued invoice amounts do not change later.

## FA Task 1-9 demonstration map

| Task | What to demonstrate |
| --- | --- |
| 1 Array | At least 10 patients; display/add/search/update/delete |
| 2 Linked list | Visit history add/delete/search/traversal in chronological order |
| 3 Stack | Hand-written linked stack in UndoStack.h: push, pop, top, empty, clear; snapshot Undo and empty-stack message |
| 4 Queue | Enqueue, dequeue, peek, display, FIFO, empty-queue message |
| 5 BST | Insert/search/delete and ID-ascending in-order output |
| 6 Graph | Prescribed directed referrals; DFS and BFS |
| 7 Searching | Same ID with Linear/Binary, found/missing and comparison counts |
| 8 Sorting | Original records, selected key/order, Merge/Bubble results, counts/time |
| 9 Additional function | Hand-written array-backed binary min-heap with siftUp/siftDown in EmergencyQueue.cpp; equal priorities use FIFO |

From A (Emergency), DFS is A B D F E C and BFS is A B C D E F. These list
reachable departments; they are not a single continuous referral route.

Search comparisons count each executed Patient ID == or < test, not loop
conditions. Binary search first sorts a copy ascending; sorting cost is excluded.
Linear is O(n); binary is O(log n) after sorting. Normal sorts display original
records first, then sorted copies. Merge and Bubble start from identical copies;
counts are record-key comparisons, times exclude copying/printing. Merge's
internal allocation is included. Small timings vary with machine load.

Both Benchmark options generate 1,000 / 5,000 / 10,000 records independently of
registered patients, including with an empty hospital. Search looks for the last
ID in ascending data. Sorting uses reverse-sorted IDs and ascending output.
The 10,000-record Bubble run performs 49,995,000 comparisons and may take over
a minute in unoptimized builds. Generated records never enter live hospital data.
Comparison counts are deterministic for these inputs; execution times vary.

## Optional CMake build

If CMake is installed, the application can also be built with:

```powershell
cmake -S . -B build
cmake --build build
```

## Source structure and remaining submission work

HospitalSystem coordinates the menus and data structures. HospitalLifecycle
handles lifecycle/Undo/schedules; HospitalCommerce handles prescriptions/billing.
HospitalServices.inc and HospitalExchange.inc are included once in
HospitalSystem.cpp. Other modules encapsulate BST, linked list, queues, graph,
algorithms, input, persistence and commerce records. Ownership uses RAII;
medical-history snapshots deep-copy nodes. No external libraries are required.

This is a local, single-user assessment demonstration, with readable files and
no authentication/encryption or multi-user locking. Use fictional patient data.
The assessment report, references, recorded presentation (maximum 15 minutes)
and Turnitin report are separate submission materials; the application does not
generate them. Checking the program does not replace those deliverables.

Submenu titles explicitly identify FA Tasks 1-9 and their structures/algorithms.
Search combines Task 5 BST with Task 7 Linear/Binary; ordered display combines
Task 5 in-order with Task 8 Merge/Bubble. Task 9 is the emergency heap queue.
Doctor, services, pharmacy, billing and data tools remain supporting functions;
no unrelated task numbers are attached to them. Main menu names stay concise.

Patient Service Queues has only Normal and Emergency category entries.
Display waiting patients is available inside each queue, with no duplicate
combined display entry at the category level. Front/Peek remains separate
from display-all to demonstrate FA Task 4.
