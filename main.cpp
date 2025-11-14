#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include "Class/Brand.h"
#include "Class/Bus.h"
#include "Class/Driver.h"
#include "Class/Route.h"
#include "Class/Seat.h"
#include "Class/Ticket.h"
#include "Class/Trip.h"
using namespace std;

// Convert a string to lowercase
string toLowerStr(string s) {
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c) { return tolower(c); });
    return s;
}

// Split CSV line into vector<string>
vector<string> splitCSV(const string &line) {
    vector<string> result;
    string token;
    stringstream ss(line);
    while (getline(ss, token, ',')) result.push_back(token);
    return result;
}

// Read a file and convert each line using a loader function
template <typename T>
vector<T> loadData(const string &filePath, T (*loader)(const string &)) {
    vector<T> data;
    ifstream fin(filePath);
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        data.push_back(loader(line));
    }
    fin.close();
    return data;
}

// Menu 1
void menuRoute(const vector<Route> &routes) {
    system("cls");
    cout << "===== ROUTE MANAGEMENT =====\n";
    for (const auto &r : routes) r.display();

    cout << "\nEnter route name or part (e.g. 'da nang - hue'): ";
    string keyword;
    getline(cin, keyword);
    keyword = toLowerStr(keyword);

    cout << "\n--- Search Results ---\n";
    bool found = false;
    for (const auto &r : routes) {
        string name = toLowerStr(r.getName());
        if (name.find(keyword) != string::npos) {
            r.display();
            found = true;
        }
    }
    if (!found) cout << "No routes found!\n";
    system("pause");
}

// Menu 2: Manage Buses
void menuBus(const vector<Brand> &brands, const vector<Bus> &buses,
             const vector<Driver> &drivers, const vector<Seat> &seats) {
    system("cls");
    cout << "===== BUS MANAGEMENT =====\n";
    cout << "Available Brands:\n";
    for (const auto &b : brands)
        cout << b.getName() << " (" << b.getId() << ")\n";

    cout << "\nEnter brand name: ";
    string input;
    getline(cin, input);
    string brandKey = toLowerStr(input);

    // Find brand ID
    string selectedBrandId = "";
    for (const auto &b : brands) {
        if (toLowerStr(b.getName()) == brandKey) {
            selectedBrandId = b.getId();
            break;
        }
    }
    if (selectedBrandId.empty()) {
        cout << "Brand not found!\n";
        system("pause");
        return;
    }

    // Display buses of that brand
    vector<Bus> brandBuses;
    for (const auto &bus : buses) {
        if (bus.getBrandId() == selectedBrandId) {
            brandBuses.push_back(bus);
            bus.display();
        }
    }
    if (brandBuses.empty()) {
        cout << "No buses for this brand.\n";
        system("pause");
        return;
    }

    cout << "\nEnter Bus ID to view driver & seats: ";
    string busId;
    getline(cin, busId);
    busId = toLowerStr(busId);

    const Bus *selectedBus = nullptr;
    for (const auto &b : buses)
        if (toLowerStr(b.getId()) == busId) selectedBus = &b;

    if (!selectedBus) {
        cout << "Bus not found!\n";
        system("pause");
        return;
    }

    // Find driver
    cout << "\n--- DRIVER INFO ---\n";
    for (const auto &d : drivers)
        if (d.getBus() == selectedBus->getId()) d.display();

    // Show seats
    cout << "\n--- SEAT LIST ---\n";
    for (const auto &s : seats)
        if (s.getBusId() == selectedBus->getId())
            cout << "Seat " << setw(2) << s.getSeatNo()
                 << " | Status: " << (s.getStatus() ? "Full" : "Empty") << "\n";

    system("pause");
}

// Menu 3: Manage Tickets
void menuTicket(const vector<Ticket> &tickets, const vector<Trip> &trips,
                const vector<Route> &routes, const vector<Bus> &buses,
                const vector<Brand> &brands) {
    system("cls");
    cout << "===== TICKET MANAGEMENT =====\n";
    cout << "Available Brands:\n";
    for (const auto &b : brands)
        cout << b.getName() << " (" << b.getId() << ")\n";

    // Ask for brand name
    cout << "\nEnter brand name to filter: ";
    string brandName;
    getline(cin, brandName);

    // Find the brand (case-insensitive)
    const Brand* targetBrand = nullptr;
    for (const auto& br : brands)
        if (toLowerStr(br.getName()) == toLowerStr(brandName)) {
            targetBrand = &br;
            break;
        }

    if (!targetBrand) {
        cout << "No brand found with name: " << brandName << "\n";
        system("pause");
        return;
    }

    // Collect all buses that belong to this brand
    vector<string> brandBusIds;
    for (const auto& b : buses)
        if (b.getBrandId() == targetBrand->getId())
            brandBusIds.push_back(b.getId());

    if (brandBusIds.empty()) {
        cout << "No buses found for brand " << targetBrand->getName() << "\n";
        system("pause");
        return;
    }

    cout << "\nShowing all tickets for brand: " << targetBrand->getName() << "\n\n";

    int count = 0;
    for (const auto &tk : tickets) {
        // check if ticket bus belongs to brand
        if (find(brandBusIds.begin(), brandBusIds.end(), tk.getBusId()) == brandBusIds.end())
            continue;

        // Find related trip
        const Trip *trip = nullptr;
        for (const auto &t : trips)
            if (t.getId() == tk.getTripId()) {
                trip = &t;
                break;
            }

        const Route *r = nullptr;
        for (const auto &x : routes)
            if (trip && x.getId() == trip->getRouteId()) r = &x;

        const Bus *bus = nullptr;
        for (const auto &b : buses)
            if (b.getId() == tk.getBusId()) bus = &b;

        // Display formatted ticket line
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
                 << setw(10) << targetBrand->getName() << "  "
                 << bus->getName() << "\n";
            count++;
        }
    }

    if (count == 0)
        cout << "No tickets found for brand: " << targetBrand->getName() << "\n";

    system("pause");
}

// Menu 4: Booking / Cancel Ticket
void menuBooking(const vector<Route>& routes, const vector<Trip>& trips,
                 const vector<Bus>& buses, vector<Seat>& seats,
                 vector<Ticket>& tickets) {
    system("cls");
    cout << "===== BOOKING / CANCEL TICKET =====\n";
    cout << "1. Book new ticket\n";
    cout << "2. Cancel ticket\n";
    cout << "Choice: ";
    string ch;
    getline(cin, ch);

    // BOOKING
    if (ch == "1") {
        string name, phone, from, to;
        int seatTypeChoice, payChoice;
        cout << "Enter passenger name: ";
        getline(cin, name);
        cout << "Enter phone number: ";
        getline(cin, phone);
        cout << "From: ";
        getline(cin, from);
        cout << "To: ";
        getline(cin, to);
        cout << "Seat type (1 = VIP, 0 = Standard): ";
        cin >> seatTypeChoice;
        cin.ignore();
        string seatType = seatTypeChoice == 1 ? "VIP" : "Standard";

        cout << "Payment method (1 = Chuyen khoan, 2 = Tien mat, 3 = ZaloPay): ";
        cin >> payChoice;
        cin.ignore();
        string payMethod;
        if (payChoice == 1) payMethod = "Chuyen khoan";
        else if (payChoice == 2) payMethod = "Tien mat";
        else payMethod = "ZaloPay";

        // Find route
        string routeId = "";
        for (const auto& r : routes)
            if (toLowerStr(r.getStart()) == toLowerStr(from) &&
                toLowerStr(r.getEnd()) == toLowerStr(to)) {
                routeId = r.getId();
                break;
            }
        if (routeId.empty()) {
            cout << "No route found!\n";
            system("pause");
            return;
        }

        // Find trips with that route
        vector<Trip> foundTrips;
        for (const auto& t : trips)
            if (toLowerStr(t.getRouteId()) == toLowerStr(routeId))
                foundTrips.push_back(t);
        if (foundTrips.empty()) {
            cout << "No trips found for this route!\n";
            system("pause");
            return;
        }

        cout << "\nAvailable trips:\n";
        for (auto& t : foundTrips) {
            // Find bus for this trip and check seat type
            for (auto& b : buses)
                if (b.getId() == t.getBusId() && toLowerStr(b.getType()) == toLowerStr(seatType)) {
                    // check if bus has empty seat
                    string seatPath = "Data/Seat/" + b.getId() + ".txt";
                    ifstream fs(seatPath);
                    string line; bool hasEmpty = false;
                    while (getline(fs, line)) {
                        auto v = splitCSV(line);
                        if (v.size() >= 3 && v[2] == "0") { hasEmpty = true; break; }
                    }
                    fs.close();
                    if (hasEmpty)
                        cout << t.getId() << " - Bus: " << b.getId()
                             << " Depart: " << t.getDepart()
                             << " Arrive: " << t.getArrival() << "\n";
                }
        }

        cout << "\nEnter trip ID to book: ";
        string tripId;
        getline(cin, tripId);

        // Find chosen trip
        const Trip* chosen = nullptr;
        for (auto& t : trips)
            if (toLowerStr(t.getId()) == toLowerStr(tripId)) {
                chosen = &t;
                break;
            }
        if (!chosen) {
            cout << "Invalid trip!\n";
            system("pause");
            return;
        }

        string busId = chosen->getBusId();
        string seatPath = "Data/Seat/" + busId + ".txt";

        // Show empty seats
        vector<string> sLines;
        ifstream fs(seatPath);
        string line;
        cout << "\nAvailable seats:\n";
        while (getline(fs, line)) {
            auto v = splitCSV(line);
            if (v.size() >= 3 && v[2] == "0")
                cout << v[1] << " ";
        }
        fs.close();

        cout << "\nEnter seat number to book: ";
        string seatNoStr;
        getline(cin, seatNoStr);

        // Determine which TK00x file belongs to this trip
        string tkFile = "";
        for (int i = 1; i <= 20; i++) {
            stringstream ss;
            ss << "Data/Ticket/TK" << setw(3) << setfill('0') << i << ".txt";
            ifstream test(ss.str());
            string temp;
            if (!test.is_open()) continue;
            getline(test, temp);
            if (!temp.empty() && temp.find(chosen->getId()) != string::npos &&
                temp.find(busId) != string::npos) {
                tkFile = ss.str();
                break;
            }
        }
        if (tkFile.empty()) tkFile = "Data/Ticket/TK001.txt";

        // Append new ticket
        vector<string> lines;
        ifstream fin(tkFile);
        while (getline(fin, line)) if (!line.empty()) lines.push_back(line);
        fin.close();

        string lastId = lines.empty() ? "TK000" : splitCSV(lines.back())[0];
        int next = stoi(lastId.substr(2)) + 1;
        stringstream nid;
        nid << "TK" << setw(3) << setfill('0') << next;
        string newId = nid.str();

        string newLine = newId + "," + chosen->getId() + "," + busId + "," +
                         seatNoStr + "," + name + "," + phone +
                         ",150000,2025-10-29 20:00," + payMethod;
        lines.push_back(newLine);
        ofstream fout(tkFile, ios::trunc);
        for (auto& l : lines) fout << l << "\n";
        fout.close();

        // Update seat file
        vector<string> updated;
        ifstream fs2(seatPath);
        while (getline(fs2, line)) {
            if (line.empty()) continue;
            auto v = splitCSV(line);
            if (v.size() >= 3 && v[1] == seatNoStr) v[2] = "1";
            updated.push_back(v[0] + "," + v[1] + "," + v[2]);
        }
        fs2.close();
        ofstream fout2(seatPath, ios::trunc);
        for (auto& s : updated) fout2 << s << "\n";
        fout2.close();

        cout << "\nTicket booked successfully and saved to " << tkFile << "!\n";
        system("pause");
    }

    // CANCEL
    else if (ch == "2") {
        string name, phone;
        cout << "Enter passenger name to cancel: ";
        getline(cin, name);
        cout << "Enter phone number: ";
        getline(cin, phone);

        bool found = false;
        string busId, seatNo;
        string tkPath = "";

        // Loop through all TK files
        for (int i = 1; i <= 20; i++) {
            stringstream ss;
            ss << "Data/Ticket/TK" << setw(3) << setfill('0') << i << ".txt";
            string path = ss.str();
            ifstream fin(path);
            if (!fin.is_open()) continue;

            vector<string> lines;
            string l;
            while (getline(fin, l)) if (!l.empty()) lines.push_back(l);
            fin.close();

            vector<string> newLines;
            for (auto& ln : lines) {
                auto v = splitCSV(ln);
                if (v.size() < 9) continue;

                // compare case-insensitive name + phone
                if (toLowerStr(v[4]) == toLowerStr(name) &&
                    toLowerStr(v[5]) == toLowerStr(phone)) {
                    busId = v[2];
                    seatNo = v[3];
                    found = true;
                    tkPath = path;
                    continue; // skip this line (delete)
                }
                newLines.push_back(ln);
            }

            // If found, rewrite file immediately and stop searching
            if (found) {
                ofstream fout(path, ios::trunc);
                for (auto& l : newLines) fout << l << "\n";
                fout.close();
                break;
            }
        }

        if (found && !busId.empty()) {
            // Update seat status to empty again
            string seatPath = "Data/Seat/" + busId + ".txt";
            vector<string> updated;
            ifstream fs(seatPath);
            string line;
            while (getline(fs, line)) {
                if (line.empty()) continue;
                auto v = splitCSV(line);
                if (v.size() >= 3 && v[1] == seatNo) v[2] = "0";
                updated.push_back(v[0] + "," + v[1] + "," + v[2]);
            }
            fs.close();

            ofstream fout2(seatPath, ios::trunc);
            for (auto& s : updated) fout2 << s << "\n";
            fout2.close();

            cout << "Ticket canceled successfully for passenger: "
                << name << " (" << phone << ")\n"
                << "Found in file: " << tkPath << "\n";
        } else {
            cout << "No matching ticket found for " << name
                << " (" << phone << ")\n";
        }

        system("pause");
    }
}

// MAIN

int main() {
    vector<Brand> brands = loadData("Data/Brand.txt", Brand::fromCSV);
    vector<Bus> buses = loadData("Data/Bus.txt", Bus::fromCSV);
    vector<Driver> drivers = loadData("Data/Driver.txt", Driver::fromCSV);
    vector<Route> routes = loadData("Data/Route.txt", Route::fromCSV);
    vector<Trip> trips = loadData("Data/Trip.txt", Trip::fromCSV);

    // --- Load all Seat files (B001 -> B020) ---
    vector<Seat> seats;
    for (int i = 1; i <= 20; i++) {
        stringstream path;
        path << "Data/Seat/B" << setw(3) << setfill('0') << i << ".txt";
        ifstream fin(path.str());
        if (fin.is_open()) {
            vector<Seat> temp = loadData(path.str(), Seat::fromCSV);
            seats.insert(seats.end(), temp.begin(), temp.end());
            fin.close();
        }
    }

    // --- Load all Ticket files (TK001 -> TK020) ---
    vector<Ticket> tickets;
    for (int i = 1; i <= 20; i++) {
        stringstream path;
        path << "Data/Ticket/TK" << setw(3) << setfill('0') << i << ".txt";
        ifstream fin(path.str());
        if (fin.is_open()) {
            vector<Ticket> temp = loadData(path.str(), Ticket::fromCSV);
            tickets.insert(tickets.end(), temp.begin(), temp.end());
            fin.close();
        }
    }

    while (true) {
        system("cls");
        cout << "===== MAIN MENU =====\n";
        cout << "1. Manage Routes\n";
        cout << "2. Manage Buses\n";
        cout << "3. Manage Tickets\n";
        cout << "4. Booking/Cancel Ticket\n";
        cout << "0. Exit\n";
        cout << "======================\n";
        cout << "Enter choice: ";
        string choice;
        getline(cin, choice);

        if (choice == "1") menuRoute(routes);
        else if (choice == "2") menuBus(brands, buses, drivers, seats);
        else if (choice == "3") menuTicket(tickets, trips, routes, buses, brands);
        else if (choice == "4") menuBooking(routes, trips, buses, seats, tickets);
        else if (choice == "0") break;
        else {
            cout << "Invalid option!\n";
            system("pause");
        }
    }
    cout << "Goodbye!\n";
    return 0;
}
