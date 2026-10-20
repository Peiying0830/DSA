// ScheduleBook.cpp
// Doctor shift rules and conversion between times and half-hour slots.
// DSA Final Assessment - Hospital Management System.

#include "ScheduleBook.h"
#include "Input.h"
#include <iomanip>
#include <sstream>
using namespace std;

// Convert HH:MM into a half-hour index; allow 24:00 only as a shift end.
int ScheduleBook::slot(const string& value, bool allowEnd) {
    if (allowEnd && value == "24:00") return 48;
    if (value.size()!=5 || value[2]!=':') return -1;
    for (int i : {0,1,3,4}) if (value[i]<'0' || value[i]>'9') return -1;
    int h=stoi(value.substr(0,2)), m=stoi(value.substr(3,2));
    if (h>23 || (m!=0 && m!=30)) return -1;
    return h*2+m/30;
}
// Format a half-hour index as a zero-padded HH:MM string.
string ScheduleBook::time(int slot) {
    ostringstream out;
    out << setfill('0') << setw(2) << slot/2 << ':' << setw(2) << (slot%2)*30;
    return out.str();
}
// Only Scheduled and Checked-in appointments block a booking slot.
bool ScheduleBook::active(const Appointment& appointment) {
    return appointment.status=="Scheduled" || appointment.status=="Checked-in";
}
// Check the entered doctor/date shifts; shift start is inclusive and end is exclusive.
bool ScheduleBook::working(const vector<DoctorShift>& shifts, int doctorID,
                           const string& date, const string& time) {
    int value=slot(time);
    if (value<0) return false;
    for (const auto& shift : shifts)
        if (shift.doctorID==doctorID && matches(shift.date,date) && value>=slot(shift.start) && value<slot(shift.end,true)) return true;
    return false;
}

// Gregorian weekday calculation; Monday=1 through Sunday=7, independent of timezone.
int ScheduleBook::weekday(const string& date) {
    if (!Input::validDate(date)) return 0;
    int year=stoi(date.substr(0,4)), month=stoi(date.substr(5,2)), day=stoi(date.substr(8,2));
    static const int offsets[] = {0,3,2,5,0,3,5,1,4,6,2,4};
    if (month<3) --year;
    int sundayZero=(year+year/4-year/100+year/400+offsets[month-1]+day)%7;
    return sundayZero==0 ? 7 : sundayZero;
}
string ScheduleBook::dayName(int day) {
    static const string names[] = {"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"};
    return day>=1 && day<=7 ? names[day-1] : "";
}
bool ScheduleBook::validShiftDay(const string& day) {
    if (Input::validDate(day)) return true; // Retain previously saved date-specific shifts.
    for (int i=1;i<=7;++i) if (day==dayName(i)) return true;
    return false;
}
bool ScheduleBook::matches(const string& shiftDay, const string& date) {
    int day=weekday(date);
    return day!=0 && (shiftDay==date || shiftDay==dayName(day));
}
