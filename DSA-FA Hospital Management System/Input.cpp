// Input.cpp
// Reusable whole-line input and calendar/time validation.
// DSA Final Assessment - Hospital Management System.

#include "Input.h"
#include "ConsoleUI.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace std;

// Read an entire nonempty trimmed line; report end-of-input without an infinite retry loop.
string Input::text(const string& prompt, size_t maximumLength, bool allowCancel) {
        for (;;) {
            cout << "    " << ConsoleUI::tint("\033[36m") << "> "
                 << ConsoleUI::tint("\033[0m") << (prompt == "Choose: " ? "Select an option: " : prompt);
            string value;
            if (!getline(cin, value)) throw runtime_error("Input closed.");
            const auto first = value.find_first_not_of(" \t\r");
            if (first == string::npos) { cout << "This field cannot be empty.\n"; continue; }
            value = value.substr(first, value.find_last_not_of(" \t\r") - first + 1);
            if (allowCancel && value == "0") {
                // Explicit Back prompts share the same destination as menu Back options.
                string navigationPrompt = prompt;
                transform(navigationPrompt.begin(), navigationPrompt.end(), navigationPrompt.begin(),
                          [](unsigned char c) { return static_cast<char>(tolower(c)); });
                if (navigationPrompt.find("0. back") != string::npos ||
                    navigationPrompt.find("0 to back") != string::npos ||
                    navigationPrompt.find("main menu") != string::npos) throw MainMenuRequested{};
                throw InputCancelled{};
            }
            if (!validText(value, maximumLength)) {
                cout << "Enter at most " << maximumLength << " bytes of text without control characters.\n";
                continue;
            }
            return value;
        }
    }

// Accept only a complete integer within bounds; reject trailing letters and overflow.
int Input::number(const string& prompt, int minimum, int maximum) {
        for (;;) {
            string value = text(prompt, 500, prompt != "Choose: ");
            istringstream stream(value);
            int result;
            char extra;
            if (all_of(value.begin(), value.end(), [](char c) { return c >= '0' && c <= '9'; }) &&
                (stream >> result) && !(stream >> extra) && result >= minimum && result <= maximum)
                return result;
            cout << "Enter a whole number from " << minimum << " to " << maximum << ".\n";
        }
    }

// Check YYYY-MM-DD syntax, month lengths and Gregorian leap-year rules.
bool Input::validDate(const string& date) {
        if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
        for (size_t i = 0; i < date.size(); ++i)
            if (i != 4 && i != 7 && (date[i] < '0' || date[i] > '9')) return false;
        int year = stoi(date.substr(0, 4)), month = stoi(date.substr(5, 2)), day = stoi(date.substr(8, 2));
        if (year < 1900 || month < 1 || month > 12) return false;
        int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
        if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0)) days[1] = 29;
        return day >= 1 && day <= days[month - 1];
    }

// Accept half-hour appointment times from 00:00 through 23:30.
bool Input::validTime(const string& time) {
        if (time.size() != 5 || time[2] != ':') return false;
        for (size_t i = 0; i < time.size(); ++i)
            if (i != 2 && (time[i] < '0' || time[i] > '9')) return false;
        int hour = stoi(time.substr(0, 2)), minute = stoi(time.substr(3, 2));
        return hour >= 0 && hour < 24 && (minute == 0 || minute == 30);
    }

// Repeat the date prompt until the entered calendar date is valid.
string Input::date() {
        for (;;) {
            string value = text("Date (YYYY-MM-DD): ");
            if (validDate(value)) return value;
            cout << "Enter a real calendar date.\n";
        }
    }

// Repeat the time prompt until it is a valid half-hour slot.
string Input::time() {
        for (;;) {
            string value = text("Time (HH:MM, 00:00-23:30, 30-minute slots): ");
            if (validTime(value)) return value;
            cout << "Select a valid 30-minute clinic slot.\n";
        }
    }

int Input::patientID(const string& prompt) {
    for (;;) {
        string value = text(prompt);
        if (!value.empty() && (value[0] == 'P' || value[0] == 'p')) value.erase(0, 1);
        if (!value.empty() && value.size() <= 6 &&
            all_of(value.begin(), value.end(), [](char c) { return c >= '0' && c <= '9'; })) {
            int id = stoi(value);
            if (id >= 1 && id <= 999999) return id;
        }
        cout << "Enter a patient ID from P001 to P999999.\n";
    }
}

// Preserve Unicode text and punctuation, but reject blank fields and terminal controls.
bool Input::validText(const string& value, size_t maximumLength) {
    return !value.empty() && value.size() <= maximumLength &&
        value.find_first_not_of(" ") != string::npos &&
        all_of(value.begin(), value.end(), [](unsigned char c) { return c >= 32 && c != 127; });
}

// Names may contain Unicode, spaces, apostrophes, hyphens and other punctuation.
string Input::name(const string& prompt, bool allowCancel) {
    for (;;) {
        string value = text(prompt, 100);
        if (allowCancel && value == "0") return value;
        bool letter = false, digit = false;
        for (unsigned char c : value) {
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c >= 128) letter = true;
            if (c >= '0' && c <= '9') digit = true;
        }
        if (letter && !digit) return value;
        cout << "Name must contain letters and cannot contain digits.\n";
    }
}

int Input::doctorID(const string& prompt) {
    for (;;) {
        string value = text(prompt);
        if (!value.empty() && (value[0] == 'D' || value[0] == 'd')) value.erase(0, 1);
        if (!value.empty() && value.size() <= 6 &&
            all_of(value.begin(), value.end(), [](char c) { return c >= '0' && c <= '9'; })) {
            int id = stoi(value);
            if (id >= 1 && id <= 999999) return id;
        }
        cout << "Enter a doctor ID from D001 to D999999.\n";
    }
}

int Input::entityID(const string& prompt, const string& prefix) {
    for (;;) {
        string value=text(prompt);
        string upper=value;
        transform(upper.begin(),upper.end(),upper.begin(),[](unsigned char c){return static_cast<char>(toupper(c));});
        if(upper.rfind(prefix,0)==0) value.erase(0,prefix.size());
        if(!value.empty() && value.size()<=6 && all_of(value.begin(),value.end(),[](char c){return c>='0' && c<='9';})) {
            int id=stoi(value);
            if(id>=1 && id<=999999) return id;
        }
        cout << "Enter a valid " << prefix << " ID, e.g. " << prefix << "001 (0 to cancel).\n";
    }
}
