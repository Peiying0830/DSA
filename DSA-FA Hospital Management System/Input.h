// Input.h
// Reusable whole-line input and calendar/time validation.
// DSA Final Assessment - Hospital Management System.

#pragma once
#include <string>
using std::string;

struct InputCancelled {};
// Unwind nested menus directly to the dashboard.
struct MainMenuRequested {};

class Input {
public:
    // Read an entire nonempty trimmed line; report end-of-input without an infinite retry loop.
    static string text(const string& prompt, size_t maximumLength = 500, bool allowCancel = true);
    static bool validText(const string& value, size_t maximumLength = 500);
    static string name(const string& prompt, bool allowCancel = false);
    // Accept only a complete integer within bounds; reject trailing letters and overflow.
    static int entityID(const string& prompt, const string& prefix);
    static int number(const string& prompt, int minimum, int maximum);
    // Check YYYY-MM-DD syntax, month lengths and Gregorian leap-year rules.
    static bool validDate(const string& date);
    // Accept half-hour appointment times from 00:00 through 23:30.
    static bool validTime(const string& time);
    // Accept a P-prefixed or numeric patient ID within the supported range.
    static int patientID(const string& prompt = "Patient ID (e.g. P001): ");
    // Repeat the date prompt until the entered calendar date is valid.
    static int doctorID(const string& prompt = "Doctor ID (e.g. D001): ");
    static string date();
    // Repeat the time prompt until it is a valid half-hour slot.
    static string time();
};
