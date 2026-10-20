// ScheduleBook.h
// Doctor shift rules and conversion between times and half-hour slots.
// DSA Final Assessment - Hospital Management System.

#pragma once
#include "Models.h"
#include <vector>
class ScheduleBook {
public:
    static int weekday(const std::string& date);
    static std::string dayName(int day);
    static bool validShiftDay(const std::string& day);
    static bool matches(const std::string& shiftDay, const std::string& date);
    // Convert HH:MM into a half-hour index; allow 24:00 only as a shift end.
    static int slot(const std::string& time, bool allowEnd = false);
    // Format a half-hour index as a zero-padded HH:MM string.
    static std::string time(int slot);
    // Only Scheduled and Checked-in appointments block a booking slot.
    static bool active(const Appointment& appointment);
    // Check the entered doctor/date shifts; shift start is inclusive and end is exclusive.
    static bool working(const std::vector<DoctorShift>& shifts, int doctorID,
                        const std::string& date, const std::string& time);
};
