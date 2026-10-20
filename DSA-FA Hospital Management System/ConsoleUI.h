// Shared console presentation; plain-text fallback for redirected output.
#pragma once
#include <cstdlib>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace ConsoleUI {
inline bool colorEnabled() {
    static const bool enabled = [] {
        if (std::getenv("NO_COLOR")) return false;
#ifdef _WIN32
        HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        return GetConsoleMode(output, &mode) &&
            SetConsoleMode(output, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#else
        return isatty(STDOUT_FILENO) != 0;
#endif
    }();
    return enabled;
}
inline const char* tint(const char* code) { return colorEnabled() ? code : ""; }

// Shared width accommodates the longer service-selection prompts.
constexpr size_t contentWidth = 100;
inline size_t displayWidth(const std::string& value) {
    size_t width = 0;
    for (size_t i = 0; i < value.size();) {
        unsigned char c = static_cast<unsigned char>(value[i]);
        size_t count = c < 128 ? 1 : (c & 0xe0) == 0xc0 ? 2 : (c & 0xf0) == 0xe0 ? 3 : 4;
        if (i + count > value.size()) count = 1;
        unsigned int code = count == 1 ? c : c & (0x7f >> count);
        for (size_t j = 1; j < count; ++j) code = (code << 6) | (static_cast<unsigned char>(value[i+j]) & 0x3f);
        bool combining = (code >= 0x300 && code <= 0x36f);
        bool wide = (code >= 0x1100 && code <= 0x115f) || (code >= 0x2e80 && code <= 0xa4cf) ||
                    (code >= 0xac00 && code <= 0xd7a3) || (code >= 0xf900 && code <= 0xfaff) ||
                    (code >= 0xff01 && code <= 0xff60) || (code >= 0x1f300 && code <= 0x1faff) ||
                    (code >= 0x20000 && code <= 0x3ffff);
        width += combining ? 0 : wide ? 2 : 1;
        i += count;
    }
    return width;
}
inline int frameDepth = 0;
inline void rule() { if (frameDepth) return; std::cout << "  +" << std::string(contentWidth + 2, '-') << "+\n"; }
// Divide a heading from its content while retaining the panel's outer edges.
inline void divider() { std::cout << "  +" << std::string(contentWidth + 2, '-') << "+\n"; }
// Nested detail panels share their parent record's border.
struct RecordBox {
    int parentDepth;
    // Independent records retain their own borders even inside a patient profile.
    explicit RecordBox(bool independent = false) : parentDepth(frameDepth) {
        if (independent) frameDepth = 0;
        rule(); ++frameDepth;
    }
    ~RecordBox() { --frameDepth; rule(); frameDepth = parentDepth; }
    RecordBox(const RecordBox&) = delete;
    RecordBox& operator=(const RecordBox&) = delete;
};
inline void row(const std::string& value = "", bool centered = false) {
    if (value.empty() || value.find_first_not_of(" \t\r\n") == std::string::npos) return;
    // Split long lines at UTF-8 boundaries; no patient data is discarded.
    std::string chunk;
    auto emit = [&] {
        size_t remaining = contentWidth - displayWidth(chunk);
        size_t left = centered ? remaining / 2 : 0;
        std::cout << "  | " << std::string(left, ' ') << chunk
                  << std::string(remaining-left, ' ') << " |\n";
        chunk.clear();
    };
    for (size_t i = 0; i < value.size();) {
        unsigned char c = static_cast<unsigned char>(value[i]);
        size_t count = c < 128 ? 1 : (c & 0xe0) == 0xc0 ? 2 : (c & 0xf0) == 0xe0 ? 3 : 4;
        if (i + count > value.size()) count = 1;
        std::string next = value.substr(i, count);
        if (displayWidth(chunk + next) > contentWidth) {
            auto space=chunk.find_last_of(' ');
            if(space!=std::string::npos && space>0) {
                std::string remainder=chunk.substr(space+1);
                chunk.erase(space);
                emit();
                chunk=remainder;
            } else emit();
        }
        chunk += next;
        i += count;
    }
    emit();
}
// Full-width column section sharing the heading/detail panel's outer edges.
inline void table(const std::vector<std::string>& headers,
                  const std::vector<std::vector<std::string>>& records) {
    const int surroundingDepth=frameDepth;
    frameDepth=0;
    std::vector<size_t> widths;
    for (const auto& header : headers) widths.push_back(displayWidth(header));
    for (const auto& record : records)
        for (size_t i=0;i<headers.size();++i)
            if (i<record.size() && displayWidth(record[i])>widths[i]) widths[i]=displayWidth(record[i]);
    size_t gridWidth=1;
    for(size_t width : widths) gridWidth+=width+3;
    for(size_t i=0;!widths.empty() && gridWidth<contentWidth+4;++i,++gridWidth)
        ++widths[i%widths.size()];
    std::string border="+";
    for (size_t width : widths) border+=std::string(width+2,'-')+"+";
    auto cells=[&](const std::vector<std::string>& record) {
        std::string line="|";
        for(size_t i=0;i<headers.size();++i) {
            const std::string value=i<record.size() ? record[i] : "";
            const size_t padding=widths[i]-displayWidth(value);
            const size_t left=i==0 ? 0 : padding/2;
            line+=" "+std::string(left,' ')+value+std::string(padding-left,' ')+" |";
        }
        std::cout << "  " << line << '\n';
    };
    std::cout << "  " << border << '\n';
    cells(headers);
    std::cout << "  " << border << '\n';
    for(const auto& record : records) cells(record);
    std::cout << "  " << border << '\n';
    frameDepth=surroundingDepth;
}
inline void banner(const std::string& title, const std::string& subtitle = "") {
    if (frameDepth) row(); else std::cout << '\n';
    rule();
    row(title, true);
    if (!subtitle.empty()) row(subtitle, true);
    rule();
}
inline void section(const std::string& title) { rule(); row(title); rule(); }
// Capitalize menu words while retaining existing acronym spelling (BST/FIFO/CSV).
inline std::string menuLabel(std::string label) {
    bool wordStart = true;
    for (char& value : label) {
        unsigned char c = static_cast<unsigned char>(value);
        if (c < 128 && std::isalpha(c)) {
            if (wordStart) value = static_cast<char>(std::toupper(c));
            wordStart = false;
        } else if (std::isspace(c) || c == '(' || c == '/' || c == '-') {
            wordStart = true;
        }
    }
    return label;
}
inline std::string option(const std::string& label) {
    auto dot = label.find(". ");
    if (dot == std::string::npos) return menuLabel(label);
    return "[" + label.substr(0, dot) + "] " + menuLabel(label.substr(dot + 2));
}
inline void menu(const std::string& title, const std::vector<std::string>& options) {
    banner(title);
    row();
    for (const auto& label : options) {
        if (label.rfind("0. ", 0) == 0) { row(); rule(); }
        row(option(label));
    }
    rule();
    std::cout << "    Enter 0 at any input to cancel.\n\n";
}
inline void dashboard(int patients, int capacity, size_t doctors, size_t appointments, size_t waiting) {
    banner("HOSPITAL MANAGEMENT SYSTEM");
    std::ostringstream stats;
    stats << "Patients: " << patients << '/' << capacity << "     Doctors: " << doctors
          << "     Appointments: " << appointments;
    row(stats.str());
    row("Waiting patients: " + std::to_string(waiting) + "     Autosave: ON");
    rule();
    row();
    row(option("[1] Patient management"));
    row(option("[2] Doctor management"));
    row(option("[3] Appointment management"));
    row(option("[4] Service queues"));
    row(option("[5] Clinical visits & pharmacy"));
    row(option("[6] Billing & payments"));
    row(option("[7] Reports & algorithms"));
    row(option("[8] Data & session"));
    row();
    rule();
    row(option("[0] Exit"));
    rule();
    std::cout << '\n';
}
inline bool savedNoticePending = false;
inline void saveNotice() {
    if (savedNoticePending) {
        row("[OK] Autosaved.");
        savedNoticePending = false;
    }
}
inline void message(const std::string& kind, const std::string& text) {
    RecordBox frame;
    saveNotice();
    row("[" + kind + "] " + text);
}
inline void field(const std::string& label, const std::string& value) { row(label + ": " + value); }
// Use the same frame for existing record output, retaining its stream formatting.
template<class Writer> inline void panel(Writer writer) {
    std::ostringstream buffer;
    writer(buffer);
    std::istringstream lines(buffer.str());
    std::string value;
    RecordBox frame;
    saveNotice();
    while (std::getline(lines, value)) if (!value.empty()) row(value);
}
}
