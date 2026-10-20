@echo off
rem Compile all source modules together; headers declare interfaces but are not compiled separately.
pushd "%~dp0"
rem C++17 enables filesystem and unique_ptr facilities; warning flags help catch mistakes.
g++ -std=c++17 -Wall -Wextra -pedantic -g main.cpp HospitalSystem.cpp PatientBST.cpp Input.cpp MedicalHistory.cpp ServiceQueue.cpp ReferralGraph.cpp Algorithms.cpp HospitalLifecycle.cpp Persistence.cpp ScheduleBook.cpp EmergencyQueue.cpp PharmacyInventory.cpp PrescriptionManager.cpp BillingSystem.cpp HospitalCommerce.cpp -o HospitalManagement.exe
if errorlevel 1 (
    popd
    exit /b 1
)
echo Built HospitalManagement.exe. Run .\HospitalManagement.exe
popd
