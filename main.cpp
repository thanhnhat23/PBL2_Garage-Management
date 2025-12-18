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
#include <filesystem>
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

namespace fs = std::filesystem;
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

    string uname, pwd, confirm, phone;
    while (true) {  
        cout << "Username (>= 3 characters, without accents): ";
        getline(cin, uname);
        if (uname.size() < 3) { cout << "Pass through.\n"; continue; }
        if (usernameExists(users, uname)) { cout << "The username already exists. Try a different name.\n"; continue; }
        break;
    }
    cout << "Password (>= 4 ky tu): "; getline(cin, pwd);
    cout << "Confirm  : "; getline(cin, confirm);
    if (pwd.size() < 4 || pwd != confirm) {
        cout << "Invalid/unusual password.\n";
        system("pause"); return;
    }
    cout << "Phone Number: "; getline(cin, phone);
    if (phone.empty()) {
        cout << "Phone numbers should not be left blank!\n";
        system("pause"); return;
    }

    string uid = nextUserId(users);
    
    users.push_back(User(uid, uname, pwd, Role::USER, phone));

    ofstream fout("Data/User.txt");
    for (auto &u : users) fout << u.toCSV() << "\n";
    fout.close();

    cout << "\nAccount created successfully!\nID: " << uid << "\nPhone: " << phone << "\n\n";
    system("pause");
}

int timeToMinutes(string timeStr) {
    if (timeStr.size() != 5 || timeStr[2] != ':') return -1;
    try {
        int h = stoi(timeStr.substr(0, 2));
        int m = stoi(timeStr.substr(3, 2));
        if (h < 0 || h > 23 || m < 0 || m > 59) return -1;
        return h * 60 + m;
    } catch(...) { return -1; }
}

string minutesToTime(int totalMinutes) {
    int h = (totalMinutes / 60) % 24; 
    int m = totalMinutes % 60;
    stringstream ss;
    ss << setfill('0') << setw(2) << h << ":" << setfill('0') << setw(2) << m;
    return ss.str();
}

// Logic to check if a bus is busy during a new trip's time slot
bool isBusBusy(const string& busId, int newStart, int newEnd, 
               const string& newRouteId, 
               const vector<Trip>& trips, 
               const vector<Route>& routes, 
               const string& currentTripId = "") {
    
    // 1 Find new route details
    const Route* newR = nullptr;
    for(const auto& r : routes) if(toLowerStr(r.getId()) == toLowerStr(newRouteId)) { newR = &r; break; }
    if (!newR) return false; 

    for (const auto& t : trips) {
        if (toLowerStr(t.getId()) == toLowerStr(currentTripId)) continue; 

        if (toLowerStr(t.getBusId()) == toLowerStr(busId)) {
            int tStart = timeToMinutes(t.getDepart());
            int tEnd = timeToMinutes(t.getArrival());
            if (tStart == -1 || tEnd == -1) continue;

            // 2 Find old route of the trip
            const Route* oldR = nullptr;
            for(const auto& r : routes) if(toLowerStr(r.getId()) == toLowerStr(t.getRouteId())) { oldR = &r; break; }
            
            if (!oldR) continue;

            // Time buffer for bus to be busy after trip ends
            int busyStart = tStart;
            int busyEnd = tEnd + 30; 

            if (toLowerStr(newR->getStart()) != toLowerStr(oldR->getEnd())) {

                int tripDuration = tEnd - tStart;
                busyEnd = tEnd + tripDuration + 30; 
            }
            // 3 Check for time overlap
            if (max(newStart, busyStart) < min(newEnd, busyEnd)) {
                string timeStr = minutesToTime(busyEnd);
                cout << "\n[!] CONFLICT DETECTED for Bus " << busId << ":\n";
                cout << "    - Occupied by Trip: " << t.getId() << " (" << t.getDepart() << " - " << t.getArrival() << ")\n";
                cout << "    - Route: " << oldR->getName() << " (" << oldR->getStart() << " -> " << oldR->getEnd() << ")\n";
                
                if (busyEnd > tEnd + 30) {
                     cout << "    - Reason: Bus needs time to return to '" << newR->getStart() << "'.\n";
                     cout << "    - Bus available at: " << timeStr << " (including return trip + 30m break).\n";
                } else {
                     cout << "    - Bus available at: " << timeStr << " (after arrival + 30m break).\n";
                }
                return true; 
            }
        }
    }
    return false;
}

// Helper to parse route name (e.g., "Da Nang - Hue" -> start="Da Nang", end="Hue")
static pair<string, string> parseRouteName(const string& fullName) {
    size_t sep = fullName.find(" - ");
    if (sep == string::npos) return {"", ""};
    string start = fullName.substr(0, sep);
    string end = fullName.substr(sep + 3);
    return {start, end};
}

// Quick pick helpers to replace manual ID typing
static int pickBrand(const vector<Brand>& brands, const string& title) {
    vector<string> opts; opts.reserve(brands.size());
    for (const auto& b : brands) opts.push_back("[" + b.getId() + "] " + b.getName());
    return ConsoleMenu::pick(title, opts);
}
static int pickRoute(const vector<Route>& routes, const string& title) {
    vector<string> opts; opts.reserve(routes.size());
    for (const auto& r : routes) opts.push_back(r.getId() + " - " + r.getName());
    return ConsoleMenu::pick(title, opts);
}
static int pickBus(const vector<Bus>& buses, const string& title) {
    vector<string> opts; opts.reserve(buses.size());
    for (const auto& b : buses) opts.push_back(b.getId() + " - " + b.getName() + " (" + b.getType() + ")");
    return ConsoleMenu::pick(title, opts);
}
static int pickTrip(const vector<Trip>& trips, const vector<Driver>& drivers, const string& title) {
    vector<string> opts; opts.reserve(trips.size());
    for (const auto& t : trips) {
        string drv = "Unknown";
        for (const auto& d : drivers) if (d.getId() == t.getDriverId()) { drv = d.getName(); break; }
        opts.push_back(t.getId() + " | Route " + t.getRouteId() + " | Bus " + t.getBusId() + " | " + drv + " | " + t.getDepart());
    }
    return ConsoleMenu::pick(title, opts);
}

// Date helpers for ticket statistics (bookedAt: YYYY-MM-DD HH:MM)
static bool parseBookedYMD(const string& bookedAt, int& y, int& m, int& d) {
    if (bookedAt.size() < 10 || bookedAt[4] != '-' || bookedAt[7] != '-') return false;
    try {
        y = stoi(bookedAt.substr(0, 4));
        m = stoi(bookedAt.substr(5, 2));
        d = stoi(bookedAt.substr(8, 2));
        if (y < 1900 || m < 1 || m > 12 || d < 1 || d > 31) return false;
        return true;
    } catch (...) { return false; }
}
static vector<int> getUniqueYears(const vector<Ticket>& tickets) {
    vector<int> years;
    for (const auto& tk : tickets) {
        int y=0,m=0,d=0;
        if (parseBookedYMD(tk.getBookedAt(), y, m, d)) {
            if (find(years.begin(), years.end(), y) == years.end()) years.push_back(y);
        }
    }
    sort(years.begin(), years.end());
    return years;
}
static vector<int> getUniqueMonths(const vector<Ticket>& tickets, int year) {
    vector<int> months;
    for (const auto& tk : tickets) {
        int y=0,m=0,d=0;
        if (parseBookedYMD(tk.getBookedAt(), y, m, d) && y == year) {
            if (find(months.begin(), months.end(), m) == months.end()) months.push_back(m);
        }
    }
    sort(months.begin(), months.end());
    return months;
}
static vector<int> getUniqueDays(const vector<Ticket>& tickets, int year, int month) {
    vector<int> days;
    for (const auto& tk : tickets) {
        int y=0,m=0,d=0;
        if (parseBookedYMD(tk.getBookedAt(), y, m, d) && y == year && m == month) {
            if (find(days.begin(), days.end(), d) == days.end()) days.push_back(d);
        }
    }
    sort(days.begin(), days.end());
    return days;
}
static vector<Ticket> filterTicketsByDate(const vector<Ticket>& tickets, int year, int month, int day) {
    vector<Ticket> out;
    for (const auto& tk : tickets) {
        int y=0,m=0,d=0;
        if (!parseBookedYMD(tk.getBookedAt(), y, m, d)) continue;
        if (year > 0 && y != year) continue;
        if (month > 0 && m != month) continue;
        if (day > 0 && d != day) continue;
        out.push_back(tk);
    }
    return out;
}
static void printTicketSummary(const vector<Ticket>& tickets) {
    unsigned long long revenue = 0;
    for (const auto& tk : tickets) revenue += tk.getPrice();
    cout << "\n[ SUMMARY ]\n";
    cout << "Total tickets: " << tickets.size() << "\n";
    cout << "Total revenue: " << revenue << " VND\n";
}

// Shared renderer for bus details (route, schedule, seats, drivers)
static void renderBusDetails(const Bus& bus,
                             const vector<Driver>& drivers,
                             const vector<Seat>& seats,
                             const vector<Route>& routes,
                             const vector<Trip>& trips) {
    vector<Bus> temp = { bus };
    cout << "\n[ BUS INFO ]\n";
    ui::PrintBuses(temp);

    int bookedSeats = 0;
    for (const auto& s : seats)
        if (toLowerStr(s.getBusId()) == toLowerStr(bus.getId()) && s.getStatus())
            bookedSeats++;
    int totalSeats = bus.getCapacity();
    int freeSeats = max(0, totalSeats - bookedSeats);
    cout << "\n[ SEAT SUMMARY ]\n";
    cout << "Total: " << totalSeats
         << " | Booked: " << bookedSeats
         << " | Available: " << freeSeats << "\n";

    vector<Driver> ds;
    for (auto &d : drivers)
        if (toLowerStr(d.getBus()) == toLowerStr(bus.getId())) ds.push_back(d);

    cout << "\n[ DRIVER ]\n";
    if (ds.empty()) cout << "(No driver assigned)\n";
    else ui::PrintDrivers(ds);

    vector<Seat> ss;
    for (auto &s : seats)
        if (toLowerStr(s.getBusId()) == toLowerStr(bus.getId())) ss.push_back(s);

    cout << "\n[ SEATS ]\n";
    if (ss.empty()) cout << "(No seat data found)\n";
    else ui::PrintSeats(ss);

    cout << "\n[ TRIPS / ROUTES ]\n";
    bool anyTrip = false;
    cout << left << setw(8) << "TripID" << setw(15) << "Route"
         << setw(22) << "From -> To" << setw(10) << "Depart" << setw(10) << "Arrive" << setw(20) << "Driver" << "\n";
    cout << string(85, '-') << "\n";
    for (const auto& t : trips) {
        if (toLowerStr(t.getBusId()) != toLowerStr(bus.getId())) continue;
        const Route* r = nullptr;
        for (const auto& rt : routes) if (toLowerStr(rt.getId()) == toLowerStr(t.getRouteId())) { r = &rt; break; }
        string drvName = "Unknown";
        for (const auto& d : drivers) if (d.getId() == t.getDriverId()) { drvName = d.getName(); break; }
        string routeLabel = (r ? (r->getId()) : t.getRouteId());
        string fromTo = (r ? (r->getStart() + " -> " + r->getEnd()) : "?");
        cout << left << setw(8) << t.getId() << setw(15) << routeLabel
             << setw(22) << fromTo << setw(10) << t.getDepart()
             << setw(10) << t.getArrival() << setw(20) << drvName << "\n";
        anyTrip = true;
    }
    if (!anyTrip) cout << "(No trips scheduled for this bus)\n";
}

// ---------------- BRAND MENU ----------------
static void menuBrand(vector<Brand>& brands) {
    while (true) {
        int sel = ConsoleMenu::pick("BRAND MANAGEMENT", {
            "View All Brands",
            "Add New Brand",
            "Update Brand",
            "Delete Brand",
            "Back"
        });
        if (sel == -1 || sel == 4) break;

        if (sel == 0) { // View
            ui::DrawPanel("ALL BRANDS");
            if (brands.empty()) {
                cout << "No brands available.\n";
            } else {
                cout << left << setw(10) << "ID" 
                     << setw(25) << "Name" 
                     << setw(15) << "Hotline" 
                     << setw(10) << "Rating" << "\n";
                cout << string(60, '-') << "\n";
                for (const auto& b : brands) {
                    cout << left << setw(10) << b.getId() 
                         << setw(25) << b.getName() 
                         << setw(15) << b.getHotline() 
                         << setw(10) << fixed << setprecision(1) << b.getRating() << "\n";
                }
            }
            cout << "\n"; system("pause");
        }
        else if (sel == 1) { // Add
            ui::DrawPanel("ADD NEW BRAND");
            flushConsoleEvents();
            string name, phone, rateStr;
            float rate = 0.0;

            cout << "Brand Name: "; getline(cin, name);
            if (name.empty()) continue;
            
            cout << "Hotline: "; getline(cin, phone);
            
            cout << "Rating (0-5): "; getline(cin, rateStr);
            try { rate = stof(rateStr); } catch(...) { rate = 0.0; }

            string lastId = brands.empty() ? "BR000" : brands.back().getId();
            int next = 0; 
            try { next = stoi(lastId.substr(2)) + 1; } catch(...) {}
            stringstream ss; ss << "BR" << setw(3) << setfill('0') << next;

            Brand b(ss.str(), name, phone, rate);
            brands.push_back(b);
            Ultil<Brand>::saveToFile("Data/Brand.txt", brands);
            
            cout << "\nAdded brand " << b.getId() << " successfully.\n";
            cout << "\n"; system("pause");
        }
        else if (sel == 2) { // Update
            ui::DrawPanel("UPDATE BRAND");
            flushConsoleEvents();
            if (brands.empty()) { cout << "No brands available.\n"; system("pause"); continue; }
            
            int idx = pickBrand(brands, "Select Brand to Update");
            if (idx < 0 || idx >= (int)brands.size()) continue;

            auto& b = brands[idx];
            cout << "\nEditing: " << b.getName() << "\n";
            
            string name, phone, rateStr;
            
            cout << "New Name [" << b.getName() << "]: "; getline(cin, name);
            if (!name.empty()) b.setName(name);
            
            cout << "New Hotline [" << b.getHotline() << "]: "; getline(cin, phone);
            if (!phone.empty()) b.setHotline(phone);

            cout << "New Rating [" << b.getRating() << "]: "; getline(cin, rateStr);
            if (!rateStr.empty()) {
                try { float r = stof(rateStr); b.setRating(r); } catch(...) {}
            }

            Ultil<Brand>::saveToFile("Data/Brand.txt", brands);
            cout << "\nUpdated successfully.\n";
            cout << "\n"; system("pause");
        }
        else if (sel == 3) { // Delete
            ui::DrawPanel("DELETE BRAND");
            flushConsoleEvents();
            if (brands.empty()) { cout << "No brands available.\n"; system("pause"); continue; }
            
            int idx = pickBrand(brands, "Select Brand to Delete");
            if (idx < 0 || idx >= (int)brands.size()) continue;

            Brand removed = brands[idx];
            brands.erase(brands.begin() + idx);
            Ultil<Brand>::saveToFile("Data/Brand.txt", brands);
            cout << "\nDeleted: " << removed.getId() << " - " << removed.getName() << "\n";
            cout << "\n"; system("pause");
        }
    }
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
            string key; getline(cin, key); 
            if (key.empty()) continue;
            key = toLowerStr(key);

            cout << "\n";
            vector<Route> found;
            for (const auto &r : routes) {
                string name = toLowerStr(r.getName() + " " + r.getStart() + " " + r.getEnd());
                if (name.find(key) != string::npos) found.push_back(r);
            }
            if (found.empty()) cout << "No matching routes found.\n";
            else ui::PrintRoutes(found);
            cout << "\n"; system("pause");
        }
        else if (sel == 2) { // Add
            ui::DrawPanel("ADD NEW ROUTE");
            flushConsoleEvents();
            string name, dist, dur;

            cout << "Enter Route Name (format: 'Da Nang - Hue'): ";
            getline(cin, name);
            if (name.empty()) continue;
            
            auto [start, end] = parseRouteName(name);
            if (start.empty() || end.empty()) {
                cout << "Invalid format. Must be 'Start - End' (with ' - ' separator).\n";
                system("pause");
                continue;
            }

            cout << "Enter Distance (in km, e.g., 100): ";
            getline(cin, dist);
            if (dist.empty()) continue;

            cout << "Enter Duration (in minutes, e.g., 180): ";
            getline(cin, dur);
            if (dur.empty()) continue;

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

            if (routes.empty()) { cout << "No routes available.\n"; system("pause"); continue; }
            int idx = pickRoute(routes, "Select Route to Update");
            if (idx < 0 || idx >= (int)routes.size()) continue;

            auto &r = routes[idx];
            cout << "\nEditing: " << r.getName() << "\n";
            string name;
            cout << "New Name (format: 'Start - End') [" << r.getName() << "]: "; 
            getline(cin, name);
            
            if (!name.empty()) {
                auto [newStart, newEnd] = parseRouteName(name);
                if (newStart.empty() || newEnd.empty()) {
                    cout << "Invalid format. Skipping name update.\n";
                } else {
                    r.setName(name);
                    r.setStart(newStart);
                    r.setEnd(newEnd);
                }
            }

            string dist, dur;
            cout << "New Distance [" << r.getDistance() << "]: "; getline(cin, dist);
            if (!dist.empty()) r.setDistance(dist);
            
            cout << "New Duration [" << r.getDuration() << "]: "; getline(cin, dur);
            if (!dur.empty()) r.setDuration(dur);

            Ultil<Route>::saveToFile("Data/Route.txt", routes);
            cout << "\nUpdated.\n\n";
            ui::PrintRoutes({r});
            cout << "\n"; system("pause");
        }
        else if (sel == 4) { // Delete
            ui::DrawPanel("DELETE ROUTE");
            flushConsoleEvents();

            if (routes.empty()) { cout << "No routes available.\n"; system("pause"); continue; }
            int idx = pickRoute(routes, "Select Route to Delete");
            if (idx < 0 || idx >= (int)routes.size()) continue;

            Route removed = routes[idx];
            routes.erase(routes.begin() + idx);
            Ultil<Route>::saveToFile("Data/Route.txt", routes);
            cout << "\nDeleted.\n\nRemoved:\n";
            ui::PrintRoutes({removed});
            cout << "\n"; system("pause");
        }
    }
}

// ---------------- BUS MENU ----------------
static void menuBus(const vector<Brand> &brands, vector<Bus> &buses,
                    vector<Driver> &drivers, vector<Seat> &seats,
                    const vector<Route>& routes, const vector<Trip>& trips) {
    while (true) {
        int sel = ConsoleMenu::pick("BUS MANAGEMENT", {
            "View Buses by Brand",
            "Search Bus",
            "Add New Bus",
            "Update Bus",
            "Delete Bus",
            "View Driver",
            "Back"
        });

        if (sel == -1 || sel == 6) break;

        // 1 View Buses by Brand (menu select)
        if (sel == 0) {
            ui::DrawPanel("VIEW BUSES BY BRAND");
            if (brands.empty()) { cout << "No brands available.\n"; system("pause"); continue; }

            int bIdx = pickBrand(brands, "Select Brand");
            if (bIdx < 0 || bIdx >= (int)brands.size()) continue;
            string targetBrandId = brands[bIdx].getId();

            vector<Bus> brandBuses;
            for (const auto &bus : buses) 
                if (bus.getBrandId() == targetBrandId) brandBuses.push_back(bus);
            
            cout << "\n";
            if (brandBuses.empty()) cout << "(No buses found for this brand)\n";
            else ui::PrintBuses(brandBuses);
            
            cout << "\n"; system("pause");
        }

        // 2 Search Bus Details
        else if (sel == 1) {
            ui::DrawPanel("SEARCH BUS DETAILS");
            if (buses.empty()) { cout << "No buses available.\n"; system("pause"); continue; }
            int idx = pickBus(buses, "Select Bus");
            if (idx < 0 || idx >= (int)buses.size()) continue;
            renderBusDetails(buses[idx], drivers, seats, routes, trips);
            
            cout << "\n"; system("pause");
        }

        // 3 Add New Bus
        else if (sel == 2) {
            ui::DrawPanel("ADD NEW BUS");
            flushConsoleEvents();
            string brandId, plate, type, drvName, drvPhone;
            int cap = 0, exp = 0;

            cout << "Available Brands:\n";
            for (const auto &b : brands) cout << " " << b.getId() << ": " << b.getName() << "\n";
            
            string inputId;
            cout << "Enter Brand ID: "; 
            getline(cin, inputId);
            if (inputId.empty()) continue;
            
            bool found = false; 
            for(const auto& b : brands) {
                if(toLowerStr(b.getId()) == toLowerStr(inputId)) {
                    brandId = b.getId(); 
                    found = true;
                    break;
                }
            }
            if(!found) continue;

            cout << "License Plate (e.g. 43A-99999): "; 
            getline(cin, plate);
            if (plate.empty()) continue;
            
            cout << "Type (VIP/Standard): "; 
            getline(cin, type);
            if (type.empty()) continue;
            if(toLowerStr(type)!="vip" && toLowerStr(type)!="standard") continue;
            type = (toLowerStr(type) == "vip" ? "VIP" : "Standard");

            cout << "Capacity (e.g. 30): "; 
            string s; getline(cin, s);
            if (s.empty()) continue;
            try { cap = stoi(s); if(cap <= 0) continue; } catch(...) { continue; }
            
            cout << "\n-- Driver Info --\n";
            cout << "Driver Name : "; getline(cin, drvName);
            if (drvName.empty()) continue;
            
            cout << "Driver Phone: "; getline(cin, drvPhone);
            if (drvPhone.empty()) continue;
            
            cout << "Experience (years): "; 
            getline(cin, s);
            if (s.empty()) continue;
            try { exp = stoi(s); } catch(...) { continue; }

            // Generate IDs
            string lastBusId = buses.empty() ? "B000" : buses.back().getId();
            int nextB = 0; try { nextB = stoi(lastBusId.substr(1)) + 1; } catch(...) {}
            stringstream sb; sb << "B" << setw(3) << setfill('0') << nextB;
            string bid = sb.str();

            string lastDrvId = drivers.empty() ? "D000" : drivers.back().getId();
            int nextD = 0; try { nextD = stoi(lastDrvId.substr(1)) + 1; } catch(...) {}
            stringstream sd; sd << "D" << setw(3) << setfill('0') << nextD;
            string did = sd.str();

            // Add to Memory
            buses.push_back(Bus(bid, brandId, plate, type, cap));
            drivers.push_back(Driver(did, bid, drvName, drvPhone, exp));

            // Create Seat File
            string sPath = "Data/Seat/" + bid + ".txt";
            ofstream f(sPath);
            if (f.is_open()) {
                for(int i=1; i<=cap; ++i) {
                    f << bid << "," << i << ",0\n";
                    seats.push_back(Seat(bid, i, false));
                }
                f.close();
            }

            // Save to Disk
            Ultil<Bus>::saveToFile("Data/Bus.txt", buses);
            Ultil<Driver>::saveToFile("Data/Driver.txt", drivers);

            cout << "\nSuccess! Created Bus " << bid << " (" << plate << ") and Driver " << did << ".\n";
            system("pause");
        }

        // 4. Update Bus
        else if (sel == 3) {
            ui::DrawPanel("UPDATE BUS");
            flushConsoleEvents();
            
            if (buses.empty()) { cout << "No buses available.\n"; system("pause"); continue; }
            int idx = pickBus(buses, "Select Bus to Update");
            if (idx < 0 || idx >= (int)buses.size()) continue;

            auto it = buses.begin() + idx;

            cout << "\nUpdating Bus " << it->getId() << " (" << it->getName() << ")...\n";
            
            string plate, type;
            cout << "New License Plate [" << it->getName() << "]: "; 
            getline(cin, plate);
            if (!plate.empty()) {
                *it = Bus(it->getId(), it->getBrandId(), plate, it->getType(), it->getCapacity());
            }

            cout << "New Type [" << it->getType() << "]: "; 
            getline(cin, type);
            if (!type.empty()) {
                if(toLowerStr(type)=="vip" || toLowerStr(type)=="standard") {
                    type = (toLowerStr(type) == "vip" ? "VIP" : "Standard");
                    *it = Bus(it->getId(), it->getBrandId(), it->getName(), type, it->getCapacity());
                } else {
                    cout << "Invalid type. Skipping.\n";
                }
            }

            Ultil<Bus>::saveToFile("Data/Bus.txt", buses);
            cout << "\nUpdated.\n";
            system("pause");
        }

        // 5. Delete Bus
        else if (sel == 4) {
            ui::DrawPanel("DELETE BUS");
            flushConsoleEvents();
            
            if (buses.empty()) { cout << "No buses available.\n"; system("pause"); continue; }
            int idx = pickBus(buses, "Select Bus to Delete");
            if (idx < 0 || idx >= (int)buses.size()) continue;

            string targetId = buses[idx].getId();
            cout << "Deleting Bus " << targetId << " (" << buses[idx].getName() << ")...\n";
            
            // Remove Bus
            buses.erase(buses.begin() + idx);
            
            // Remove Drivers
            auto itD = remove_if(drivers.begin(), drivers.end(), 
                [&](const Driver& d){ return toLowerStr(d.getBus()) == toLowerStr(targetId); });
            drivers.erase(itD, drivers.end());

            // Remove Seats (Memory)
            auto itS = remove_if(seats.begin(), seats.end(), 
                [&](const Seat& s){ return toLowerStr(s.getBusId()) == toLowerStr(targetId); });
            seats.erase(itS, seats.end());

            // Remove Seat File (Disk)
            string sPath = "Data/Seat/" + targetId + ".txt";
            if (fs::exists(sPath)) fs::remove(sPath);

            // Save Changes
            Ultil<Bus>::saveToFile("Data/Bus.txt", buses);
            Ultil<Driver>::saveToFile("Data/Driver.txt", drivers);

            cout << "Deleted successfully.\n";
            system("pause");
        }

        // 6. View Driver
        else if (sel == 5) {
            ui::DrawPanel("VIEW DRIVER");
            flushConsoleEvents();
            
            if (buses.empty()) { cout << "No buses available.\n"; system("pause"); continue; }
            int idx = pickBus(buses, "Select Bus to View Drivers");
            if (idx < 0 || idx >= (int)buses.size()) continue;

            const auto& b = buses[idx];
            vector<Driver> ds;
            for (const auto& d : drivers) {
                if (toLowerStr(d.getBus()) == toLowerStr(b.getId())) ds.push_back(d);
            }
            
            cout << "\nDrivers for Bus " << b.getId() << " (" << b.getName() << "):\n";
            if (ds.empty()) {
                cout << "(No driver assigned)\n";
            } else {
                ui::PrintDrivers(ds);
            }
            
            cout << "\n"; system("pause");
        }
    }
}


// ---------------- DRIVER MENU ----------------
static void menuDriver(vector<Driver>& drivers, const vector<Bus>& buses, const vector<Trip>& trips) {
    while (true) {
        int sel = ConsoleMenu::pick("DRIVER MANAGEMENT", {
            "View All Drivers",
            "Search Driver",
            "Add New Driver",
            "Update Driver",
            "Delete Driver",
            "View Driver Schedule",
            "Back"
        });
        if (sel == -1 || sel == 6) break;

        if (sel == 0) { // View All
            ui::DrawPanel("ALL DRIVERS");
            if (drivers.empty()) {
                cout << "No drivers available.\n";
            } else {
                ui::PrintDrivers(drivers);
            }
            cout << "\n"; system("pause");
        }
        else if (sel == 1) { // Search
            ui::DrawPanel("SEARCH DRIVER");
            flushConsoleEvents();
            cout << "Keyword (name or phone): ";
            string key; getline(cin, key);
            if (key.empty()) continue;
            key = toLowerStr(key);

            vector<Driver> found;
            for (const auto& d : drivers) {
                string searchStr = toLowerStr(d.getName() + " " + d.getPhone());
                if (searchStr.find(key) != string::npos) found.push_back(d);
            }
            
            cout << "\n";
            if (found.empty()) cout << "No matching drivers found.\n";
            else ui::PrintDrivers(found);
            cout << "\n"; system("pause");
        }
        else if (sel == 2) { // Add
            ui::DrawPanel("ADD NEW DRIVER");
            flushConsoleEvents();
            
            if (buses.empty()) { cout << "No buses available.\n"; system("pause"); continue; }
            int bidx = pickBus(buses, "Select Bus for Driver");
            if (bidx < 0 || bidx >= (int)buses.size()) continue;
            string busId = buses[bidx].getId();

            string name, phone;
            int exp;
            cout << "Driver Name: "; getline(cin, name);
            if (name.empty()) continue;
            cout << "Phone: "; getline(cin, phone);
            if (phone.empty()) continue;
            cout << "Experience (years): ";
            string expStr; getline(cin, expStr);
            try { exp = stoi(expStr); } catch(...) { continue; }

            string lastId = drivers.empty() ? "D000" : drivers.back().getId();
            int next = 0;
            try { next = stoi(lastId.substr(1)) + 1; } catch(...) {}
            stringstream ss; ss << "D" << setw(3) << setfill('0') << next;

            Driver d(ss.str(), busId, name, phone, exp);
            drivers.push_back(d);
            Ultil<Driver>::saveToFile("Data/Driver.txt", drivers);
            cout << "\nAdded driver " << d.getId() << " successfully.\n";
            ui::PrintDrivers({d});
            cout << "\n"; system("pause");
        }
        else if (sel == 3) { // Update
            ui::DrawPanel("UPDATE DRIVER");
            flushConsoleEvents();
            if (drivers.empty()) { cout << "No drivers available.\n"; system("pause"); continue; }

            cout << "All Drivers:\n";
            ui::PrintDrivers(drivers);
            
            cout << "\nDriver ID to update: ";
            string did; getline(cin, did);
            
            auto it = find_if(drivers.begin(), drivers.end(), 
                [&](const Driver& d) { return toLowerStr(d.getId()) == toLowerStr(did); });
            if (it == drivers.end()) { cout << "Invalid driver ID.\n"; system("pause"); continue; }

            cout << "\nUpdating: " << it->getName() << "\n";
            string name, phone, expStr;
            cout << "New Name [" << it->getName() << "]: "; getline(cin, name);
            if (!name.empty()) it->setName(name);
            
            cout << "New Phone [" << it->getPhone() << "]: "; getline(cin, phone);
            if (!phone.empty()) it->setPhone(phone);
            
            cout << "New Experience [" << it->getExp() << "]: "; getline(cin, expStr);
            if (!expStr.empty()) {
                try { int exp = stoi(expStr); it->setExp(exp); } catch(...) {}
            }

            Ultil<Driver>::saveToFile("Data/Driver.txt", drivers);
            cout << "\nUpdated successfully.\n";
            ui::PrintDrivers({*it});
            cout << "\n"; system("pause");
        }
        else if (sel == 4) { // Delete
            ui::DrawPanel("DELETE DRIVER");
            flushConsoleEvents();
            if (drivers.empty()) { cout << "No drivers available.\n"; system("pause"); continue; }

            cout << "All Drivers:\n";
            ui::PrintDrivers(drivers);
            
            cout << "\nDriver ID to delete: ";
            string did; getline(cin, did);
            
            auto it = find_if(drivers.begin(), drivers.end(),
                [&](const Driver& d) { return toLowerStr(d.getId()) == toLowerStr(did); });
            if (it == drivers.end()) { cout << "Invalid driver ID.\n"; system("pause"); continue; }

            Driver removed = *it;
            drivers.erase(it);
            Ultil<Driver>::saveToFile("Data/Driver.txt", drivers);
            cout << "\nDeleted: " << removed.getId() << " - " << removed.getName() << "\n";
            cout << "\n"; system("pause");
        }
        else if (sel == 5) { // View Schedule
            ui::DrawPanel("DRIVER SCHEDULE");
            flushConsoleEvents();
            if (drivers.empty()) { cout << "No drivers available.\n"; system("pause"); continue; }

            cout << "All Drivers:\n";
            ui::PrintDrivers(drivers);
            
            cout << "\nDriver ID: ";
            string did; getline(cin, did);
            
            auto it = find_if(drivers.begin(), drivers.end(),
                [&](const Driver& d) { return toLowerStr(d.getId()) == toLowerStr(did); });
            if (it == drivers.end()) { cout << "Invalid driver ID.\n"; system("pause"); continue; }

            cout << "\n[ DRIVER INFO ]\n";
            ui::PrintDrivers({*it});

            cout << "\n[ ASSIGNED TRIPS ]\n";
            bool found = false;
            cout << left << setw(8) << "TripID" << setw(8) << "Route" << setw(8) << "Bus"
                 << setw(10) << "Depart" << setw(10) << "Arrive" << "\n";
            cout << string(50, '-') << "\n";
            for (const auto& t : trips) {
                if (toLowerStr(t.getDriverId()) == toLowerStr(did)) {
                    cout << left << setw(8) << t.getId()
                         << setw(8) << t.getRouteId()
                         << setw(8) << t.getBusId()
                         << setw(10) << t.getDepart()
                         << setw(10) << t.getArrival() << "\n";
                    found = true;
                }
            }
            if (!found) cout << "(No trips assigned)\n";
            cout << "\n"; system("pause");
        }
    }
}

// ---------------- TICKET MENU ----------------
static void menuTicket(const vector<Ticket> &tickets, const vector<Trip> &trips,
                       const vector<Route> &routes, const vector<Bus> &buses,
                       const vector<Brand> &brands) {
    while (true) {
        // Main Ticket Menu using ConsoleMenu
        int sel = ConsoleMenu::pick("TICKET MANAGEMENT", {
            "View All Tickets",
            "Search Ticket by ID",
            "Search Ticket by Passenger",
            "Filter by Route",
            "Filter by Date Range",
            "View Tickets by Brand",
            "Back"
        });

        if (sel == -1 || sel == 6) break; // Back or Error

        if (sel == 0) { // View All
            ui::DrawPanel("ALL TICKETS");
            if (tickets.empty()) {
                cout << "No tickets available.\n";
            } else {
                ui::PrintTickets(tickets, trips, routes, buses);
            }
            cout << "\n"; system("pause");
        }
        else if (sel == 1) { // Search by ID
            ui::DrawPanel("SEARCH TICKET BY ID");
            flushConsoleEvents();
            cout << "Ticket ID: ";
            string tid; getline(cin, tid);
            if (tid.empty()) continue;

            vector<Ticket> found;
            for (const auto& tk : tickets) {
                if (toLowerStr(tk.getId()) == toLowerStr(tid)) {
                    found.push_back(tk);
                    break;
                }
            }

            cout << "\n";
            if (found.empty()) cout << "Ticket not found.\n";
            else ui::PrintTickets(found, trips, routes, buses);
            cout << "\n"; system("pause");
        }
        else if (sel == 2) { // Search by Passenger
            ui::DrawPanel("SEARCH TICKET BY PASSENGER");
            flushConsoleEvents();
            cout << "Passenger name or phone: ";
            string key; getline(cin, key);
            if (key.empty()) continue;
            key = toLowerStr(key);

            vector<Ticket> found;
            for (const auto& tk : tickets) {
                string searchStr = toLowerStr(tk.getPassengerName() + " " + tk.getPhoneNumber());
                if (searchStr.find(key) != string::npos) found.push_back(tk);
            }

            cout << "\n";
            if (found.empty()) cout << "No tickets found.\n";
            else ui::PrintTickets(found, trips, routes, buses);
            cout << "\n"; system("pause");
        }
        else if (sel == 3) { // Filter by Route
            ui::DrawPanel("FILTER TICKETS BY ROUTE");
            flushConsoleEvents();
            
            if (routes.empty()) { cout << "No routes available.\n"; system("pause"); continue; }
            int ridx = pickRoute(routes, "Select Route");
            if (ridx < 0 || ridx >= (int)routes.size()) continue;
            string routeId = routes[ridx].getId();

            // Get all trips for this route
            vector<string> tripIds;
            for (const auto& t : trips) {
                if (toLowerStr(t.getRouteId()) == toLowerStr(routeId)) {
                    tripIds.push_back(t.getId());
                }
            }

            // Filter tickets
            vector<Ticket> found;
            for (const auto& tk : tickets) {
                for (const auto& tid : tripIds) {
                    if (toLowerStr(tk.getTripId()) == toLowerStr(tid)) {
                        found.push_back(tk);
                        break;
                    }
                }
            }

            cout << "\n";
            if (found.empty()) cout << "No tickets for this route.\n";
            else ui::PrintTickets(found, trips, routes, buses);
            cout << "\n"; system("pause");
        }
        else if (sel == 4) { // Filter by Date Range
            ui::DrawPanel("FILTER TICKETS BY DATE RANGE");
            flushConsoleEvents();
            
            cout << "Start Date (YYYY-MM-DD): ";
            string startDate; getline(cin, startDate);
            if (startDate.size() != 10) continue;
            
            cout << "End Date (YYYY-MM-DD): ";
            string endDate; getline(cin, endDate);
            if (endDate.size() != 10) continue;

            vector<Ticket> found;
            for (const auto& tk : tickets) {
                string bookedDate = tk.getBookedAt().substr(0, 10); // Extract YYYY-MM-DD
                if (bookedDate >= startDate && bookedDate <= endDate) {
                    found.push_back(tk);
                }
            }

            cout << "\n";
            if (found.empty()) cout << "No tickets in this date range.\n";
            else {
                ui::PrintTickets(found, trips, routes, buses);
                unsigned long long revenue = 0;
                for (const auto& tk : found) revenue += tk.getPrice();
                cout << "\nTotal tickets: " << found.size() << " | Revenue: " << revenue << " VND\n";
            }
            cout << "\n"; system("pause");
        }
        else if (sel == 5) { // View Tickets by Brand
            ui::DrawPanel("FILTER TICKETS");
            if (brands.empty()) { cout << "No brands available.\n"; system("pause"); continue; }

            int bIdx = pickBrand(brands, "Select Brand");
            if (bIdx < 0 || bIdx >= (int)brands.size()) continue;
            const Brand *targetBrand = &brands[bIdx];

            // Filter Buses belonging to this Brand
            vector<string> brandBusIds;
            for (const auto& b : buses)
                if (b.getBrandId() == targetBrand->getId()) brandBusIds.push_back(b.getId());

            if (brandBusIds.empty()) {
                cout << "\nNo buses found for this brand.\n";
                system("pause");
                continue;
            }

            // Sub-menu: Select Ticket Type (VIP / Standard)
            while (true) {
                string subTitle = "BRAND: " + targetBrand->getName();
                int subSel = ConsoleMenu::pick(subTitle, {
                    "View VIP Tickets",
                    "View Standard Tickets",
                    "Back"
                });

                if (subSel == -1 || subSel == 2) break;

                string typeFilter = (subSel == 0 ? "VIP" : "Standard");
                
                // Filter Tickets
                vector<Ticket> filteredTickets;
                for (const auto &tk : tickets) {
                    // 1. Check if ticket belongs to one of the Brand's buses
                    bool isBrandBus = false;
                    for(const string& bid : brandBusIds) {
                        if(bid == tk.getBusId()) { 
                            isBrandBus = true; 
                            break; 
                        }
                    }
                    if (!isBrandBus) continue;

                    // 2. Check if Bus Type matches the selected filter
                    for (const auto &b : buses) {
                        if (b.getId() == tk.getBusId()) {
                            if (toLowerStr(b.getType()) == toLowerStr(typeFilter)) {
                                filteredTickets.push_back(tk);
                            }
                            break;
                        }
                    }
                }

                // Display Results using Table UI
                ui::DrawPanel(typeFilter + " TICKETS: " + targetBrand->getName());
                
                if (filteredTickets.empty()) {
                    cout << "(No " << typeFilter << " tickets found)\n";
                } else {
                    // Call pretty print function from Table.cpp
                    ui::PrintTickets(filteredTickets, trips, routes, buses);
                }
                
                cout << "\n"; 
                system("pause");
            }
        }
    }
}

// ---------------- SEAT MENU ----------------
static void menuSeat(vector<Seat>& seats, const vector<Bus>& buses) {
    while (true) {
        int sel = ConsoleMenu::pick("SEAT MANAGEMENT", {
            "View Seat Map by Bus",
            "Update Seat Status",
            "Back"
        });
        if (sel == -1 || sel == 2) break;

        if (sel == 0) { // View Seat Map
            ui::DrawPanel("SEAT MAP");
            flushConsoleEvents();
            
            if (buses.empty()) { cout << "No buses available.\n"; system("pause"); continue; }
            int bidx = pickBus(buses, "Select Bus");
            if (bidx < 0 || bidx >= (int)buses.size()) continue;
            string busId = buses[bidx].getId();

            vector<Seat> busSeats;
            for (const auto& s : seats) {
                if (toLowerStr(s.getBusId()) == toLowerStr(busId)) {
                    busSeats.push_back(s);
                }
            }

            cout << "\n[ SEAT MAP: Bus " << busId << " - " << buses[bidx].getName() << " ]\n";
            if (busSeats.empty()) {
                cout << "(No seat data found)\n";
            } else {
                ui::PrintSeats(busSeats);
                
                int booked = 0, total = busSeats.size();
                for (const auto& s : busSeats) if (s.getStatus()) booked++;
                cout << "\nTotal: " << total << " | Booked: " << booked << " | Available: " << (total - booked) << "\n";
            }
            cout << "\n"; system("pause");
        }
        else if (sel == 1) { // Update Seat Status
            ui::DrawPanel("UPDATE SEAT STATUS");
            flushConsoleEvents();
            
            if (buses.empty()) { cout << "No buses available.\n"; system("pause"); continue; }
            int bidx = pickBus(buses, "Select Bus");
            if (bidx < 0 || bidx >= (int)buses.size()) continue;
            string busId = buses[bidx].getId();

            vector<Seat> busSeats;
            for (const auto& s : seats) {
                if (toLowerStr(s.getBusId()) == toLowerStr(busId)) busSeats.push_back(s);
            }

            if (busSeats.empty()) {
                cout << "\n(No seat data found for this bus)\n";
                system("pause");
                continue;
            }

            cout << "\nCurrent Seat Status:\n";
            ui::PrintSeats(busSeats);

            cout << "\nSeat Number to update: ";
            string snoStr; getline(cin, snoStr);
            int sno = -1;
            try { sno = stoi(snoStr); } catch(...) { continue; }

            auto it = find_if(seats.begin(), seats.end(), 
                [&](const Seat& s) { return toLowerStr(s.getBusId()) == toLowerStr(busId) && s.getSeatNo() == sno; });
            
            if (it == seats.end()) {
                cout << "Invalid seat number.\n";
                system("pause");
                continue;
            }

            cout << "Set status (1=Booked, 0=Available): ";
            string statusStr; getline(cin, statusStr);
            bool newStatus = (statusStr == "1");
            
            it->setStatus(newStatus);

            // Update file
            string seatPath = "Data/Seat/" + busId + ".txt";
            vector<string> lines;
            ifstream fin(seatPath);
            string line;
            while (getline(fin, line)) {
                if (line.empty()) continue;
                auto v = splitCSV(line);
                if (v.size() >= 3 && v[1] == snoStr) {
                    v[2] = (newStatus ? "1" : "0");
                }
                lines.push_back(v[0] + "," + v[1] + "," + v[2]);
            }
            fin.close();

            ofstream fout(seatPath, ios::trunc);
            for (const auto& l : lines) fout << l << "\n";
            fout.close();

            cout << "\nSeat " << sno << " updated to: " << (newStatus ? "Booked" : "Available") << "\n";
            cout << "\n"; system("pause");
        }
    }
}

// ---------------- TICKET STATS ----------------
static void menuTicketStats(const vector<Ticket>& tickets,
                            const vector<Trip>& trips,
                            const vector<Route>& routes,
                            const vector<Bus>& buses) {
    while (true) {
        int sel = ConsoleMenu::pick("TICKET STATISTICS", {
            "By Day (Year -> Month -> Day)",
            "By Month (Year -> Month)",
            "By Year",
            "Back"
        });
        if (sel == -1 || sel == 3) break;

        if (sel == 0) {
            ui::DrawPanel("STAT BY DAY");
            auto years = getUniqueYears(tickets);
            if (years.empty()) { cout << "No tickets available.\n"; system("pause"); continue; }
            vector<string> yOpts; for (int y : years) yOpts.push_back(to_string(y));
            int yIdx = ConsoleMenu::pick("Select Year", yOpts);
            if (yIdx < 0 || yIdx >= (int)years.size()) continue;
            int year = years[yIdx];

            auto months = getUniqueMonths(tickets, year);
            if (months.empty()) { cout << "No tickets for this year.\n"; system("pause"); continue; }
            vector<string> mOpts; for (int m : months) { stringstream ss; ss << setfill('0') << setw(2) << m; mOpts.push_back(ss.str()); }
            int mIdx = ConsoleMenu::pick("Select Month", mOpts);
            if (mIdx < 0 || mIdx >= (int)months.size()) continue;
            int month = months[mIdx];

            auto days = getUniqueDays(tickets, year, month);
            if (days.empty()) { cout << "No tickets for this month.\n"; system("pause"); continue; }
            vector<string> dOpts; for (int d : days) { stringstream ss; ss << setfill('0') << setw(2) << d; dOpts.push_back(ss.str()); }
            int dIdx = ConsoleMenu::pick("Select Day", dOpts);
            if (dIdx < 0 || dIdx >= (int)days.size()) continue;
            int day = days[dIdx];

            vector<Ticket> filtered = filterTicketsByDate(tickets, year, month, day);
            printTicketSummary(filtered);
            if (filtered.empty()) cout << "No tickets found for this date.\n";
            else ui::PrintTickets(filtered, trips, routes, buses);
            cout << "\n"; system("pause");
        }
        else if (sel == 1) {
            ui::DrawPanel("STAT BY MONTH");
            auto years = getUniqueYears(tickets);
            if (years.empty()) { cout << "No tickets available.\n"; system("pause"); continue; }
            vector<string> yOpts; for (int y : years) yOpts.push_back(to_string(y));
            int yIdx = ConsoleMenu::pick("Select Year", yOpts);
            if (yIdx < 0 || yIdx >= (int)years.size()) continue;
            int year = years[yIdx];

            auto months = getUniqueMonths(tickets, year);
            if (months.empty()) { cout << "No tickets for this year.\n"; system("pause"); continue; }
            vector<string> mOpts; for (int m : months) { stringstream ss; ss << setfill('0') << setw(2) << m; mOpts.push_back(ss.str()); }
            int mIdx = ConsoleMenu::pick("Select Month", mOpts);
            if (mIdx < 0 || mIdx >= (int)months.size()) continue;
            int month = months[mIdx];

            vector<Ticket> filtered = filterTicketsByDate(tickets, year, month, -1);
            printTicketSummary(filtered);
            if (filtered.empty()) cout << "No tickets found for this month.\n";
            else ui::PrintTickets(filtered, trips, routes, buses);
            cout << "\n"; system("pause");
        }
        else if (sel == 2) {
            ui::DrawPanel("STAT BY YEAR");
            auto years = getUniqueYears(tickets);
            if (years.empty()) { cout << "No tickets available.\n"; system("pause"); continue; }
            vector<string> yOpts; for (int y : years) yOpts.push_back(to_string(y));
            int yIdx = ConsoleMenu::pick("Select Year", yOpts);
            if (yIdx < 0 || yIdx >= (int)years.size()) continue;
            int year = years[yIdx];

            vector<Ticket> filtered = filterTicketsByDate(tickets, year, -1, -1);
            printTicketSummary(filtered);
            if (filtered.empty()) cout << "No tickets found for this year.\n";
            else ui::PrintTickets(filtered, trips, routes, buses);
            cout << "\n"; system("pause");
        }
    }
}
// ---------------- BOOK / CANCEL ----------------
static void menuBooking(vector<Route>& routes, const vector<Trip>& trips,
                        const vector<Bus>& buses, vector<Seat>& seats,
                        vector<Ticket>& tickets) {
    int sel = ConsoleMenu::pick("BOOKING / CANCEL TICKET",
                                { "Book new ticket", "Cancel ticket", "Back" });
    if (sel == -1 || sel == 2) return;

    if (sel == 0) {
        ui::DrawPanel("BOOK NEW TICKET");
        flushConsoleEvents();

        if (routes.empty()) { cout << "No routes available.\n"; system("pause"); return; }
        
        cout << "\n--- 1. SELECT ROUTE ---\n";
        cout << left << setw(10) << "ID" << setw(30) << "Route Name" 
             << setw(10) << "KM" << setw(15) << "Price (VND)" << "\n";
        cout << string(70, '-') << "\n";
        
        for (const auto& r : routes) {
            long distance = 0;
            try { distance = stol(r.getDistance()); } catch(...) {}
            long price = distance * 1000;
            if (price < 50000) price = 50000;

            cout << left << setw(10) << r.getId() 
                 << setw(30) << r.getName() 
                 << setw(10) << r.getDistance()
                 << setw(15) << price << "\n";
        }

        string routeId;
        cout << "\nEnter Route ID (e.g. R001): "; getline(cin, routeId);
        
        const Route* selectedRoute = nullptr;
        for (const auto& r : routes) {
            if (toLowerStr(r.getId()) == toLowerStr(routeId)) {
                selectedRoute = &r;
                break;
            }
        }
        if (!selectedRoute) { cout << "(!) Route ID not found.\n"; system("pause"); return; }

        vector<const Trip*> routeTrips;
        for (const auto& t : trips) {
            if (toLowerStr(t.getRouteId()) == toLowerStr(selectedRoute->getId())) {
                routeTrips.push_back(&t);
            }
        }

        if (routeTrips.empty()) { cout << "(!) No trips available for this route.\n"; system("pause"); return; }

        cout << "\n--- 2. SELECT TRIP ---\n";
        cout << left << setw(10) << "ID" << setw(10) << "Bus" << setw(15) << "Depart" << setw(15) << "Arrive" << "\n";
        cout << string(60, '-') << "\n";
        for (const auto* t : routeTrips) {
            cout << left << setw(10) << t->getId()
                 << setw(10) << t->getBusId()
                 << setw(15) << t->getDepart()
                 << setw(15) << t->getArrival() << "\n";
        }

        string tripId;
        cout << "\nEnter Trip ID (e.g. T001): "; getline(cin, tripId);
        const Trip* selectedTrip = nullptr;
        for (const auto* t : routeTrips) {
            if (toLowerStr(t->getId()) == toLowerStr(tripId)) { selectedTrip = t; break; }
        }
        if (!selectedTrip) { cout << "(!) Invalid Trip ID.\n"; system("pause"); return; }

        string travelDate;
        cout << "\n--- 3. ENTER DATE ---\n";
        cout << "Enter Date (YYYY-MM-DD): "; getline(cin, travelDate);
        if (travelDate.size() != 10) { cout << "(!) Invalid date format.\n"; system("pause"); return; }

        const Bus* currentBus = nullptr;
        for (const auto& b : buses) if (b.getId() == selectedTrip->getBusId()) { currentBus = &b; break; }
        
        vector<int> bookedSeats;
        for (const auto& tk : tickets) {
            if (tk.getTripId() == selectedTrip->getId() && tk.getBookedAt().find(travelDate) == 0) {
                bookedSeats.push_back(tk.getSeatNo());
            }
        }

        cout << "\n--- 4. SELECT SEAT (Bus: " << currentBus->getName() << ") ---\n";
        int capacity = currentBus->getCapacity();
        for (int i = 1; i <= capacity; i++) {
            bool isBooked = false;
            for (int bs : bookedSeats) if (bs == i) isBooked = true;
            if (isBooked) cout << "[ X] "; else cout << "[" << setw(2) << i << "] ";
            if (i % 4 == 0) cout << "\n";
        }
        cout << "\n";

        string seatStr; cout << "Enter Seat Number: "; getline(cin, seatStr);
        int seatNo = 0; try { seatNo = stoi(seatStr); } catch(...) {}
        if (seatNo < 1 || seatNo > capacity) { cout << "(!) Seat number invalid.\n"; system("pause"); return; }
        for (int bs : bookedSeats) if (bs == seatNo) { cout << "(!) Seat already booked.\n"; system("pause"); return; }

        string name, phone;
        cout << "\n--- 5. PASSENGER INFO ---\n";
        cout << "Name: "; getline(cin, name);
        if (name.empty()) return;
        cout << "Phone: "; getline(cin, phone);
        if (phone.empty()) return;

        int payOpt;
        cout << "Payment (1: Cash, 2: Bank Transfer, 3: Momo): "; 
        string payStr; getline(cin, payStr);
        try { payOpt = stoi(payStr); } catch(...) { payOpt=1; }
        string payment = (payOpt == 2) ? "Bank Transfer" : (payOpt == 3 ? "Momo" : "Cash");

        unsigned long finalPrice = 0;
        try { 
            long dist = stol(selectedRoute->getDistance());
            finalPrice = dist * 1000;
            if (finalPrice < 50000) finalPrice = 50000;
        } catch(...) { finalPrice = 50000; }

        string lastId = tickets.empty() ? "TK000" : tickets.back().getId();
        int nextId = 0; try { nextId = stoi(lastId.substr(2)) + 1; } catch(...) {}
        stringstream ss; ss << "TK" << setw(3) << setfill('0') << nextId;
        string ticketId = ss.str();
        string bookedAt = travelDate + " " + nowStr().substr(11);

        Ticket newTk(ticketId, selectedTrip->getId(), currentBus->getId(), seatNo, 
                     name, phone, finalPrice, bookedAt, payment);
        
        tickets.push_back(newTk);
        ofstream f("Data/Ticket/TK001.txt", ios::app);
        f << newTk.toCSV() << "\n";
        f.close();

        cout << "\n>>> BOOKING SUCCESSFUL! <<<\n";
        cout << "Ticket ID: " << ticketId << " | Seat: " << seatNo << "\n";
        cout << "Route: " << selectedRoute->getName() << " (" << selectedRoute->getDistance() << " km)\n";
        cout << "PRICE: " << finalPrice << " VND\n";
        system("pause");
    }
    else if (sel == 1) {
        ui::DrawPanel("CANCEL TICKET");
        flushConsoleEvents();
        cout << "Enter Ticket ID to cancel: ";
        string tid; getline(cin, tid);
        
        bool found = false;
        for (auto it = tickets.begin(); it != tickets.end(); ++it) {
            if (toLowerStr(it->getId()) == toLowerStr(tid)) {
                tickets.erase(it); 
                found = true;
                cout << "Ticket removed from memory (RAM). For permanent deletion, please use Admin UI.\n";
                break;
            }
        }
        if (!found) cout << "Ticket ID not found.\n";
        system("pause");
    }
}
// ---------------- TRIP MENU ----------------
static void menuTrip(vector<Trip>& trips, const vector<Route>& routes, 
                     const vector<Bus>& buses, const vector<Driver>& drivers) {
    while(true) {
        int sel = ConsoleMenu::pick("TRIP MANAGEMENT", {
            "View All Trips",
            "Add New Trip (Smart Schedule)",
            "Update Trip",
            "Delete Trip",
            "Back"
        });
        if (sel == -1 || sel == 4) break;

        // 1 View
        if (sel == 0) { 
            ui::DrawPanel("ALL TRIPS");
            cout << left << setw(8) << "ID" << setw(8) << "Route" << setw(8) << "Bus" 
                 << setw(20) << "Driver" << setw(10) << "Depart" << setw(10) << "Arrive" << "\n";
            cout << string(70, '-') << "\n";
            for(const auto& t : trips) {
                 string drvName = "Unknown"; 
                 for(const auto& d : drivers) if(d.getId() == t.getDriverId()) { drvName = d.getName(); break; }
                 cout << left << setw(8) << t.getId() << setw(8) << t.getRouteId() << setw(8) << t.getBusId()
                 << setw(20) << drvName << setw(10) << t.getDepart() << setw(10) << t.getArrival() << "\n";
            }
            cout << "\n"; system("pause");
        }

        // 2 Add New Trip
        else if (sel == 1) { 
            ui::DrawPanel("ADD NEW TRIP");
            flushConsoleEvents();
            string rid, bid, dep, arr;
            const Route* selectedRoute = nullptr;

            // A Select Route
            if (routes.empty()) { cout << "No routes available.\n"; system("pause"); continue; }
            int ridx = pickRoute(routes, "Select Route");
            if (ridx < 0 || ridx >= (int)routes.size()) continue;
            selectedRoute = &routes[ridx];
            rid = selectedRoute->getId();

            // B Select Bus
            if (buses.empty()) { cout << "No buses available.\n"; system("pause"); continue; }
            int bidx = pickBus(buses, "Select Bus");
            if (bidx < 0 || bidx >= (int)buses.size()) continue;
            bid = buses[bidx].getId();

            // C Departure Time
            int depMin = 0;
            cout << "Departure Time (HH:MM): "; getline(cin, dep);
            if (dep.empty()) continue;
            depMin = timeToMinutes(dep);
            if (depMin == -1) { cout << "Invalid time format.\n"; system("pause"); continue; }

            // D Calc Arrival
            int duration = 0; try { duration = stoi(selectedRoute->getDuration()); } catch(...) { duration = 60; }
            int arrMin = depMin + duration;
            arr = minutesToTime(arrMin);
            cout << "-> Estimated Arrival: " << arr << " (Duration: " << duration << "m)\n";

            if (isBusBusy(bid, depMin, arrMin, rid, trips, routes)) { 
                cout << "\nPlease choose another bus or time.\n";
                system("pause");
                continue; 
            }

            // E Auto Driver
            string did = "Unknown";
            for(const auto& d: drivers) if(toLowerStr(d.getBus()) == toLowerStr(bid)) { did = d.getId(); break; }

            // F Save
            string lastId = "T000"; 
            if (!trips.empty()) lastId = trips.back().getId();
            int next = 0; try { next = stoi(lastId.substr(1)) + 1; } catch(...) {}
            stringstream ss; ss << "T" << setw(3) << setfill('0') << next;
            
            Trip t(ss.str(), rid, bid, did, dep, arr);
            trips.push_back(t);
            Ultil<Trip>::saveToFile("Data/Trip.txt", trips);
            
            cout << "\nSuccess! Trip " << ss.str() << " created.\n";
            system("pause");
        }

        // 3 Update Trip
        else if (sel == 2) { 
            ui::DrawPanel("UPDATE TRIP");
            flushConsoleEvents();
            
            cout << "All Trips:\n";
            cout << left << setw(8) << "ID" << setw(8) << "Route" << setw(8) << "Bus" 
                 << setw(20) << "Driver" << setw(10) << "Depart" << setw(10) << "Arrive" << "\n";
            cout << string(70, '-') << "\n";
            for(const auto& t : trips) {
                 string drvName = "Unknown"; 
                 for(const auto& d : drivers) if(d.getId() == t.getDriverId()) { drvName = d.getName(); break; }
                 cout << left << setw(8) << t.getId() << setw(8) << t.getRouteId() << setw(8) << t.getBusId()
                 << setw(20) << drvName << setw(10) << t.getDepart() << setw(10) << t.getArrival() << "\n";
            }
            
            if (trips.empty()) { cout << "No trips available.\n"; system("pause"); continue; }
            int tidx = pickTrip(trips, drivers, "Select Trip to Update");
            if (tidx < 0 || tidx >= (int)trips.size()) continue;

            Trip* tripToUpdate = &trips[tidx];

            cout << "\nUpdating " << tripToUpdate->getId() << "...\n";
            
            string rid, bid, dep;
            cout << "New Route ID [" << tripToUpdate->getRouteId() << "]: "; getline(cin, rid);
            if(rid.empty()) rid = tripToUpdate->getRouteId();

            cout << "New Bus ID   [" << tripToUpdate->getBusId() << "]: "; getline(cin, bid);
            if(bid.empty()) bid = tripToUpdate->getBusId();

            cout << "New Depart   [" << tripToUpdate->getDepart() << "]: "; getline(cin, dep);
            if(dep.empty()) dep = tripToUpdate->getDepart();

            const Route* rObj = nullptr;
            for(const auto& r : routes) if(toLowerStr(r.getId()) == toLowerStr(rid)) { rObj = &r; break; }
            
            if(!rObj) { cout << "Error: Route ID not valid.\n"; system("pause"); continue; }
            
            int depMin = timeToMinutes(dep);
            if(depMin == -1) { cout << "Error: Invalid time format.\n"; system("pause"); continue; }
            
            int duration = 0; try { duration = stoi(rObj->getDuration()); } catch(...) {}
            int arrMin = depMin + duration;
            string newArr = minutesToTime(arrMin);
            if (isBusBusy(bid, depMin, arrMin, rid, trips, routes, tripToUpdate->getId())) {
                cout << "\nUpdate failed due to conflict.\n";
                system("pause");
                continue;
            }

            string newDid = "Unknown";
            for(const auto& d: drivers) if(toLowerStr(d.getBus()) == toLowerStr(bid)) { newDid = d.getId(); break; }

            *tripToUpdate = Trip(tripToUpdate->getId(), rid, bid, newDid, dep, newArr);

            Ultil<Trip>::saveToFile("Data/Trip.txt", trips);
            cout << "\nTrip updated! New Arrival: " << newArr << "\n";
            system("pause");
        }

        // 4 Delete Trip
        else if (sel == 3) {
            ui::DrawPanel("DELETE TRIP");
            flushConsoleEvents();
            
            cout << "All Trips:\n";
            cout << left << setw(8) << "ID" << setw(8) << "Route" << setw(8) << "Bus" 
                 << setw(20) << "Driver" << setw(10) << "Depart" << setw(10) << "Arrive" << "\n";
            cout << string(70, '-') << "\n";
            for(const auto& t : trips) {
                 string drvName = "Unknown"; 
                 for(const auto& d : drivers) if(d.getId() == t.getDriverId()) { drvName = d.getName(); break; }
                 cout << left << setw(8) << t.getId() << setw(8) << t.getRouteId() << setw(8) << t.getBusId()
                 << setw(20) << drvName << setw(10) << t.getDepart() << setw(10) << t.getArrival() << "\n";
            }
            
            if (trips.empty()) { cout << "No trips available.\n"; system("pause"); continue; }
            int tidx = pickTrip(trips, drivers, "Select Trip to Delete");
            if (tidx < 0 || tidx >= (int)trips.size()) continue;

            trips.erase(trips.begin() + tidx);
            Ultil<Trip>::saveToFile("Data/Trip.txt", trips);
            cout << "Deleted.\n";
            system("pause");
        }
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
            "Manage Brands",
            "Manage Routes",
            "Manage Buses",
            "Manage Drivers",
            "Manage Trips (Link Route & Bus)",
            "Manage Tickets",
            "Manage Seats",
            "Ticket Statistics",
            "Booking / Cancel",
            "Logout",
            "Exit"
        });
        if (sel == -1) continue;
        if (sel == 0)      menuBrand(brands);
        else if (sel == 1) menuRoute(routes);
        else if (sel == 2) menuBus(brands, buses, drivers, seats, routes, trips);
        else if (sel == 3) menuDriver(drivers, buses, trips);
        else if (sel == 4) menuTrip(trips, routes, buses, drivers);
        else if (sel == 5) menuTicket(tickets, trips, routes, buses, brands);
        else if (sel == 6) menuSeat(seats, buses);
        else if (sel == 7) menuTicketStats(tickets, trips, routes, buses);
        else if (sel == 8) menuBooking(routes, trips, buses, seats, tickets);
        else if (sel == 9) { auth.logout(); break; }
        else if (sel == 10) { auth.logout(); exit(0); }
    }
}
static void viewMyTickets(const User& currentUser, const vector<Ticket>& tickets, 
                          const vector<Trip>& trips, const vector<Route>& routes) {
    ui::DrawPanel("MY TICKETS");
    
    string myName = currentUser.getUsername();
    string myPhone = currentUser.getPhoneNumber();
    cout << left << setw(10) << "TicketID" << setw(20) << "Route" << setw(15) << "Date/Time" 
         << setw(5) << "Seat" << setw(15) << "Price" << setw(15) << "Passenger" << "\n";
    cout << string(85, '-') << "\n";

    bool found = false;
    for (const auto& tk : tickets) {
        bool matchName = (toLowerStr(tk.getPassengerName()) == toLowerStr(myName));
        bool matchPhone = (!myPhone.empty() && tk.getPhoneNumber() == myPhone);

        if (matchName || matchPhone) {
            string routeName = tk.getTripId(); 
            for(const auto& t : trips) {
                if(t.getId() == tk.getTripId()) {
                    for(const auto& r : routes) {
                        if(r.getId() == t.getRouteId()) {
                            routeName = r.getStart() + " -> " + r.getEnd();
                            break;
                        }
                    }
                    break;
                }
            }

            cout << left << setw(10) << tk.getId()
                 << setw(20) << routeName.substr(0, 19)
                 << setw(15) << tk.getBookedAt().substr(0, 14)
                 << setw(5) << tk.getSeatNo()
                 << setw(15) << tk.getPrice()
                 << setw(15) << tk.getPassengerName() << "\n";
            found = true;
        }
    }

    if (!found) cout << "\n(You have no tickets, or your tickets haven't been synchronized with your phone number)\n";
    cout << "\n"; system("pause");
}

static void userMenu(AuthManager& auth,
                     vector<Route>& routes, const vector<Trip>& trips,
                     const vector<Bus>& buses, const vector<Driver>& drivers,
                     vector<Seat>& seats, vector<Ticket>& tickets) {
    while (auth.isLoggedIn() && !auth.isAdmin()) {
        int sel = ConsoleMenu::pick("MAIN MENU (USER)", { 
            "Booking / Cancel", 
            "View Bus Info", 
            "View My Tickets",
            "Logout", 
            "Exit" 
        });

        if (sel == -1) continue;
        
        if (sel == 0) {
            menuBooking(routes, trips, buses, seats, tickets);
        }
        else if (sel == 1) {
            if (buses.empty()) { cout << "No buses available.\n"; system("pause"); continue; }
            int idx = pickBus(buses, "Select Bus");
            if (idx < 0 || idx >= (int)buses.size()) continue;
            renderBusDetails(buses[idx], drivers, seats, routes, trips);
            cout << "\n"; system("pause");
        }
        else if (sel == 2) {
            const User* u = auth.getCurrentUser(); 
            if (u) {
                viewMyTickets(*u, tickets, trips, routes);
            } else {
                cout << "User authentication error.\n"; system("pause");
            }
        }
        else if (sel == 3) { auth.logout(); break; }
        else if (sel == 4) { auth.logout(); exit(0); }
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
        users.push_back(User("U001", "admin", "admin123", Role::ADMIN, "999"));
        users.push_back(User("U002", "user1", "123456", Role::USER, "000"));
        ofstream fout("Data/User.txt"); for (auto &u : users) fout << u.toCSV() << "\n"; fout.close();
    }

    // Auth + menus
    for (;;) {
        AuthManager auth;
        if (!loginFlow(auth, users)) break;
        if (auth.isAdmin()) adminMenu(auth, brands, buses, drivers, routes, trips, seats, tickets);
        else                userMenu(auth, routes, trips, buses, drivers, seats, tickets);
    }
    cout << "Goodbye!\n";
    return 0;
}
