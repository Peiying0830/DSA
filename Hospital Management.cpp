// FA development prototype: patient array, BST index, and appointment management.
// Compile: g++ -std=c++17 -Wall -Wextra -pedantic "Hospital Management.cpp" -o HospitalManagement.exe
#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stack>
#include <string>
#include <vector>
using namespace std;

struct Patient {
    int id = 0;
    string name;
    int age = 0;
    string gender, department, condition;
};

// The index stores IDs; patient records have one authoritative home in the array.
class PatientBST {
    struct Node {
        int id;
        unique_ptr<Node> left, right;
        explicit Node(int key) : id(key) {}
    };
    unique_ptr<Node> root;
    static void insert(unique_ptr<Node>& node, int id) {
        if (!node) node = make_unique<Node>(id);
        else if (id < node->id) insert(node->left, id);
        else if (id > node->id) insert(node->right, id);
    }
    static void erase(unique_ptr<Node>& node, int id) {
        if (!node) return;
        if (id < node->id) erase(node->left, id);
        else if (id > node->id) erase(node->right, id);
        else if (!node->left) node = move(node->right);
        else if (!node->right) node = move(node->left);
        else {
            Node* successor = node->right.get();
            while (successor->left) successor = successor->left.get();
            node->id = successor->id;
            erase(node->right, successor->id);
        }
    }
    static void inorder(const Node* node, vector<int>& ids) {
        if (!node) return;
        inorder(node->left.get(), ids);
        ids.push_back(node->id);
        inorder(node->right.get(), ids);
    }
public:
    void insert(int id) { insert(root, id); }
    void erase(int id) { erase(root, id); }
    void clear() { root.reset(); }
    bool contains(int id) const {
        const Node* node = root.get();
        while (node) {
            if (id == node->id) return true;
            node = id < node->id ? node->left.get() : node->right.get();
        }
        return false;
    }
    vector<int> sortedIDs() const {
        vector<int> ids;
        inorder(root.get(), ids);
        return ids;
    }
};

struct Doctor { int id; string name, department; };
struct Appointment {
    int id, patientID, doctorID;
    string date, time, reason;
};

// Whole-line input rejects inputs such as "12abc" and handles end-of-input.
class Input {
public:
    static string text(const string& prompt) {
        for (;;) {
            cout << prompt;
            string value;
            if (!getline(cin, value)) throw runtime_error("Input closed.");
            const auto first = value.find_first_not_of(" \t\r");
            if (first != string::npos)
                return value.substr(first, value.find_last_not_of(" \t\r") - first + 1);
            cout << "This field cannot be empty.\n";
        }
    }
    static int number(const string& prompt, int minimum, int maximum) {
        for (;;) {
            string value = text(prompt);
            istringstream stream(value);
            int result;
            char extra;
            if ((stream >> result) && !(stream >> extra) && result >= minimum && result <= maximum)
                return result;
            cout << "Enter a whole number from " << minimum << " to " << maximum << ".\n";
        }
    }
    static bool validDate(const string& date) {
        if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
        for (size_t i = 0; i < date.size(); ++i)
            if (i != 4 && i != 7 && (date[i] < '0' || date[i] > '9')) return false;
        int year = stoi(date.substr(0, 4)), month = stoi(date.substr(5, 2)), day = stoi(date.substr(8, 2));
        if (year < 1900 || month < 1 || month > 12) return false;
        int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
        if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0)) days[1] = 29;
        return day >= 1 && day <= days[month - 1];
    }
    static bool validTime(const string& time) {
        if (time.size() != 5 || time[2] != ':') return false;
        for (size_t i = 0; i < time.size(); ++i)
            if (i != 2 && (time[i] < '0' || time[i] > '9')) return false;
        int hour = stoi(time.substr(0, 2)), minute = stoi(time.substr(3, 2));
        return hour >= 9 && hour < 17 && (minute == 0 || minute == 30);
    }
    static string date() {
        for (;;) {
            string value = text("Date (YYYY-MM-DD): ");
            if (validDate(value)) return value;
            cout << "Enter a real calendar date.\n";
        }
    }
    static string time() {
        for (;;) {
            string value = text("Time (HH:MM, 09:00-16:30, 30-minute slots): ");
            if (validTime(value)) return value;
            cout << "Select a valid 30-minute clinic slot.\n";
        }
    }
};

class HospitalSystem {
    static constexpr int capacity = 100;
    struct State {
        array<Patient, capacity> patients{};
        int count = 0;
        vector<Appointment> appointments;
    } state;
    struct UndoEntry { State previous; string description; };
    stack<UndoEntry> undo;
    PatientBST index;
    int nextAppointmentID = 1; // Never reuse an ID after cancellation or undo.
    const vector<Doctor> doctors = {
        {1, "Dr. Lim", "Cardiology"}, {2, "Dr. Aisha", "Neurology"},
        {3, "Dr. Kumar", "General Medicine"}, {4, "Dr. Tan", "Surgery"}
    };
    int patientPosition(int id) const {
        for (int i = 0; i < state.count; ++i)
            if (state.patients[i].id == id) return i;
        return -1;
    }
    void remember(const string& description) { undo.push({state, description}); }
    void rebuildIndex() {
        index.clear();
        for (int i = 0; i < state.count; ++i) index.insert(state.patients[i].id);
    }
    static void printPatient(const Patient& patient) {
        cout << "ID: " << patient.id << " | " << patient.name << " | Age: " << patient.age
             << " | " << patient.gender << " | " << patient.department
             << " | " << patient.condition << '\n';
    }
    Patient readPatient(int id) {
        Patient patient;
        patient.id = id;
        patient.name = Input::text("Patient name: ");
        patient.age = Input::number("Age (0-130): ", 0, 130);
        int gender = Input::number("Gender: 1. Female  2. Male  3. Other/unspecified: ", 1, 3);
        patient.gender = gender == 1 ? "Female" : gender == 2 ? "Male" : "Other/unspecified";
        patient.department = Input::text("Department: ");
        patient.condition = Input::text("Medical condition: ");
        return patient;
    }
    void addPatient() {
        if (state.count == capacity) { cout << "Patient array is full.\n"; return; }
        int id = Input::number("New Patient ID: ", 1, 999999);
        if (index.contains(id)) { cout << "Patient ID already exists.\n"; return; }
        Patient patient = readPatient(id);
        remember("Add patient " + to_string(id));
        state.patients[state.count++] = patient;
        index.insert(id);
        cout << "Patient added to array and BST.\n";
    }
    void editPatient() {
        int id = Input::number("Patient ID to update: ", 1, 999999);
        int position = patientPosition(id);
        if (position < 0) { cout << "Patient not found.\n"; return; }
        printPatient(state.patients[position]);
        Patient updated = readPatient(id);
        remember("Update patient " + to_string(id));
        state.patients[position] = updated;
        cout << "Patient updated.\n";
    }
    void deletePatient() {
        int id = Input::number("Patient ID to delete: ", 1, 999999);
        int position = patientPosition(id);
        if (position < 0) { cout << "Patient not found.\n"; return; }
        for (const auto& appointment : state.appointments)
            if (appointment.patientID == id) {
                cout << "Cancel this patient's appointments before deleting the patient.\n";
                return;
            }
        if (Input::number("Confirm deletion? 1. Yes  2. No: ", 1, 2) == 2) return;
        remember("Delete patient " + to_string(id));
        for (int i = position; i < state.count - 1; ++i) state.patients[i] = state.patients[i + 1];
        state.patients[--state.count] = Patient{};
        index.erase(id);
        cout << "Patient deleted from array and BST.\n";
    }
    bool conflict(const Appointment& proposed, int excludedID = -1) const {
        for (const auto& existing : state.appointments)
            if (existing.id != excludedID && existing.date == proposed.date && existing.time == proposed.time &&
                (existing.doctorID == proposed.doctorID || existing.patientID == proposed.patientID))
                return true;
        return false;
    }
    void showDoctors() const {
        for (const auto& doctor : doctors)
            cout << doctor.id << ". " << doctor.name << " - " << doctor.department << '\n';
    }
    void addAppointment() {
        int patientID = Input::number("Patient ID: ", 1, 999999);
        if (!index.contains(patientID)) { cout << "Register the patient first.\n"; return; }
        showDoctors();
        int doctorID = Input::number("Doctor ID: ", 1, static_cast<int>(doctors.size()));
        string date = Input::date(), time = Input::time();
        Appointment appointment{nextAppointmentID, patientID, doctorID, date, time, ""};
        if (conflict(appointment)) { cout << "Booking rejected: doctor or patient already booked in this slot.\n"; return; }
        appointment.reason = Input::text("Appointment reason: ");
        remember("Book appointment " + to_string(appointment.id));
        state.appointments.push_back(appointment);
        ++nextAppointmentID;
        cout << "Appointment booked. ID: " << appointment.id << '\n';
    }
    void showAppointments(int patientID = 0) const {
        vector<Appointment> sorted;
        for (const auto& appointment : state.appointments)
            if (patientID == 0 || appointment.patientID == patientID) sorted.push_back(appointment);
        sort(sorted.begin(), sorted.end(), [](const Appointment& a, const Appointment& b) {
            if (a.date != b.date) return a.date < b.date;
            if (a.time != b.time) return a.time < b.time;
            return a.id < b.id;
        });
        if (sorted.empty()) { cout << "No appointments found.\n"; return; }
        for (const auto& appointment : sorted) {
            int position = patientPosition(appointment.patientID);
            cout << "Appointment #" << appointment.id << " | " << appointment.date << ' ' << appointment.time
                 << " | Patient " << appointment.patientID << " (" << state.patients[position].name << ")"
                 << " | " << doctors[appointment.doctorID - 1].name << " | " << appointment.reason << '\n';
        }
    }
    void changeAppointment(bool cancel) {
        int id = Input::number("Appointment ID: ", 1, 999999);
        auto found = find_if(state.appointments.begin(), state.appointments.end(),
                             [id](const Appointment& appointment) { return appointment.id == id; });
        if (found == state.appointments.end()) { cout << "Appointment not found.\n"; return; }
        if (cancel) {
            if (Input::number("Confirm cancellation? 1. Yes  2. No: ", 1, 2) == 2) return;
            remember("Cancel appointment " + to_string(id));
            state.appointments.erase(found);
            cout << "Appointment cancelled.\n";
        } else {
            Appointment updated = *found;
            updated.date = Input::date();
            updated.time = Input::time();
            if (conflict(updated, id)) { cout << "Rescheduling rejected: doctor or patient has a conflicting appointment.\n"; return; }
            remember("Reschedule appointment " + to_string(id));
            *found = updated;
            cout << "Appointment rescheduled.\n";
        }
    }
    void undoLast() {
        if (undo.empty()) { cout << "No operations available to undo.\n"; return; }
        cout << "Undone: " << undo.top().description << '\n';
        state = move(undo.top().previous);
        undo.pop();
        rebuildIndex();
    }
    void patientMenu() {
        for (;;) {
            cout << "\n--- PATIENT MANAGEMENT ---\n1. Display array\n2. Add patient\n3. Update patient\n4. Delete patient\n5. Search using BST\n6. BST in-order display\n0. Back\n";
            int choice = Input::number("Choose: ", 0, 6);
            if (!choice) return;
            if (choice == 1) {
                if (!state.count) cout << "No patient records.\n";
                for (int i = 0; i < state.count; ++i) printPatient(state.patients[i]);
            } else if (choice == 2) addPatient();
            else if (choice == 3) editPatient();
            else if (choice == 4) deletePatient();
            else if (choice == 5) {
                int id = Input::number("Patient ID: ", 1, 999999);
                if (index.contains(id)) printPatient(state.patients[patientPosition(id)]);
                else cout << "Patient not found.\n";
            } else {
                auto ids = index.sortedIDs();
                if (ids.empty()) cout << "BST is empty.\n";
                for (int id : ids) printPatient(state.patients[patientPosition(id)]);
            }
        }
    }
    void appointmentMenu() {
        for (;;) {
            cout << "\n--- APPOINTMENT MANAGEMENT ---\n1. Book appointment\n2. All appointments (chronological)\n3. Patient appointments\n4. Reschedule\n5. Cancel\n6. Doctor directory\n0. Back\n";
            int choice = Input::number("Choose: ", 0, 6);
            if (!choice) return;
            if (choice == 1) addAppointment();
            else if (choice == 2) showAppointments();
            else if (choice == 3) {
                int id = Input::number("Patient ID: ", 1, 999999);
                if (!index.contains(id)) cout << "Patient not found.\n";
                else showAppointments(id);
            } else if (choice == 4) changeAppointment(false);
            else if (choice == 5) changeAppointment(true);
            else showDoctors();
        }
    }
public:
    HospitalSystem() {
        state.patients = {{
            {1006,"Amir",32,"Male","Cardiology","Hypertension"},
            {1003,"Mei Ling",24,"Female","General Medicine","Migraine"},
            {1009,"Ravi",45,"Male","Surgery","Knee injury"},
            {1001,"Aisha",29,"Female","Neurology","Headache"},
            {1005,"Daniel",61,"Male","Cardiology","Chest discomfort"},
            {1008,"Siti",18,"Female","General Medicine","Asthma"},
            {1002,"Jason",40,"Male","General Medicine","Fever"},
            {1010,"Priya",36,"Female","Surgery","Follow-up"},
            {1004,"Farah",52,"Female","Cardiology","Hypertension"},
            {1007,"Wei Han",27,"Male","Neurology","Dizziness"}
        }};
        state.count = 10;
        rebuildIndex();
    }
    void run() {
        for (;;) {
            cout << "\n==================================================\n"
                 << "           HOSPITAL MANAGEMENT SYSTEM\n"
                 << "==================================================\n"
                 << "Patients: " << state.count << '/' << capacity
                 << " | Appointments: " << state.appointments.size() << '\n'
                 << "1. Patient management (Array + BST)\n2. Appointment management\n"
                 << "3. Undo last change\n4. Show latest operation\n0. Exit\n";
            int choice = Input::number("Choose: ", 0, 4);
            if (!choice) { cout << "Session ended. Data is stored in memory only.\n"; return; }
            if (choice == 1) patientMenu();
            else if (choice == 2) appointmentMenu();
            else if (choice == 3) undoLast();
            else if (undo.empty()) cout << "No operations available to undo.\n";
            else cout << "Latest operation: " << undo.top().description << '\n';
        }
    }
};

int main() {
    try {
        HospitalSystem hospital;
        hospital.run();
    } catch (const runtime_error& error) {
        cout << '\n' << error.what() << " Session ended.\n";
    }
    return 0;
}
