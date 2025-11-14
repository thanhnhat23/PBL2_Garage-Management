// File: main.cpp
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

using namespace std;

// ---------- Helpers ----------
string toLowerStr(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return tolower(c); });
    return s;
}
vector<string> splitCSV(const string &line) {
    vector<string> v; string t; stringstream ss(line);
    while (getline(ss, t, ',')) v.push_back(t);
    return v;
}
template <typename T> vector<T> loadData(const string &file, T (*loader)(const string&)) {
    vector<T> out; ifstream f(file); string line;
    while (getline(f, line)) { if (!line.empty()) out.push_back(loader(line)); }
    return out;
}
string nowStr() {
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
static void panel(const string& title) {
    system("cls");
    int W = max(36, (int)title.size() + 6);
    cout << "+" << string(W-2, '-') << "+\n";
    int padL = (W-2 - (int)title.size())/2;
    cout << "|" << string(padL, ' ') << title
         << string(W-2 - padL - (int)title.size(), ' ') << "|\n";
    cout << "+" << string(W-2, '-') << "+\n\n";
}
static void flushConsoleEvents() {
#ifdef _WIN32
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
#endif
}

// ---------------- ROUTE MENU ----------------
void menuRoute(vector<Route> &routes) {
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
            panel("ALL ROUTES");
            for (const auto &r : routes) r.display();
            cout << "\n"; system("pause");
        }
        else if (sel == 1) { // Search
            panel("SEARCH ROUTE");
            flushConsoleEvents();
            cout << "Keyword (e.g. 'da nang - hue'): ";
            string key; getline(cin, key); key = toLowerStr(key);

            cout << "\nResult:\n";
            bool found = false;
            for (const auto &r : routes) {
                string name = toLowerStr(r.getName());
                if (name.find(key) != string::npos) { r.display(); found = true; }
            }
            if (!found) cout << "No routes found.\n";
            cout << "\n"; system("pause");
        }
        else if (sel == 2) { // Add
            panel("ADD NEW ROUTE");
            flushConsoleEvents();
            string name, start, end, dist, dur;
            cout << "Route Name (e.g., Da Nang - Hue): "; getline(cin, name);
            cout << "Start Point: "; getline(cin, start);
            cout << "End Point  : "; getline(cin, end);
            cout << "Distance (km): "; getline(cin, dist);
            cout << "Duration (minutes): "; getline(cin, dur);

            string lastId = routes.empty() ? "R000" : routes.back().getId();
            int next = stoi(lastId.substr(1)) + 1;
            stringstream ss; ss << "R" << setw(3) << setfill('0') << next;
            Route r(ss.str(), name, start, end, dist, dur);
            routes.push_back(r);
            Ultil<Route>::saveToFile("Data/Route.txt", routes);
            cout << "\nAdded route " << r.getId() << " successfully.\n";
            system("pause");
        }
        else if (sel == 3) { // Update
            panel("UPDATE ROUTE");
            flushConsoleEvents();
            cout << "Route ID (e.g., R001): ";
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
                    cout << "\nUpdated.\n";
                    break;
                }
            }
            if (!ok) cout << "Route ID not found.\n";
            system("pause");
        }
        else if (sel == 4) { // Delete
            panel("DELETE ROUTE");
            flushConsoleEvents();
            cout << "Route ID (e.g., R001): ";
            string id; getline(cin, id);

            bool erased = false;
            for (auto it = routes.begin(); it != routes.end(); ++it) {
                if (toLowerStr(it->getId()) == toLowerStr(id)) {
                    cout << "Deleting: " << it->getName() << "\n";
                    routes.erase(it); erased = true; break;
                }
            }
            if (erased) { Ultil<Route>::saveToFile("Data/Route.txt", routes); cout << "Deleted.\n"; }
            else cout << "Route ID not found.\n";
            system("pause");
        }
    }
}

// ---------------- BUS MENU ----------------
void menuBus(const vector<Brand> &brands, const vector<Bus> &buses,
             const vector<Driver> &drivers, const vector<Seat> &seats) {
    panel("BUS MANAGEMENT");
    cout << "Brands:\n";
    for (const auto &b : brands) cout << " - " << b.getName() << " (" << b.getId() << ")\n";

    cout << "\nEnter brand name: ";
    string input; flushConsoleEvents(); getline(cin, input);
    string key = toLowerStr(input);

    string brandId;
    for (const auto &b : brands) if (toLowerStr(b.getName()) == key) brandId = b.getId();
    if (brandId.empty()) { cout << "Brand not found.\n"; system("pause"); return; }

    cout << "\nBuses of brand:\n";
    vector<const Bus*> brandBuses;
    for (const auto &bus : buses) if (bus.getBrandId() == brandId) { brandBuses.push_back(&bus); bus.display(); }
    if (brandBuses.empty()) { cout << "No buses.\n"; system("pause"); return; }

    cout << "\nBus ID to view driver & seats: ";
    string id; getline(cin, id); id = toLowerStr(id);

    const Bus *chosen = nullptr;
    for (auto p : brandBuses) if (toLowerStr(p->getId()) == id) { chosen = p; break; }
    if (!chosen) { cout << "Bus not found.\n"; system("pause"); return; }

    cout << "\n--- DRIVER INFO ---\n";
    for (const auto &d : drivers) if (d.getBus() == chosen->getId()) d.display();

    cout << "\n--- SEAT LIST ---\n";
    for (const auto &s : seats) if (s.getBusId() == chosen->getId())
        cout << "Seat " << setw(2) << s.getSeatNo() << " | Status: " << (s.getStatus() ? "Full" : "Empty") << "\n";

    cout << "\n"; system("pause");
}

// ---------------- TICKET MENU ----------------
void menuTicket(const vector<Ticket> &tickets, const vector<Trip> &trips,
                const vector<Route> &routes, const vector<Bus> &buses,
                const vector<Brand> &brands) {
    panel("TICKET MANAGEMENT");
    cout << "Brands:\n";
    for (const auto &b : brands) cout << " - " << b.getName() << " (" << b.getId() << ")\n";

    cout << "\nBrand name to filter: ";
    string name; flushConsoleEvents(); getline(cin, name);
    const Brand *br = nullptr;
    for (const auto& x : brands) if (toLowerStr(x.getName()) == toLowerStr(name)) { br = &x; break; }
    if (!br) { cout << "Brand not found.\n"; system("pause"); return; }

    vector<string> busIds;
    for (const auto& b : buses) if (b.getBrandId() == br->getId()) busIds.push_back(b.getId());
    if (busIds.empty()) { cout << "No buses for this brand.\n"; system("pause"); return; }

    cout << "\nTickets of brand: " << br->getName() << "\n\n";
    int cnt = 0;
    for (const auto &tk : tickets) {
        if (find(busIds.begin(), busIds.end(), tk.getBusId()) == busIds.end()) continue;

        const Trip *trip = nullptr; for (const auto &t : trips) if (t.getId() == tk.getTripId()) { trip = &t; break; }
        const Route *r   = nullptr; for (const auto &x : routes) if (trip && x.getId() == trip->getRouteId()) { r = &x; break; }
        const Bus   *bus = nullptr; for (const auto &b : buses) if (b.getId() == tk.getBusId()) { bus = &b; break; }

        if (r && trip && bus) {
            cout << setw(6) << left << tk.getId() << "  "
                 << setw(3) << tk.getSeatNo() << "  "
                 << setw(6) << bus->getType() << "  "
                 << setw(18) << tk.getPassengerName() << "  "
                 << setw(13) << tk.getPhoneNumber() << "  "
                 << setw(8) << to_string(tk.getPrice()) + "vnd" << "  "
                 << setw(14) << tk.getBookedAt() << "  "
                 << setw(12) << tk.getPaymentMethod() << "  "
                 << setw(17) << (r ? r->getStart() + " - " + r->getEnd() : "Unknown") << "  "
                 << setw(15) << (trip ? trip->getDepart() + " - " + trip->getArrival() : "Unknown") << "  "
                 << setw(10) << br->getName() << "  "
                 << bus->getName() << "\n";
            cnt++;
        }
    }
    if (cnt == 0) cout << "No tickets.\n";
    cout << "\n"; system("pause");
}

// ---------------- BOOK / CANCEL ----------------
void menuBooking(vector<Route>& routes, const vector<Trip>& trips,
                 const vector<Bus>& buses, vector<Seat>& seats,
                 vector<Ticket>& tickets) {
    int sel = ConsoleMenu::pick("BOOKING / CANCEL TICKET", { "Book new ticket", "Cancel ticket", "Back" });
    if (sel == -1 || sel == 2) return;

    // BOOK
    if (sel == 0) {
        panel("BOOK NEW TICKET");
        flushConsoleEvents();
        string name, phone, from, to;
        int seatTypeChoice, payChoice;
        cout << "Passenger name: "; getline(cin, name);
        cout << "Phone number : "; getline(cin, phone);
        cout << "From         : "; getline(cin, from);
        cout << "To           : "; getline(cin, to);
        cout << "Seat type (1=VIP, 0=Standard): "; cin >> seatTypeChoice; cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string seatType = seatTypeChoice == 1 ? "VIP" : "Standard";
        cout << "Payment (1=Chuyen khoan, 2=Tien mat, 3=ZaloPay): "; cin >> payChoice; cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string payMethod = (payChoice==1?"Chuyen khoan":(payChoice==2?"Tien mat":"ZaloPay"));

        // Find route
        string routeId; const Route* bookedRoute = nullptr;
        for (const auto& r : routes) if (toLowerStr(r.getStart()) == toLowerStr(from) && toLowerStr(r.getEnd()) == toLowerStr(to)) { routeId = r.getId(); bookedRoute = &r; break; }
        if (routeId.empty()) { cout << "\nNo route found.\n"; system("pause"); return; }

        // Find trips
        vector<const Trip*> foundTrips;
        for (const auto& t : trips) if (toLowerStr(t.getRouteId()) == toLowerStr(routeId)) foundTrips.push_back(&t);
        if (foundTrips.empty()) { cout << "\nNo trips for this route.\n"; system("pause"); return; }

        cout << "\nAvailable trips:\n";
        for (auto t : foundTrips) {
            for (const auto& b : buses) if (b.getId() == t->getBusId() && toLowerStr(b.getType()) == toLowerStr(seatType)) {
                string seatFile = "Data/Seat/" + b.getId() + ".txt";
                ifstream fs(seatFile); string line; bool hasEmpty=false;
                while (getline(fs, line)) { auto v = splitCSV(line); if (v.size() >= 3 && v[2] == "0") { hasEmpty = true; break; } }
                if (hasEmpty) cout << " " << t->getId() << "  Bus:" << b.getId()
                                   << "  Depart:" << t->getDepart() << "  Arrive:" << t->getArrival() << "\n";
            }
        }

        flushConsoleEvents();
        cout << "\nTrip ID to book: ";
        string tripId; getline(cin, tripId);

        const Trip* chosen = nullptr;
        for (const auto& t : trips) if (toLowerStr(t.getId()) == toLowerStr(tripId)) { chosen = &t; break; }
        if (!chosen) { cout << "Invalid trip.\n"; system("pause"); return; }

        string busId = chosen->getBusId();
        string seatPath = "Data/Seat/" + busId + ".txt";

        vector<int> emptySeats; ifstream fs(seatPath); string line;
        cout << "\nEmpty seats: ";
        while (getline(fs, line)) { auto v = splitCSV(line); if (v.size() >= 3 && v[2] == "0") { cout << v[1] << " "; emptySeats.push_back(stoi(v[1])); } }
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
                double d = stod(bookedRoute->getDistance());
                price = (d <= 50) ? (unsigned long)(d * 1000) : (unsigned long)(50*1000 + (d - 50) * 1500);
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
            if (!temp.empty() && temp.find(chosen->getId()) != string::npos && temp.find(busId) != string::npos) { tkFile = ss.str(); break; }
        }
        if (tkFile.empty()) tkFile = "Data/Ticket/TK001.txt";

        // new ticket id
        vector<string> lines; ifstream fin(tkFile); while (getline(fin, line)) if (!line.empty()) lines.push_back(line); fin.close();
        string lastId = lines.empty()? "TK000" : splitCSV(lines.back())[0];
        int next = stoi(lastId.substr(2)) + 1; stringstream nid; nid << "TK" << setw(3) << setfill('0') << next; string tid = nid.str();

        string newLine = tid + "," + chosen->getId() + "," + busId + "," + seatNoStr + "," +
                         name + "," + phone + "," + to_string(price) + "," + bookedAt + "," + payMethod;
        lines.push_back(newLine);
        ofstream fout(tkFile, ios::trunc); for (auto &l : lines) fout << l << "\n"; fout.close();

        // update seat file
        vector<string> updated; ifstream fs2(seatPath);
        while (getline(fs2, line)) { if (line.empty()) continue; auto v = splitCSV(line); if (v.size() >= 3 && v[1] == seatNoStr) v[2] = "1"; updated.push_back(v[0]+","+v[1]+","+v[2]); }
        fs2.close();
        ofstream fout2(seatPath, ios::trunc); for (auto &s : updated) fout2 << s << "\n"; fout2.close();
        for (auto &s : seats) if (s.getBusId() == busId && s.getSeatNo() == seatNo) { s.setStatus(true); break; }
        tickets.push_back(Ticket::fromCSV(newLine));

        cout << "\nBooked successfully. Ticket ID: " << tid << "\n"; system("pause");
    }
    // CANCEL
    else if (sel == 1) {
        panel("CANCEL TICKET");
        flushConsoleEvents();
        cout << "Passenger name: "; string name; getline(cin, name);
        cout << "Phone number  : "; string phone; getline(cin, phone);

        bool found = false; string busId, seatNo; int seatNoInt = -1; string tkPath;
        for (int i = 1; i <= 20; i++) {
            stringstream ss; ss << "Data/Ticket/TK" << setw(3) << setfill('0') << i << ".txt";
            string path = ss.str(); ifstream fin(path); if (!fin.is_open()) continue;

            vector<string> lines; string l; while (getline(fin, l)) if (!l.empty()) lines.push_back(l); fin.close();
            vector<string> keep;

            for (auto &ln : lines) {
                auto v = splitCSV(ln);
                if (v.size() < 9) { keep.push_back(ln); continue; }

                if (toLowerStr(v[4]) == toLowerStr(name) && toLowerStr(v[5]) == toLowerStr(phone)) {
                    busId = v[2]; seatNo = v[3]; seatNoInt = stoi(v[3]); found = true; tkPath = path; continue;
                }
                keep.push_back(ln);
            }

            if (found) {
                ofstream fout(path, ios::trunc); for (auto &l2 : keep) fout << l2 << "\n"; fout.close();
                for (auto it = tickets.begin(); it != tickets.end(); ++it) {
                    if (toLowerStr(it->getPassengerName()) == toLowerStr(name) && toLowerStr(it->getPhoneNumber()) == toLowerStr(phone)) {
                        tickets.erase(it); break;
                    }
                }
                break;
            }
        }

        if (found && !busId.empty()) {
            string seatPath = "Data/Seat/" + busId + ".txt";
            vector<string> updated; ifstream fs(seatPath); string line;
            while (getline(fs, line)) { if (line.empty()) continue; auto v = splitCSV(line); if (v.size() >= 3 && v[1] == seatNo) v[2] = "0"; updated.push_back(v[0]+","+v[1]+","+v[2]); }
            fs.close();
            ofstream fout2(seatPath, ios::trunc); for (auto &s : updated) fout2 << s << "\n"; fout2.close();
            for (auto &s : seats) if (s.getBusId() == busId && s.getSeatNo() == seatNoInt) { s.setStatus(false); break; }
            cout << "\nCanceled. (" << name << ", " << phone << ")\n";
        } else {
            cout << "No matching ticket.\n";
        }
        system("pause");
    }
}

// ---------------- LOGIN & ROLE MENUS ----------------
bool loginFlow(AuthManager& auth, const vector<User>& users) {
    auth.load(users);
    while (true) {
        int sel = ConsoleMenu::pick("WELCOME", { "Login", "Exit" });
        if (sel == -1 || sel == 1) return false;
        if (sel == 0) {
            panel("LOGIN");
            flushConsoleEvents();
            cout << "Username: "; string u; getline(cin, u);
            cout << "Password: "; string p; getline(cin, p);
            if (auth.login(u, p)) { cout << "\nLogin success. Hello " << u << ".\n"; system("pause"); return true; }
            else { cout << "\nInvalid username/password.\n"; system("pause"); }
        }
    }
}
void adminMenu(AuthManager& auth,
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
void userMenu(AuthManager& auth,
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
