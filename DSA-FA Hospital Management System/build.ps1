$ErrorActionPreference = 'Stop'
# Use the project directory so all module paths resolve consistently.
Push-Location $PSScriptRoot
try {
    $sources = @('main.cpp', 'HospitalSystem.cpp', 'PatientBST.cpp', 'Input.cpp',
        'MedicalHistory.cpp', 'ServiceQueue.cpp', 'ReferralGraph.cpp', 'Algorithms.cpp', 'HospitalLifecycle.cpp', 'Persistence.cpp', 'ScheduleBook.cpp', 'EmergencyQueue.cpp', 'PharmacyInventory.cpp', 'PrescriptionManager.cpp', 'BillingSystem.cpp', 'HospitalCommerce.cpp')
    & g++ -std=c++17 -Wall -Wextra -pedantic @sources -o HospitalManagement.exe
    if ($LASTEXITCODE -ne 0) { throw 'Compilation failed.' }
    Write-Host 'Built HospitalManagement.exe. Run .\HospitalManagement.exe'
} finally {
    Pop-Location
}
