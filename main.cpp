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
#include "Class/Brand.h"
#include "Class/Bus.h"
#include "Class/Driver.h"
#include "Class/Route.h"
#include "Class/Seat.h"
#include "Class/Ticket.h"
#include "Class/Trip.h"
using namespace std;

// Clear input buffer
void clearInputBuffer() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

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

// Get current date and time
string getCurrentDateTime() {
    auto now = chrono::system_clock::now();
    time_t now_time = chrono::system_clock::to_time_t(now);
    
    tm ltm;
    #ifdef _WIN32
        localtime_s(&ltm, &now_time);
    #else
        ltm = *localtime(&now_time);
    #endif

    stringstream ss;
    ss << (1900 + ltm.tm_year) << "-"
       << setfill('0') << setw(2) << (1 + ltm.tm_mon) << "-"
       << setfill('0') << setw(2) << ltm.tm_mday << " "
       << setfill('0') << setw(2) << ltm.tm_hour << ":"
       << setfill('0') << setw(2) << ltm.tm_min;
    return ss.str();
}

// Menu 1
void menuRoute(vector<Route> &routes) {
    string choice;
    while (true) {
        system("cls");
        cout << "===== ROUTE MANAGEMENT =====\n";
        cout << "1. View All Routes\n";
        cout << "2. Search Route\n";
        cout << "3. Add New Route\n";
        cout << "4. Update Route\n";
        cout << "5. Delete Route\n";
        cout << "0. Back to Main Menu\n";
        cout << "============================\n";
        cout << "Enter choice: ";
        getline(cin, choice);

        // 1 See all routes
        if (choice == "1") {
            system("cls");
            cout << "--- All Available Routes ---\n";
            for (const auto &r : routes) r.display();
            system("pause");
        }
        // 2 Search route
        else if (choice == "2") {
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
        // 3 Add new route
        else if (choice == "3") {
            string name, start, end, dist, dur;
            cout << "Enter Route Name (e.g., Da Nang - Hue): ";
            getline(cin, name);
            cout << "Enter Start Point: ";
            getline(cin, start);
            cout << "Enter End Point: ";
            getline(cin, end);
            cout << "Enter Distance (in km, e.g., 100): ";
            getline(cin, dist);
            cout << "Enter Duration (in minutes, e.g., 180): ";
            getline(cin, dur);

            // Generate new Route ID
            string lastId = routes.empty() ? "R000" : routes.back().getId();
            int nextIdNum = stoi(lastId.substr(1)) + 1;
            stringstream ss;
            ss << "R" << setw(3) << setfill('0') << nextIdNum;
            string newId = ss.str();

            Route newRoute(newId, name, start, end, dist, dur);
            routes.push_back(newRoute);
            Ultil<Route>::saveToFile("Data/Route.txt", routes);
            cout << "Route " << newId << " added successfully!\n";
            system("pause");
        }
        // 4 Update route
        else if (choice == "4") {
            cout << "Enter Route ID to update (e.g., R001): ";
            string idToUpdate;
            getline(cin, idToUpdate);

            bool found = false;
            for (auto &r : routes) {
                if (toLowerStr(r.getId()) == toLowerStr(idToUpdate)) {
                    found = true;
                    cout << "Updating route: " << r.getName() << "\n";
                    string name, start, end, dist, dur;
                    
                    cout << "Enter NEW Route Name (current: " << r.getName() << "): ";
                    getline(cin, name);
                    cout << "Enter NEW Start Point (current: " << r.getStart() << "): ";
                    getline(cin, start);
                    cout << "Enter NEW End Point (current: " << r.getEnd() << "): ";
                    getline(cin, end);
                    cout << "Enter NEW Distance (current: " << r.getDistance() << "): ";
                    getline(cin, dist);
                    cout << "Enter NEW Duration (current: " << r.getDuration() << "): ";
                    getline(cin, dur);

                    // Update route details
                    r.setName(name);
                    r.setStart(start);
                    r.setEnd(end);
                    r.setDistance(dist);
                    r.setDuration(dur);

                    Ultil<Route>::saveToFile("Data/Route.txt", routes);
                    cout << "Route " << r.getId() << " updated!\n";
                    break;
                }
            }
            if (!found) cout << "Route ID not found!\n";
            system("pause");
        }
        // 5 Delete route
        else if (choice == "5") {
            cout << "Enter Route ID to delete (e.g., R001): ";
            string idToDelete;
            getline(cin, idToDelete);

            bool erased = false;
            for (auto it = routes.begin(); it != routes.end(); ++it) {
                if (toLowerStr(it->getId()) == toLowerStr(idToDelete)) {
                    cout << "Deleting route: " << it->getName() << "\n";
                    routes.erase(it);
                    erased = true;
                    break;
                }
            }

            if (erased) {
                Ultil<Route>::saveToFile("Data/Route.txt", routes);
                cout << "Route deleted successfully!\n";
            } else {
                cout << "Route ID not found!\n";
            }
            system("pause");
        }
        // 0 Back to main menu
        else if (choice == "0") {
            break;
        }
        else {
            cout << "Invalid option!\n";
            system("pause");
        }
    }
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
void menuBooking(vector<Route>& routes, const vector<Trip>& trips,
                 const vector<Bus>& buses, vector<Seat>& seats,
                 vector<Ticket>& tickets) {
    system("cls");
    cout << "===== BOOKING / CANCEL TICKET =====\n";
    cout << "1. Book new ticket\n";
    cout << "2. Cancel ticket\n";
    cout << "Choice: ";
    string ch;
    getline(cin, ch);

    // 1 Book new ticket
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
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
        string seatType = seatTypeChoice == 1 ? "VIP" : "Standard";

        cout << "Payment method (1 = Chuyen khoan, 2 = Tien mat, 3 = ZaloPay): ";
        cin >> payChoice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
        string payMethod;
        if (payChoice == 1) payMethod = "Chuyen khoan";
        else if (payChoice == 2) payMethod = "Tien mat";
        else payMethod = "ZaloPay";

        // Find route
        string routeId = "";
        const Route* bookedRoute = nullptr;
        for (const auto& r : routes)
            if (toLowerStr(r.getStart()) == toLowerStr(from) &&
                toLowerStr(r.getEnd()) == toLowerStr(to)) {
                routeId = r.getId();
                bookedRoute = &r;
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
            for (auto& b : buses)
                if (b.getId() == t.getBusId() && toLowerStr(b.getType()) == toLowerStr(seatType)) {
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

        vector<int> emptySeats; 
        ifstream fs(seatPath);
        string line;
        cout << "\nAvailable seats:\n";
        while (getline(fs, line)) {
            auto v = splitCSV(line);
            if (v.size() >= 3 && v[2] == "0") {
                cout << v[1] << " ";
                emptySeats.push_back(stoi(v[1])); 
            }
        }
        fs.close();

        cout << "\nEnter seat number to book: ";
        string seatNoStr;
        getline(cin, seatNoStr);
        int seatNoInt;

        try {
            seatNoInt = stoi(seatNoStr);
        } catch (...) {
            cout << "Invalid seat number! Booking failed.\n";
            system("pause");
            return; 
        }

        bool isValidSeat = false;
        for (int emptySeat : emptySeats) {
            if (emptySeat == seatNoInt) {
                isValidSeat = true;
                break;
            }
        }

        if (!isValidSeat) {
            cout << "Error: Seat " << seatNoStr << " is already booked or does not exist. Please try again.\n";
            system("pause");
            return; 
        }

        string currentBookedAt = getCurrentDateTime();
        unsigned long price = 0; 

        if (bookedRoute) {
            try {
                double distance = stod(bookedRoute->getDistance());
                if (distance <= 50) {
                    price = (unsigned long)(distance * 1000);
                } else {
                    price = (unsigned long)(50 * 1000 + (distance - 50) * 1500);
                }
            } catch (...) {
                cout << "Warning: Could not calculate price. Using default.\n";
            }
        } 

        // Find ticket file to save
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

        // Generate new Ticket ID
        vector<string> lines;
        ifstream fin(tkFile);
        while (getline(fin, line)) if (!line.empty()) lines.push_back(line);
        fin.close();

        string lastId = lines.empty() ? "TK000" : splitCSV(lines.back())[0];
        int next = stoi(lastId.substr(2)) + 1;
        stringstream nid;
        nid << "TK" << setw(3) << setfill('0') << next;
        string newId = nid.str();

        // Create new ticket line
        string newLine = newId + "," + chosen->getId() + "," + busId + "," +
                 seatNoStr + "," + name + "," + phone + "," +
                 to_string(price) + "," + currentBookedAt + "," + payMethod;
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
        
        // Update vector 'seats' in memory
        for (auto& s : seats) {
            if (s.getBusId() == busId && s.getSeatNo() == seatNoInt) {
                s.setStatus(true);
                break;
            }
        }
        // Update vector 'tickets' in memory
        tickets.push_back(Ticket::fromCSV(newLine));

        cout << "\nTicket booked successfully and saved to " << tkFile << "!\n";
        cout << "\n===== TICKET CONFIRMATION =====\n";
        
        const Bus* bookedBus = nullptr;
        for (const auto& b : buses) {
            if (b.getId() == busId) {
                bookedBus = &b;
                break;
            }
        }

        cout << left << setw(15) << "Ticket ID:" << newId << "\n";
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
        cout << left << setw(15) << "Booked At:" << currentBookedAt << "\n";
        cout << left << setw(15) << "Payment:" << payMethod << "\n";
        cout << "=================================\n";
        
        system("pause");
    }
    // 2. CANCEL TICKET
    else if (ch == "2") {
        string name, phone;
        cout << "Enter passenger name to cancel: ";
        getline(cin, name);
        cout << "Enter phone number: ";
        getline(cin, phone);

        bool found = false;
        string busId, seatNo;
        int seatNoInt = -1;
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

                if (toLowerStr(v[4]) == toLowerStr(name) &&
                    toLowerStr(v[5]) == toLowerStr(phone)) {
                    busId = v[2];
                    seatNo = v[3];
                    seatNoInt = stoi(v[3]);
                    found = true;
                    tkPath = path;
                    continue; 
                }
                newLines.push_back(ln);
            }

            if (found) {
                // Update ticket file
                ofstream fout(path, ios::trunc);
                for (auto& l : newLines) fout << l << "\n";
                fout.close();
                
                // 1 Update vector 'tickets' in memory
                for (auto it = tickets.begin(); it != tickets.end(); ++it) {
                    if (toLowerStr(it->getPassengerName()) == toLowerStr(name) &&
                        toLowerStr(it->getPhoneNumber()) == toLowerStr(phone)) {
                        tickets.erase(it);
                        break; 
                    }
                }
                break;
            }
        }

        if (found && !busId.empty()) {
            // Update seat file
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

            // 2 Update vector 'seats' in memory
            for (auto& s : seats) {
                if (s.getBusId() == busId && s.getSeatNo() == seatNoInt) {
                    s.setStatus(false);
                    break;
                }
            }

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
