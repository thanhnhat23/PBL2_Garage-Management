#include "../Class/Driver.h"
#include <sstream>
#include <iomanip>
#include <iostream>
using namespace std;

Driver::Driver() {
    driverId = "";
    busId = "";
    nameDriver = "";
    phoneDriver = "";
    experienceYears = 0;
}

Driver::Driver(string id, string bus, string name, string phone, int exp) {
    driverId = id;
    busId = bus;
    nameDriver = name;
    phoneDriver = phone;
    experienceYears = exp;
}

void Driver::display() const {
    static bool headerShown = false;
    if (!headerShown) {
        cout << left
             << setw(8) << "ID"
             << setw(20) << "Driver"
             << setw(15) << "Phone"
             << setw(10) << "Exp" << endl;
        headerShown = true;
    }

    cout << left << setw(8) << driverId
         << setw(20) << nameDriver
         << setw(15) << phoneDriver
         << setw(5) << (to_string(experienceYears) + " year") << endl;
}

string Driver::toCSV() const {
    return 
        driverId + "," + 
        busId + "," + 
        nameDriver + "," + 
        phoneDriver + "," + 
        to_string(experienceYears);
}

Driver Driver::fromCSV(const string& line) {
    vector<string> p;
    string token;
    stringstream ss(line);

    // split all fields
    while (getline(ss, token, ',')) p.push_back(token);
    if (p.size() < 5) return Driver(); // skip invalid lines

    return Driver(
        p[0], // driverId
        p[1], // name
        p[2], // license
        p[3], // busId
        stoi(p[4]) // experience
    );
}
