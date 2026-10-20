// Persistence.h
// Validated, versioned autosave with a temporary file and previous-state backup.
// DSA Final Assessment - Hospital Management System.

#pragma once
#include "HospitalData.h"
#include <filesystem>
class Persistence {
    std::filesystem::path directory;
    std::filesystem::path filename;
public:
    // Keep the selected data directory; no records are written by construction.
    explicit Persistence(std::filesystem::path directory, std::filesystem::path filename = "hospital.db");
    // Check whether the main hospital database already exists.
    bool exists() const;
    // Parse into temporary state; validate IDs, references, statuses and sections before returning it.
    SavedHospital load() const;
    // Write and close a temporary database, back up the old file, then replace the main file.
    void save(const SavedHospital& hospital, const std::string& report = "") const;
    // Return the main save path for startup messages and error reporting.
    std::string path() const;
    // Format current time as Malaysia time (UTC+8), independent of host timezone.
    static std::string timestamp();
};
