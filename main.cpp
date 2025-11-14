// File: main.cpp  (FULL)
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <limits>
#include <chrono>
#include <ctime>
#ifdef _WIN32
  #include <windows.h>
#endif
#include "Class/Ultil.h"
#include "Class/Brand.h"
#include "Class/Bus.h"
#include "Class/Driver.h"
#include "Class/Route.h"
#include "Class/Seat.h"
#include "Class/Ticket.h"
#include "Class/Trip.h"
#include "Class/User.h"
#include "Class/Auth.h"
#include "Class/ConsoleMenu.h"
#include "Class/Table.h" 

using namespace std;

// ---------- Helpers ----------
static string toLowerStr(string s) {
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c){ return (char)tolower(c); });
    return s;
}
static vector<string> splitCSV(const string &line) {
    vector<string> v; string t; stringstream ss(line);
    while (getline(ss, t, ',')) v.push_back(t);
    return v;
}
template <typename T>
static vector<T> loadData(const string &file, T (*loader)(const string&)) {
    vector<T> out; ifstream f(file); string line;
    while (getline(f, line)) if (!line.empty()) out.push_back(loader(line));
    return out;
}
static string nowStr() {
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    tm ltm;
#ifdef _WIN32
    localtime_s(&ltm, &t);
#else
    ltm = *localtime(&t);
#endif
    stringstream ss;
    ss << (1900+ltm.tm_year) << "-" << setfill('0') << setw(2) << (1+ltm.tm_mon)
       << "-" << setw(2) << ltm.tm_mday << " " << setw(2) << ltm.tm_hour
       << ":" << setw(2) << ltm.tm_min;
    return ss.str();
}
static void flushConsoleEvents() {
#ifdef _WIN32
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
#endif
}
static bool endsWithNoCase(const string& s, const string& suf) {
    if (s.size() < suf.size()) return false;
    for (size_t i=0;i<suf.size();++i)
        if (tolower(s[s.size()-suf.size()+i]) != tolower(suf[i])) return false;
    return true;
}
static string ensureSuffix(const string& s, const string& suf) {
    return endsWithNoCase(s, suf) ? s : (s + suf);
}

// ---------- Users (Create Account) ----------
static string nextUserId(const vector<User>& users) {
    int mx = 0;
    for (auto &u : users) {
        if (u.getId().size() >= 2 &&
            (u.getId()[0]=='U' || u.getId()[0]=='u')) {
            try { mx = max(mx, stoi(u.getId().substr(1))); } catch(...) {}
        }
    }
    stringstream ss; ss << "U" << setw(3) << setfill('0') << (mx+1);
    return ss.str();
}
static bool usernameExists(const vector<User>& users, const string& uname) {
    for (auto &u : users)
        if (toLowerStr(u.getUsername()) == toLowerStr(uname)) return true;
    return false;
}
static void createAccount(vector<User>& users) {
    ui::DrawPanel("CREATE ACCOUNT");
    flushConsoleEvents();

    string uname, pwd, confirm;
    while (true) {
        cout << "Username (>= 3 ky tu, khong dau): ";
        getline(cin, uname);
        if (uname.size() < 3) { cout << "Qua ngan. Thu lai.\n"; continue; }
        if (usernameExists(users, uname)) { cout << "Username da ton tai. Thu ten khac.\n"; continue; }
        break;
    }
    cout << "Password (>= 4 ky tu): "; getline(cin, pwd);
    cout << "Confirm  : "; getline(cin, confirm);
    if (pwd.size() < 4 || pwd != confirm) {
        cout << "Password khong hop le/khong trung. Huy tao tai khoan.\n";
        system("pause"); return;
    }

    string uid = nextUserId(users);
    users.push_back(User(uid, uname, pwd, Role::USER));

    ofstream fout("Data/User.txt");
    for (auto &u : users) fout << u.toCSV() << "\n";
    fout.close();

    cout << "\nTao tai khoan thanh cong!\n"
         << "  ID: " << uid << "\n"
         << "  Username: " << uname << "\n"
         << "  Role: USER\n\n";
    system("pause");
}

// ---------------- ROUTE MENU ----------------
static void menuRoute(vector<Route> &routes) {
    while (true) {
        int sel = ConsoleMenu::pick("ROUTE MANAGEMENT", {
            "View All Routes",
            "Search Route",
            "Add New Route",
            "Update Route",
            "Delete Route",
            "Back"
        });
        if (sel == -1 || sel == 5) break;

        if (sel == 0) { // View
            ui::DrawPanel("ALL ROUTES");
            ui::PrintRoutes(routes); 
            cout << "\n"; system("pause");
        }
        else if (sel == 1) { // Search
            ui::DrawPanel("SEARCH ROUTE");
            flushConsoleEvents();
            cout << "Keyword (e.g.: 'da nang - hue' or 'hue'): ";
            string key; getline(cin, key); key = toLowerStr(key);

            cout << "\n";
            vector<Route> found;
            for (const auto &r : routes) {
                string name = toLowerStr(r.getName() + " " + r.getStart() + " " + r.getEnd());
                if (name.find(key) != string::npos) found.push_back(r);
            }
            if (found.empty()) cout << "Error: Route name cannot be empty. Please try again.\n";
            else ui::PrintRoutes(found);
            cout << "\n"; system("pause");
        }
        else if (sel == 2) { // Add
            ui::DrawPanel("ADD NEW ROUTE");
            flushConsoleEvents();
            string name, start, end, dist, dur;

            while (true) {
                cout << "Enter Route Name (e.g., Da Nang - Hue): ";
                getline(cin, name);
                if (!name.empty()) break;
                cout << "Error: Route name cannot be empty. Please try again.\n";
            }
            while (true) {
                cout << "Enter Start Point: ";
                getline(cin, start);
                if (!start.empty()) break;
                cout << "Error: Start point cannot be empty. Please try again.\n";
            }
            while (true) {
                cout << "Enter End Point: ";
                getline(cin, end);
                if (!end.empty()) break;
                cout << "Error: End point cannot be empty. Please try again.\n";
            }
            while (true) {
                cout << "Enter Distance (in km, e.g., 100): ";
                getline(cin, dist);
                if (!dist.empty()) break;
                cout << "Error: Distance cannot be empty. Please try again.\n";
            }
            while (true) {
                cout << "Enter Duration (in minutes, e.g., 180): ";
                getline(cin, dur);
                if (!dur.empty()) break;
                cout << "Error: Duration cannot be empty. Please try again.\n";
            }

            string lastId = routes.empty() ? "R000" : routes.back().getId();
            int next = 0; try { next = stoi(lastId.substr(1)) + 1; } catch(...) {}
            stringstream ss; ss << "R" << setw(3) << setfill('0') << next;

            Route r(ss.str(), name, start, end, dist, dur);
            routes.push_back(r);
            Ultil<Route>::saveToFile("Data/Route.txt", routes);
            cout << "\nAdded route " << r.getId() << " successfully.\n\n";
            ui::PrintRoutes({r});
            cout << "\n"; system("pause");
        }
        else if (sel == 3) { // Update
            ui::DrawPanel("UPDATE ROUTE");
            flushConsoleEvents();
            cout << "Route ID (vd: R001): ";
            string id; getline(cin, id);

            bool ok = false;
            for (auto &r : routes) {
                if (toLowerStr(r.getId()) == toLowerStr(id)) {
                    ok = true;
                    cout << "Editing: " << r.getName() << "\n";
                    string name, start, end, dist, dur;
                    cout << "New Name     [" << r.getName()     << "]: "; getline(cin, name);
                    cout << "New Start    [" << r.getStart()    << "]: "; getline(cin, start);
                    cout << "New End      [" << r.getEnd()      << "]: "; getline(cin, end);
                    cout << "New Distance [" << r.getDistance() << "]: "; getline(cin, dist);
                    cout << "New Duration [" << r.getDuration() << "]: "; getline(cin, dur);

                    if (!name.empty()) r.setName(name);
                    if (!start.empty()) r.setStart(start);
                    if (!end.empty()) r.setEnd(end);
                    if (!dist.empty()) r.setDistance(dist);
                    if (!dur.empty()) r.setDuration(dur);

                    Ultil<Route>::saveToFile("Data/Route.txt", routes);
                    cout << "\nUpdated.\n\n";
                    ui::PrintRoutes({r});
                    break;
                }
            }
            if (!ok) cout << "Route ID not found.\n";
            cout << "\n"; system("pause");
        }
        else if (sel == 4) { // Delete
            ui::DrawPanel("DELETE ROUTE");
            flushConsoleEvents();
            cout << "Route ID (vd: R001): ";
            string id; getline(cin, id);

            bool erased = false; Route removed;
            for (auto it = routes.begin(); it != routes.end(); ++it) {
                if (toLowerStr(it->getId()) == toLowerStr(id)) {
                    removed = *it; routes.erase(it); erased = true; break;
                }
            }
            if (erased) {
                Ultil<Route>::saveToFile("Data/Route.txt", routes);
                cout << "Deleted.\n\nRemoved:\n";
                ui::PrintRoutes({removed});
            } else {
                cout << "Route ID not found.\n";
            }
            cout << "\n"; system("pause");
        }
    }
}

// ---------------- BUS MENU ----------------
static void menuBus(const vector<Brand> &brands, const vector<Bus> &buses,
                      const vector<Driver> &drivers, const vector<Seat> &seats) {
    ui::DrawPanel("BUS MANAGEMENT");
    cout << "Brands:\n";
    for (const auto &b : brands) cout << " - " << b.getName() << " (" << b.getId() << ")\n";

    cout << "\nEnter brand name: ";
    string input; flushConsoleEvents(); getline(cin, input);
    string key = toLowerStr(input);

    string brandId;
    for (const auto &b : brands) if (toLowerStr(b.getName()) == key) brandId = b.getId();
    if (brandId.empty()) { cout << "\nBrand not found.\n"; system("pause"); return; }

    vector<Bus> brandBuses;
    for (const auto &bus : buses) if (bus.getBrandId() == brandId) brandBuses.push_back(bus);

    cout << "\nBUSES OF BRAND:\n\n";
    if (brandBuses.empty()) { cout << "(No buses)\n\n"; system("pause"); return; }
    ui::PrintBuses(brandBuses);

    cout << "\nBus ID to view driver & seats: ";
    string id; getline(cin, id); id = toLowerStr(id);

    const Bus *chosen = nullptr;
    for (const auto& b : brandBuses) if (toLowerStr(b.getId()) == id) { chosen = &b; break; }
    if (!chosen) { cout << "\nBus not found.\n"; system("pause"); return; }

    vector<Driver> ds; for (auto &d : drivers) if (d.getBus() == chosen->getId()) ds.push_back(d);
    cout << "\n--- DRIVER INFO ---\n\n";
    if (ds.empty()) cout << "(No driver)\n";
    else ui::PrintDrivers(ds);

    vector<Seat> ss; for (auto &s : seats) if (s.getBusId() == chosen->getId()) ss.push_back(s);
    cout << "\n--- SEAT LIST ---\n\n";
    if (ss.empty()) cout << "(No seat file)\n";
    else ui::PrintSeats(ss);

    cout << "\n"; system("pause");
}


// ---------------- TICKET MENU ----------------
static void menuTicket(const vector<Ticket> &tickets, const vector<Trip> &trips,
                         const vector<Route> &routes, const vector<Bus> &buses,
                         const vector<Brand> &brands) {
    
    ui::DrawPanel("TICKET MANAGEMENT");
    cout << "Available Brands:\n";
    for (const auto &b : brands)
        cout << " - " << b.getName() << " (" << b.getId() << ")\n";

    cout << "\nEnter brand name to filter: ";
    string name; 
    flushConsoleEvents(); 
    getline(cin, name);

    const Brand *br = nullptr;
    for (const auto& x : brands) if (toLowerStr(x.getName()) == toLowerStr(name)) { br = &x; break; }
    if (!br) { cout << "\nBrand not found.\n"; system("pause"); return; }

    vector<string> busIds;
    for (const auto& b : buses) if (b.getBrandId() == br->getId()) busIds.push_back(b.getId());
    if (busIds.empty()) { cout << "\nNo buses for this brand.\n"; system("pause"); return; }

    while (true) {
        string panelTitle = "BRAND: " + br->getName();
        int sel = ConsoleMenu::pick(panelTitle,
                                    { "View VIP Tickets", "View Standard Tickets", "Back" });

        if (sel == -1 || sel == 2) break;

        string seatTypeToFilter = (sel == 0 ? "VIP" : "Standard");
        
        string resultTitle = "TICKETS: " + br->getName() + " (" + seatTypeToFilter + ")";
        ui::DrawPanel(resultTitle);

        vector<Ticket> tkFiltered;
        for (const auto &tk : tickets) {
            if (find(busIds.begin(), busIds.end(), tk.getBusId()) == busIds.end())
                continue;

            const Bus *bus = nullptr;
            for (const auto &b : buses)
                if (b.getId() == tk.getBusId()) { bus = &b; break; }

            if (bus && toLowerStr(bus->getType()) == toLowerStr(seatTypeToFilter)) {
                tkFiltered.push_back(tk);
            }
        }
        
        cout << "\nShowing all " << seatTypeToFilter << " tickets for brand: " << br->getName() << "\n\n";

        if (tkFiltered.empty()) {
            cout << "(No " << seatTypeToFilter << " tickets found for this brand)\n";
        } else {
            ui::PrintTickets(tkFiltered, trips, routes, buses);
        }

        cout << "\n"; 
        system("pause");
    }
}

// ---------------- BOOK / CANCEL ----------------
static void menuBooking(vector<Route>& routes, const vector<Trip>& trips,
                         const vector<Bus>& buses, vector<Seat>& seats,
                         vector<Ticket>& tickets) {
    int sel = ConsoleMenu::pick("BOOKING / CANCEL TICKET",
                                 { "Book new ticket", "Cancel ticket", "Back" });
    if (sel == -1 || sel == 2) return;

    if (sel == 0) { // BOOK
        ui::DrawPanel("BOOK NEW TICKET");
        flushConsoleEvents();

        string name, phone, from, to;
        int payChoice;
        cout << "Passenger name: "; getline(cin, name);
        if (name.empty()) { cout << "\nName cannot be empty.\n"; system("pause"); return; }
        cout << "Phone number : "; getline(cin, phone);
        if (phone.empty()) { cout << "\nPhone cannot be empty.\n"; system("pause"); return; }
        cout << "From           : "; getline(cin, from);
        cout << "To             : "; getline(cin, to);

        string routeId; const Route* bookedRoute = nullptr;
        for (const auto& r : routes)
            if (toLowerStr(r.getStart()) == toLowerStr(from) &&
                toLowerStr(r.getEnd())   == toLowerStr(to))
            { routeId = r.getId(); bookedRoute = &r; break; }
        
        if (routeId.empty()) { cout << "\nNo route found.\n"; system("pause"); return; }
        vector<const Trip*> foundTrips;
        for (const auto& t : trips) 
            if (toLowerStr(t.getRouteId()) == toLowerStr(routeId)) 
                foundTrips.push_back(&t);
        
        if (foundTrips.empty()) { cout << "\nNo trips for this route.\n"; system("pause"); return; }

        bool hasVip = false;
        bool hasStandard = false;
        for (const auto& t : foundTrips) {
            for (const auto& b : buses) {
                if (t->getBusId() == b.getId()) {
                    if (toLowerStr(b.getType()) == "vip") hasVip = true;
                    if (toLowerStr(b.getType()) == "standard") hasStandard = true;
                    break; 
                }
            }
            if (hasVip && hasStandard) break;
        }

        string seatType;
        int seatTypeChoice;

        if (hasVip && hasStandard) {
            cout << "Seat type (1=VIP, 0=Standard): ";
            cin >> seatTypeChoice; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            seatType = (seatTypeChoice == 1 ? "VIP" : "Standard");
        } else if (hasVip) {
            cout << "Seat type: VIP (Only option available for this route)\n";
            seatType = "VIP";
        } else if (hasStandard) {
            cout << "Seat type: Standard (Only option available for this route)\n";
            seatType = "Standard";
        } else {
            cout << "\nNo available seat types (VIP/Standard) found for this route.\n";
            system("pause");
            return;
        }

        cout << "Payment (1=Chuyen khoan, 2=Tien mat, 3=ZaloPay): ";
        cin >> payChoice; cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string payMethod = (payChoice==1?"Chuyen khoan":(payChoice==2?"Tien mat":"ZaloPay"));


        // 4 Shows available trips matching seat type
        cout << "\nAvailable trips (Matching your " << seatType << " choice):\n";
        cout << left << setw(8) << "TripID" << setw(8) << "Bus"
             << setw(10) << "Depart" << setw(10) << "Arrive" << "\n";
        
        int tripsDisplayed = 0;
        for (auto t : foundTrips) { 
            for (const auto& b : buses)
                if (b.getId() == t->getBusId() &&
                    toLowerStr(b.getType()) == toLowerStr(seatType)) { // match seat type
                    string seatFile = "Data/Seat/" + b.getId() + ".txt";
                    ifstream fs(seatFile); string line; bool hasEmpty=false;
                    while (getline(fs, line)) {
                        auto v = splitCSV(line);
                        if (v.size() >= 3 && v[2] == "0") { hasEmpty = true; break; }
                    }
                    fs.close();
                    if (hasEmpty) {
                        cout << left << setw(8) << t->getId()
                             << setw(8) << b.getId()
                             << setw(10) << t->getDepart()
                             << setw(10) << t->getArrival() << "\n";
                        tripsDisplayed++;
                    }
                }
        }
        
        if (tripsDisplayed == 0) {
            cout << "\nNo available trips found for the selected route with a '" << seatType << "' bus.\n";
            cout << "The bus might be full, or only offers the other seat type.\n";
            system("pause");
            return;
        }

        flushConsoleEvents();
        cout << "\nTrip ID to book: ";
        string tripId; getline(cin, tripId);

        const Trip* chosen = nullptr;
        for (const auto& t : trips)
            if (toLowerStr(t.getId()) == toLowerStr(tripId)) { chosen = &t; break; }
        if (!chosen) { cout << "Invalid trip.\n"; system("pause"); return; }

        string busId = chosen->getBusId();
        string seatPath = "Data/Seat/" + busId + ".txt";

        vector<int> emptySeats; ifstream fs(seatPath); string line;
        cout << "\nEmpty seats: ";
        while (getline(fs, line)) {
            auto v = splitCSV(line);
            if (v.size() >= 3 && v[2] == "0") {
                cout << v[1] << " ";
                try { emptySeats.push_back(stoi(v[1])); } catch(...) {}
            }
        }
        fs.close();

        cout << "\nSeat number: ";
        string seatNoStr; getline(cin, seatNoStr);
        int seatNo = -1; try { seatNo = stoi(seatNoStr); } catch (...) {}
        if (seatNo < 0 || find(emptySeats.begin(), emptySeats.end(), seatNo) == emptySeats.end()) {
            cout << "Invalid or already booked seat.\n"; system("pause"); return;
        }

        unsigned long price = 0;
        if (bookedRoute) {
            try {
                double d = stod(ensureSuffix(bookedRoute->getDistance(), ""));
                price = (d <= 50) ? (unsigned long)(d * 1000)
                                  : (unsigned long)(50*1000 + (d - 50) * 1500);
            } catch (...) { /* keep 0 */ }
        }
        string bookedAt = nowStr(); 

        // pick ticket file
        string tkFile;
        for (int i = 1; i <= 20; i++) {
            stringstream ss; ss << "Data/Ticket/TK" << setw(3) << setfill('0') << i << ".txt";
            ifstream test(ss.str()); string temp;
            if (!test.is_open()) continue;
            getline(test, temp);
            if (!temp.empty() &&
                temp.find(chosen->getId()) != string::npos &&
                temp.find(busId) != string::npos) { tkFile = ss.str(); break; }
        }
        if (tkFile.empty()) tkFile = "Data/Ticket/TK001.txt";

        // new ticket id
        vector<string> lines; ifstream fin(tkFile);
        while (getline(fin, line)) if (!line.empty()) lines.push_back(line);
        fin.close();
        string lastId = lines.empty()? "TK000" : splitCSV(lines.back())[0];
        int next = 0; try { next = stoi(lastId.substr(2)) + 1; } catch(...) {}
        stringstream nid; nid << "TK" << setw(3) << setfill('0') << next; string tid = nid.str();

        string newLine = tid + "," + chosen->getId() + "," + busId + "," + seatNoStr + ","
                         + name + "," + phone + "," + to_string(price) + "," + bookedAt + "," + payMethod;
        lines.push_back(newLine);
        ofstream fout(tkFile, ios::trunc); for (auto &l : lines) fout << l << "\n"; fout.close();

        // update seat file
        vector<string> updated; ifstream fs2(seatPath);
        while (getline(fs2, line)) {
            if (line.empty()) continue;
            auto v = splitCSV(line);
            if (v.size() >= 3 && v[1] == seatNoStr) v[2] = "1";
            updated.push_back(v[0]+","+v[1]+","+v[2]);
        }
        fs2.close();
        ofstream fout2(seatPath, ios::trunc); for (auto &s : updated) fout2 << s << "\n"; fout2.close();
        for (auto &s : seats) if (s.getBusId() == busId && s.getSeatNo() == seatNo) { s.setStatus(true); break; }
        tickets.push_back(Ticket::fromCSV(newLine));

        cout << "\nBooked successfully. Ticket ID: " << tid << "\n";
        
        cout << "\n===== TICKET CONFIRMATION =====\n";
        
        const Bus* bookedBus = nullptr;
        for (const auto& b : buses) {
            if (b.getId() == busId) {
                bookedBus = &b;
                break;
            }
        }

        cout << left << setw(15) << "Ticket ID:" << tid << "\n";
        cout << left << setw(15) << "Passenger:" << name << " (" << phone << ")\n";

        if (bookedRoute) {
            cout << left << setw(15) << "Route:" << bookedRoute->getName() << " (" << bookedRoute->getStart() << " -> " << bookedRoute->getEnd() << ")\n";
        }
        if (bookedBus) {
            cout << left << setw(15) << "Bus License:" << bookedBus->getName() << " (ID: " << busId << ")\n";
        }
        
        cout << left << setw(15) << "Seat Number:" << seatNoStr << " (Type: " << seatType << ")\n";
        cout << left << setw(15) << "Departure:" << chosen->getDepart() << "\n";
        cout << left << setw(15) << "Arrival:" << chosen->getArrival() << "\n";
        cout << left << setw(15) << "Price:" << price << " VND\n";
        cout << left << setw(15) << "Booked At:" << bookedAt << "\n";
        cout << left << setw(15) << "Payment:" << payMethod << "\n";
        cout << "=================================\n";

        cout << "\n"; system("pause");
    }
    else if (sel == 1) { // CANCEL
        ui::DrawPanel("CANCEL TICKET");
        flushConsoleEvents();
        cout << "Passenger name: "; string name; getline(cin, name);
        cout << "Phone number  : "; string phone; getline(cin, phone);

        bool found = false; string busId, seatNo; int seatNoInt = -1;
        string tkPath = ""; 

        for (int i = 1; i <= 20; i++) {
            stringstream ss; ss << "Data/Ticket/TK" << setw(3) << setfill('0') << i << ".txt";
            string path = ss.str(); ifstream fin(path); if (!fin.is_open()) continue;

            vector<string> lines; string l; while (getline(fin, l)) if (!l.empty()) lines.push_back(l); fin.close();
            vector<string> keep;

            for (auto &ln : lines) {
                auto v = splitCSV(ln);
                if (v.size() < 9) { keep.push_back(ln); continue; }
                if (toLowerStr(v[4]) == toLowerStr(name) && toLowerStr(v[5]) == toLowerStr(phone)) {
                    busId = v[2]; seatNo = v[3];
                    try { seatNoInt = stoi(v[3]); } catch(...) {}
                    found = true; 
                    tkPath = path;
                    continue;
                }
                keep.push_back(ln);
            }
            if (found) { 
                ofstream fout(path, ios::trunc);
                for (auto &l2 : keep) fout << l2 << "\n"; fout.close(); 
                
                for (auto it = tickets.begin(); it != tickets.end(); ++it) {
                    if (toLowerStr(it->getPassengerName()) == toLowerStr(name) &&
                        toLowerStr(it->getPhoneNumber()) == toLowerStr(phone) &&
                        it->getBusId() == busId &&
                        it->getSeatNo() == seatNoInt) 
                    {
                        tickets.erase(it);
                        break; 
                    }
                }
                break; 
            }
        }
        if (found && !busId.empty()) {
            string seatPath = "Data/Seat/" + busId + ".txt";
            vector<string> updated; ifstream fs(seatPath); string line;
            while (getline(fs, line)) {
                if (line.empty()) continue; auto v = splitCSV(line);
                if (v.size() >= 3 && v[1] == seatNo) v[2] = "0";
                updated.push_back(v[0]+","+v[1]+","+v[2]);
            }
            fs.close();
            ofstream fout2(seatPath, ios::trunc); for (auto &s : updated) fout2 << s << "\n"; fout2.close();
            for (auto &s : seats) if (s.getBusId() == busId && s.getSeatNo() == seatNoInt) { s.setStatus(false); break; }
            
            cout << "\nCanceled ticket for (" << name << ", " << phone << ").\n";
            cout << "Updated file: " << tkPath << "\n";
        } else {
            cout << "No matching ticket.\n";
        }
        cout << "\n"; system("pause");
    }
}

// ---------------- LOGIN & ROLE MENUS ----------------
static bool loginFlow(AuthManager& auth, vector<User>& users) {
    while (true) {
        int sel = ConsoleMenu::pick("WELCOME", { "Login", "Create Account", "Exit" });
        if (sel == -1 || sel == 2) return false;
        if (sel == 1) { createAccount(users); continue; }

        if (sel == 0) {
            ui::DrawPanel("LOGIN");
            flushConsoleEvents();
            cout << "Username: "; string u; getline(cin, u);
            cout << "Password: "; string p; getline(cin, p);

            auth.load(users);
            if (auth.login(u, p)) {
                cout << "\nLogin success. Hello " << u << ".\n";
                system("pause"); return true;
            } else {
                cout << "\nInvalid username/password.\n"; system("pause");
            }
        }
    }
}
static void adminMenu(AuthManager& auth,
               vector<Brand>& brands, vector<Bus>& buses, vector<Driver>& drivers,
               vector<Route>& routes, vector<Trip>& trips, vector<Seat>& seats, vector<Ticket>& tickets) {
    while (auth.isLoggedIn() && auth.isAdmin()) {
        int sel = ConsoleMenu::pick("MAIN MENU (ADMIN)", {
            "Manage Routes",
            "Manage Buses",
            "Manage Tickets",
            "Booking / Cancel",
            "Logout",
            "Exit"
        });
        if (sel == -1) continue;
        if (sel == 0)      menuRoute(routes);
        else if (sel == 1) menuBus(brands, buses, drivers, seats);
        else if (sel == 2) menuTicket(tickets, trips, routes, buses, brands);
        else if (sel == 3) menuBooking(routes, trips, buses, seats, tickets);
        else if (sel == 4) { auth.logout(); break; }
        else if (sel == 5) { auth.logout(); exit(0); }
    }
}
static void userMenu(AuthManager& auth,
              vector<Route>& routes, const vector<Trip>& trips,
              const vector<Bus>& buses, vector<Seat>& seats, vector<Ticket>& tickets) {
    while (auth.isLoggedIn() && !auth.isAdmin()) {
        int sel = ConsoleMenu::pick("MAIN MENU (USER)", { "Booking / Cancel", "Logout", "Exit" });
        if (sel == -1) continue;
        if (sel == 0)      menuBooking(routes, trips, buses, seats, tickets);
        else if (sel == 1) { auth.logout(); break; }
        else if (sel == 2) { auth.logout(); exit(0); }
    }
}

// ---------------- MAIN ----------------
int main() {
    // Load core data
    vector<Brand>  brands = loadData("Data/Brand.txt",  Brand::fromCSV);
    vector<Bus>    buses  = loadData("Data/Bus.txt",    Bus::fromCSV);
    vector<Driver> drivers= loadData("Data/Driver.txt", Driver::fromCSV);
    vector<Route>  routes = loadData("Data/Route.txt",  Route::fromCSV);
    vector<Trip>   trips  = loadData("Data/Trip.txt",   Trip::fromCSV);

    // Seats
    vector<Seat> seats;
    for (int i = 1; i <= 20; i++) {
        stringstream p; p << "Data/Seat/B" << setw(3) << setfill('0') << i << ".txt";
        ifstream fin(p.str()); if (fin.is_open()) {
            vector<Seat> tmp = loadData(p.str(), Seat::fromCSV);
            seats.insert(seats.end(), tmp.begin(), tmp.end());
            fin.close();
        }
    }
    // Tickets
    vector<Ticket> tickets;
    for (int i = 1; i <= 20; i++) {
        stringstream p; p << "Data/Ticket/TK" << setw(3) << setfill('0') << i << ".txt";
        ifstream fin(p.str()); if (fin.is_open()) {
            vector<Ticket> tmp = loadData(p.str(), Ticket::fromCSV);
            tickets.insert(tickets.end(), tmp.begin(), tmp.end());
            fin.close();
        }
    }
    // Users
    vector<User> users = loadData("Data/User.txt", User::fromCSV);
    if (users.empty()) {
        users.push_back(User("U001", "admin", "admin123", Role::ADMIN));
        users.push_back(User("U002", "user1", "123456",   Role::USER));
        ofstream fout("Data/User.txt"); for (auto &u : users) fout << u.toCSV() << "\n"; fout.close();
    }

    // Auth + menus
    for (;;) {
        AuthManager auth;
        if (!loginFlow(auth, users)) break;
        if (auth.isAdmin()) adminMenu(auth, brands, buses, drivers, routes, trips, seats, tickets);
        else                userMenu(auth, routes, trips, buses, seats, tickets);
    }
    cout << "Goodbye!\n";
    return 0;
}
