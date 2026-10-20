using namespace std;

// Included once by HospitalSystem.cpp. Prices are entered by staff, in exact cents.
namespace {
long long serviceReadPrice(const string& prompt) {
    for(;;) {long long cents=0;if(BillingSystem::parsePrice(Input::text(prompt),cents)) return cents;ConsoleUI::message("!","Enter RM 0.01 to RM 1000000.00, at most two decimals.");}
}
}
void HospitalSystem::showServices() const {
    ConsoleUI::banner("SERVICES & PRICES");
    for(const auto& service:state.services) {
        ConsoleUI::RecordBox frame;
        ConsoleUI::row(formatID("S",service.id)+" | "+service.name);
        ConsoleUI::field("Price",BillingSystem::money(service.priceCents));
    }
    if(state.services.empty()) ConsoleUI::message("INFO","No services configured.");
}
void HospitalSystem::servicesMenu() {
    for(;;) {
        try {
            ConsoleUI::menu("SERVICES",{"1. Services and prices","2. Update service prices","0. Back to main menu"});
            int choice=Input::number("Choose: ",0,2);
            if (!choice) throw MainMenuRequested{};
            if(choice==1) visitServicesMenu();
            else servicePricesMenu();
        } catch(const InputCancelled&) {ConsoleUI::message("INFO","Operation cancelled. Nothing saved.");}
    }
}
void HospitalSystem::clinicalMenu() {
    for(;;) {
        try {
            ConsoleUI::menu("CLINICAL VISITS & PHARMACY",{"1. Visit medical records (Linked List)","2. Services and prescriptions","3. Pharmacy and dispensing","4. Update service prices","5. Add new service","0. Back to main menu"});
            int choice=Input::number("Choose: ",0,5);
            if (!choice) throw MainMenuRequested{};
            if(choice==1) visitRecordsMenu();
            else if(choice==2) treatmentOrdersMenu();
            else if(choice==3) pharmacyMenu();
            else if(choice==4) servicePricesMenu();
            else addService();
        } catch(const InputCancelled&) {ConsoleUI::message("INFO","Operation cancelled. Nothing saved.");}
    }
}
void HospitalSystem::treatmentOrdersMenu() {
    for(;;) {
        try {
            ConsoleUI::menu("SERVICES & PRESCRIPTIONS",{"1. Manage visit services","2. Prescriptions","0. Back to main menu"});
            int choice=Input::number("Choose: ",0,2);
            if (!choice) throw MainMenuRequested{};
            if(choice==1) visitServicesMenu();
            else prescriptionMenu();
        } catch(const InputCancelled&) {ConsoleUI::message("INFO","Operation cancelled. Nothing saved.");}
    }
}
void HospitalSystem::visitServicesMenu() {
    showServices();
    bool any=false;
    for(const auto& visit:state.appointments) if(visit.status=="Checked-in" || visit.status=="Completed") {printAppointment(visit);any=true;}
    if(!any) {ConsoleUI::message("INFO","No checked-in or completed visits. Check in an appointment first.");return;}
    int id=Input::entityID("Appointment ID to manage (0 to return to main menu): ","A");
    auto visit=find_if(state.appointments.begin(),state.appointments.end(),[&](const Appointment& a){return a.id==id && (a.status=="Checked-in" || a.status=="Completed");});
    if(visit==state.appointments.end()) {ConsoleUI::message("!","Select a checked-in or completed visit.");return;}
    printAppointment(*visit);
    showVisitServices(id);
    for(;;) {
        try {
            ConsoleUI::menu("VISIT SERVICES - "+formatID("A",id),{"1. Add service","2. View selected services","3. Remove service","0. Back to main menu"});
            int choice=Input::number("Choose: ",0,3);
            if (!choice) throw MainMenuRequested{};
            if(choice==1) addVisitService(id);
            else if(choice==2) showVisitServices(id);
            else removeVisitService(id);
        } catch(const InputCancelled&) {ConsoleUI::message("INFO","Operation cancelled. Nothing saved.");}
    }
}
void HospitalSystem::servicePricesMenu() {
    for(;;) {
        showServices();
        try { changeServicePrice(); }
        catch(const InputCancelled&) { return; }
    }
}
void HospitalSystem::addVisitService(int id) {
    auto visit=find_if(state.appointments.begin(),state.appointments.end(),[&](const Appointment& a){return a.id==id && a.status=="Checked-in";});
    if(visit==state.appointments.end()) {ConsoleUI::message("!","Select a checked-in appointment.");return;}
    Invoice* invoice=nullptr;
    for(const auto& saved:state.billing.records()) if(saved.appointmentID==id) { invoice=state.billing.findMutable(saved.id); break; }
    if(!invoice) {ConsoleUI::message("!","Check in the appointment first so its consultation invoice can be created.");return;}
    if(invoice->status=="Paid") {ConsoleUI::message("!","This visit invoice is already paid. Add any required service before recording payment.");return;}
    showServices();
    int serviceID=Input::entityID("Service ID: ","S");
    auto service=find_if(state.services.begin(),state.services.end(),[&](const HospitalService& s){return s.id==serviceID;});
    if(service==state.services.end()) {ConsoleUI::message("!","Service not found.");return;}
    long long total=BillingSystem::total(*invoice);int count=0;
    for(const auto& order:state.visitServices) if(order.appointmentID==id) {
        if(order.serviceID==serviceID) {ConsoleUI::message("!","Service already added. Remove it first to change its quantity.");return;}
        ++count;
    }
    if(count>=19 || invoice->items.size()>=20 || nextServiceOrderID>999999 || state.visitServices.size()>=100000) {ConsoleUI::message("!","Service order limit reached.");return;}
    int quantity=Input::number("Quantity: ",1,100000);
    if(total+quantity*service->priceCents>BillingSystem::maxInvoiceCents) {ConsoleUI::message("!","Visit total exceeds RM 1000000.00. Nothing added.");return;}
    if(Input::number("Add this service to the visit? 1. Yes 2. No: ",1,2)==2) return;
    remember("Add service to appointment "+to_string(id));
    state.visitServices.push_back({nextServiceOrderID++,id,serviceID,quantity,service->name,service->priceCents});
    invoice->items.push_back({service->name,quantity,service->priceCents});
    if(commitChange()) {ConsoleUI::message("OK","Service added to invoice "+formatID("I",invoice->id)+". Amount updated automatically.");showVisitServices(id);showInvoice(*invoice);}
}
void HospitalSystem::showVisitServices(int id) const {
    ConsoleUI::RecordBox frame;
    ConsoleUI::row("SERVICES FOR "+formatID("A",id));
    bool any=false;long long total=0;
    for(const auto& order:state.visitServices) if(order.appointmentID==id) {
        any=true;
        ConsoleUI::row(formatID("V",order.id)+" | "+formatID("S",order.serviceID)+" | "+order.description);
        ConsoleUI::field("Quantity",to_string(order.quantity));
        ConsoleUI::field("Captured unit price",BillingSystem::money(order.unitPriceCents));
        ConsoleUI::field("Amount",BillingSystem::money(order.quantity*order.unitPriceCents));
        total+=order.quantity*order.unitPriceCents;
    }
    if(!any) ConsoleUI::row("No additional services selected.");
    ConsoleUI::field("Additional services total",BillingSystem::money(total));
}
void HospitalSystem::removeVisitService(int appointmentID) {
    auto visit=find_if(state.appointments.begin(),state.appointments.end(),[&](const Appointment& a){return a.id==appointmentID && a.status=="Checked-in";});
    if(visit==state.appointments.end()) {ConsoleUI::message("!","Only checked-in visits can be changed.");return;}
    Invoice* invoice=nullptr;
    for(const auto& saved:state.billing.records()) if(saved.appointmentID==appointmentID) { invoice=state.billing.findMutable(saved.id); break; }
    if(!invoice) {ConsoleUI::message("!","No consultation invoice exists for this visit.");return;}
    if(invoice->status=="Paid") {ConsoleUI::message("!","This visit invoice is already paid and cannot be changed.");return;}
    showVisitServices(appointmentID);
    int id=Input::entityID("Visit service ID to remove: ","V");
    auto order=find_if(state.visitServices.begin(),state.visitServices.end(),[&](const VisitService& o){return o.id==id && o.appointmentID==appointmentID;});
    if(order==state.visitServices.end()) {ConsoleUI::message("!","Visit service not found.");return;}
    auto billItem=find_if(invoice->items.begin(),invoice->items.end(),[&](const BillItem& item){
        int index=static_cast<int>(&item-invoice->items.data());
        for(const auto& rx:state.prescriptions.records())
            if(rx.invoiceID==invoice->id && index>=rx.invoiceItemOffset && index<rx.invoiceItemOffset+static_cast<int>(rx.items.size())) return false;
        return item.description==order->description && item.quantity==order->quantity && item.unitPriceCents==order->unitPriceCents;
    });
    if(billItem==invoice->items.end()) {ConsoleUI::message("!","Invoice service line was not found.");return;}
    remember("Remove visit service "+to_string(id));
    state.prescriptions.adjustInvoiceOffsetsAfterRemoval(invoice->id,static_cast<int>(billItem-invoice->items.begin()));
    invoice->items.erase(billItem); state.visitServices.erase(order);
    if(commitChange()) {ConsoleUI::message("OK","Visit service and its invoice charge removed.");showVisitServices(appointmentID);showInvoice(*invoice);}
}
void HospitalSystem::addService() {
    int id=1;
    for(const auto& service:state.services) id=max(id,service.id+1);
    if(id>999999 || state.services.size()>=100000) {
        ConsoleUI::message("!","Service catalog limit reached.");return;
    }
    string name=Input::text("New service name: ",100);
    auto normalized=[](string value) {
        transform(value.begin(),value.end(),value.begin(),[](unsigned char c){return static_cast<char>(tolower(c));});
        return value;
    };
    for(const auto& service:state.services) if(normalized(service.name)==normalized(name)) {
        ConsoleUI::message("!","Service already exists: "+formatID("S",service.id)+". Use its existing ID.");return;
    }
    long long cents=serviceReadPrice("Service price (RM): ");
    ConsoleUI::message("INFO",formatID("S",id)+" | "+name+" | "+BillingSystem::money(cents));
    if(Input::number("Add this service? 1. Yes 2. No: ",1,2)==2) return;
    remember("Add service "+to_string(id));
    state.services.push_back({id,name,cents});
    if(commitChange()) ConsoleUI::message("OK","Service "+formatID("S",id)+" added. It is now available for visit services.");
}
void HospitalSystem::changeServicePrice() {
    int id=Input::entityID("Service ID to update (0 to return to main menu): ","S");
    auto service=find_if(state.services.begin(),state.services.end(),[&](const HospitalService& s){return s.id==id;});
    if(service==state.services.end()) {ConsoleUI::message("!","Service not found.");return;}
    long long cents=serviceReadPrice("New service price (RM): ");
    remember("Update service price "+to_string(id));service->priceCents=cents;
    if(commitChange()) ConsoleUI::message("OK","Service price updated. Existing visit charges keep their captured price.");
}
void HospitalSystem::updateDoctorFee() {
    showDoctors();int id=Input::doctorID("Doctor ID: ");int pos=doctorPosition(id);
    if(pos<0) {ConsoleUI::message("!","Doctor not found.");return;}
    long long cents=serviceReadPrice("Consultation fee (RM): ");
    remember("Update consultation fee for doctor "+to_string(id));state.doctors[pos].consultationFeeCents=cents;
    if(commitChange()) ConsoleUI::message("OK","Consultation fee updated. Issued invoices keep their amounts.");
}
bool HospitalSystem::completeVisit(Appointment& appointment) {
    const Invoice* invoice=nullptr;
    for(const auto& saved:state.billing.records()) if(saved.appointmentID==appointment.id) { invoice=&saved; break; }
    if(!invoice) {ConsoleUI::message("!","No consultation invoice exists. Check in the appointment before completing the visit.");return false;}
    showInvoice(*invoice);
    if(Input::number("Complete this visit? 1. Yes 2. No: ",1,2)==2) return false;
    remember("Complete appointment "+to_string(appointment.id));
    appointment.status="Completed";
    if(!commitChange()) return false;
    ConsoleUI::message("OK","Appointment completed. Invoice "+formatID("I",invoice->id)+" remains available in Billing & Payments.");return true;
}
