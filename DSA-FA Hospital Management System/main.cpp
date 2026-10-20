// main.cpp
// Program entry point; locate storage and start the hospital application.
// DSA Final Assessment - Hospital Management System.

#include "HospitalSystem.h"
#include <iostream>
#include <stdexcept>
#include <filesystem>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
using namespace std;

// Initialise the application and handle startup/input errors at the program boundary.
int main(int argc, char* argv[]) {
    try {
        auto executable = filesystem::absolute(argv[0]);
#ifdef _WIN32
        wchar_t buffer[32768];
        DWORD length=GetModuleFileNameW(nullptr,buffer,32768);
        if (!length || length>=32768) throw runtime_error("Cannot determine application folder.");
        executable=filesystem::path(buffer);
#endif
        auto dataDirectory=executable.parent_path()/"data";
        if (argc==3 && string(argv[1])=="--data-dir") dataDirectory=filesystem::absolute(argv[2]);
        else if (argc!=1) { cerr << "Usage: HospitalManagement [--data-dir directory]\n"; return 1; }
        HospitalSystem hospital(dataDirectory);
        hospital.run();
    } catch (const exception& error) {
        cout << '\n' << error.what() << " Session ended.\n";
        if (string(error.what())!="Input closed.") return 1;
    }
    return 0;
}
