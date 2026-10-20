cmake_minimum_required(VERSION 3.16)
# Build the application from every implementation module.
project(DSA_FA LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(MODULES HospitalSystem.cpp PatientBST.cpp Input.cpp MedicalHistory.cpp
    ServiceQueue.cpp ReferralGraph.cpp Algorithms.cpp HospitalLifecycle.cpp Persistence.cpp ScheduleBook.cpp EmergencyQueue.cpp PharmacyInventory.cpp PrescriptionManager.cpp BillingSystem.cpp HospitalCommerce.cpp)
add_executable(HospitalManagement main.cpp ${MODULES})
if(MSVC)
    target_compile_options(HospitalManagement PRIVATE /W4)
else()
    target_compile_options(HospitalManagement PRIVATE -Wall -Wextra -pedantic)
endif()
