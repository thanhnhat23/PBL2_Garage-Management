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
#include "Class/Table.h"     // << dùng bộ in bảng (ui::DrawPanel / ui::Print*)

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
// đảm bảo hậu tố (km / p) cho string số
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
            ui::PrintRoutes(routes); // in bảng đẹp (ID/Route/Distance/Minutes)  :contentReference[oaicite:5]{index=5}
            cout << "\n"; system("pause");
        }
        else if (sel == 1) { // Search
            ui::DrawPanel("SEARCH ROUTE");
            flushConsoleEvents();
            cout << "Keyword (vd: 'da nang - hue' hoac 'hue'): ";
            string key; getline(cin, key); key = toLowerStr(key);

            cout << "\n";
            // lọc rồi in lại bằng bảng
            vector<Route> found;
            for (const auto &r : routes) {
                string name = toLowerStr(r.getName() + " " + r.getStart() + " " + r.getEnd()); // :contentReference[oaicite:6]{index=6}
                if (name.find(key) != string::npos) found.push_back(r);
            }
            if (found.empty()) cout << "(Khong co tuyen phu hop)\n";
            else ui::PrintRoutes(found);
            cout << "\n"; system("pause");
        }
        else if (sel == 2) { // Add
            ui::DrawPanel("ADD NEW ROUTE");
            flushConsoleEvents();
            string name, start, end, dist, dur;
            cout << "Route Name (vd: Da Nang - Hue): "; getline(cin, name);
            cout << "Start Point: "; getline(cin, start);
            cout << "End Point  : "; getline(cin, end);
            cout << "Distance (km): "; getline(cin, dist);
            cout << "Duration (minutes): "; getline(cin, dur);

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
    for (const auto &b : brands) cout << " - " << b.getName() << " (" << b.getId() << ")\n"; // :contentReference[oaicite:7]{index=7}

    cout << "\nEnter brand name: ";
    string input; flushConsoleEvents(); getline(cin, input);
    string key = toLowerStr(input);

    string brandId;
    for (const auto &b : brands) if (toLowerStr(b.getName()) == key) brandId = b.getId(); // :contentReference[oaicite:8]{index=8}
    if (brandId.empty()) { cout << "\nBrand not found.\n"; system("pause"); return; }

    vector<Bus> brandBuses;
    for (const auto &bus : buses) if (bus.getBrandId() == brandId) brandBuses.push_back(bus); // :contentReference[oaicite:9]{index=9}

    cout << "\nBUSES OF BRAND:\n\n";
    if (brandBuses.empty()) { cout << "(No buses)\n\n"; system("pause"); return; }
    ui::PrintBuses(brandBuses); // ID | Plate(getName) | Type | BrandId  :contentReference[oaicite:10]{index=10}

    cout << "\nBus ID to view driver & seats: ";
    string id; getline(cin, id); id = toLowerStr(id);

    const Bus *chosen = nullptr;
    for (const auto& b : brandBuses) if (toLowerStr(b.getId()) == id) { chosen = &b; break; } // :contentReference[oaicite:11]{index=11}
    if (!chosen) { cout << "\nBus not found.\n"; system("pause"); return; }

    vector<Driver> ds; for (auto &d : drivers) if (d.getBus() == chosen->getId()) ds.push_back(d); // :contentReference[oaicite:12]{index=12}
    cout << "\n--- DRIVER INFO ---\n\n";
    if (ds.empty()) cout << "(No driver)\n";
    else ui::PrintDrivers(ds); // Name/Phone/Exp/Bus  :contentReference[oaicite:13]{index=13}

    vector<Seat> ss; for (auto &s : seats) if (s.getBusId() == chosen->getId()) ss.push_back(s); // :contentReference[oaicite:14]{index=14}
    cout << "\n--- SEAT LIST ---\n\n";
    if (ss.empty()) cout << "(No seat file)\n";
    else ui::PrintSeats(ss);  // SeatNo / Status  :contentReference[oaicite:15]{index=15}

    cout << "\n"; system("pause");
}

// ---------------- TICKET MENU ----------------
static void menuTicket(const vector<Ticket> &tickets, const vector<Trip> &trips,
                       const vector<Route> &routes, const vector<Bus> &buses,
                       const vector<Brand> &brands) {
    ui::DrawPanel("TICKET MANAGEMENT");
    cout << "Brands:\n";
    for (const auto &b : brands) cout << " - " << b.getName() << " (" << b.getId() << ")\n"; // :contentReference[oaicite:16]{index=16}

    cout << "\nBrand name to filter: ";
    string name; flushConsoleEvents(); getline(cin, name);

    const Brand *br = nullptr;
    for (const auto& x : brands) if (toLowerStr(x.getName()) == toLowerStr(name)) { br = &x; break; } // :contentReference[oaicite:17]{index=17}
    if (!br) { cout << "\nBrand not found.\n"; system("pause"); return; }

    vector<string> busIds;
    for (const auto& b : buses) if (b.getBrandId() == br->getId()) busIds.push_back(b.getId()); // :contentReference[oaicite:18]{index=18}
    if (busIds.empty()) { cout << "\nNo buses for this brand.\n"; system("pause"); return; }

    // Lọc vé theo brand rồi in bảng tổng hợp có Route/Trip/Type (map từ trips/routes/buses)
    vector<Ticket> tkBrand;
    for (auto &tk : tickets)
        if (find(busIds.begin(), busIds.end(), tk.getBusId()) != busIds.end()) tkBrand.push_back(tk); // :contentReference[oaicite:19]{index=19}

    cout << "\nTICKETS OF BRAND: " << br->getName() << "\n\n"; // :contentReference[oaicite:20]{index=20}
    if (tkBrand.empty()) cout << "(No tickets)\n";
    else ui::PrintTickets(tkBrand, trips, routes, buses); // dùng map Trip/Route/Bus để hiển thị Route/Trip/Type  :contentReference[oaicite:21]{index=21}

    cout << "\n"; system("pause");
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
        int seatTypeChoice, payChoice;
        cout << "Passenger name: "; getline(cin, name);
        cout << "Phone number : "; getline(cin, phone);
        cout << "From         : "; getline(cin, from);
        cout << "To           : "; getline(cin, to);
        cout << "Seat type (1=VIP, 0=Standard): ";
        cin >> seatTypeChoice; cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string seatType = seatTypeChoice == 1 ? "VIP" : "Standard";
        cout << "Payment (1=Chuyen khoan, 2=Tien mat, 3=ZaloPay): ";
        cin >> payChoice; cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string payMethod = (payChoice==1?"Chuyen khoan":(payChoice==2?"Tien mat":"ZaloPay"));

        // Route
        string routeId; const Route* bookedRoute = nullptr;
        for (const auto& r : routes)
            if (toLowerStr(r.getStart()) == toLowerStr(from) &&
                toLowerStr(r.getEnd())   == toLowerStr(to))
            { routeId = r.getId(); bookedRoute = &r; break; } // :contentReference[oaicite:22]{index=22}
        if (routeId.empty()) { cout << "\nNo route found.\n"; system("pause"); return; }

        // Trips còn ghế trống & đúng loại ghế
        vector<const Trip*> foundTrips;
        for (const auto& t : trips) if (toLowerStr(t.getRouteId()) == toLowerStr(routeId)) foundTrips.push_back(&t); // 
        if (foundTrips.empty()) { cout << "\nNo trips for this route.\n"; system("pause"); return; }

        cout << "\nAvailable trips:\n";
        cout << left << setw(8) << "TripID" << setw(8) << "Bus"
             << setw(10) << "Depart" << setw(10) << "Arrive" << "\n";
        for (auto t : foundTrips) {
            for (const auto& b : buses)
                if (b.getId() == t->getBusId() &&
                    toLowerStr(b.getType()) == toLowerStr(seatType)) {           // :contentReference[oaicite:24]{index=24}
                    string seatFile = "Data/Seat/" + b.getId() + ".txt";
                    ifstream fs(seatFile); string line; bool hasEmpty=false;
                    while (getline(fs, line)) {
                        auto v = splitCSV(line);
                        if (v.size() >= 3 && v[2] == "0") { hasEmpty = true; break; }
                    }
                    if (hasEmpty)
                        cout << left << setw(8) << t->getId()
                             << setw(8) << b.getId()
                             << setw(10) << t->getDepart()
                             << setw(10) << t->getArrival() << "\n";              // 
                }
        }

        flushConsoleEvents();
        cout << "\nTrip ID to book: ";
        string tripId; getline(cin, tripId);

        const Trip* chosen = nullptr;
        for (const auto& t : trips)
            if (toLowerStr(t.getId()) == toLowerStr(tripId)) { chosen = &t; break; } // 
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
            cout << "Invalid seat.\n"; system("pause"); return;
        }

        unsigned long price = 0;
        if (bookedRoute) {
            try {
                double d = stod(ensureSuffix(bookedRoute->getDistance(), "")); // "100km" vẫn parse -> 100  :contentReference[oaicite:27]{index=27}
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
                temp.find(busId) != string::npos) { tkFile = ss.str(); break; } // :contentReference[oaicite:28]{index=28}
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
        for (auto &s : seats) if (s.getBusId() == busId && s.getSeatNo() == seatNo) { s.setStatus(true); break; } // :contentReference[oaicite:29]{index=29}
        tickets.push_back(Ticket::fromCSV(newLine)); // :contentReference[oaicite:30]{index=30}

        cout << "\nBooked successfully. Ticket ID: " << tid << "\n";
        cout << "\n"; system("pause");
    }
    else if (sel == 1) { // CANCEL
        ui::DrawPanel("CANCEL TICKET");
        flushConsoleEvents();
        cout << "Passenger name: "; string name; getline(cin, name);
        cout << "Phone number  : "; string phone; getline(cin, phone);

        bool found = false; string busId, seatNo; int seatNoInt = -1;
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
                    found = true; continue;
                }
                keep.push_back(ln);
            }
            if (found) { ofstream fout(path, ios::trunc);
                         for (auto &l2 : keep) fout << l2 << "\n"; fout.close(); break; }
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
            for (auto &s : seats) if (s.getBusId() == busId && s.getSeatNo() == seatNoInt) { s.setStatus(false); break; } // :contentReference[oaicite:31]{index=31}
            cout << "\nCanceled. (" << name << ", " << phone << ")\n";
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
    // Users (tạo mặc định nếu trống)
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
