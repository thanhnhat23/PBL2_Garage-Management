#include "../Class/Bus.h"
#include <sstream>
#include <iomanip>
#include <iostream>
using namespace std;

Bus::Bus() {
    busId = "";
    brandId = "";
    licensePlate = "";
    type = "";
    capacity = 0;
}

Bus::Bus(string id, string brId, string plate, string t, int c) {
    busId = id;
    brandId = brId;
    licensePlate = plate;
    type = t;
    capacity = c;
}

void Bus::display() const {
    static bool headerShown = false;
    if (!headerShown) {
        cout << left
            << setw(8)  << "ID"
            << setw(12) << "License"
            << setw(10) << "Type"
            << setw(12)  << "Capacity" << endl;
        headerShown = true;
    }
    cout << left
         << setw(8) << busId
         << setw(12) << licensePlate
         << setw(13) << type
         << setw(9) << capacity << endl;
}

string Bus::toCSV() const {
    return 
        busId + "," + 
        brandId + "," +
        licensePlate + "," + 
        type + "," + 
        to_string(capacity);
}

Bus Bus::fromCSV(const string& line) {
    vector<string> p;
    string token;
    stringstream ss(line);

    // split all fields
    while (getline(ss, token, ',')) p.push_back(token);
    if (p.size() < 5) return Bus(); // skip invalid lines

    return Bus(
        p[0], // busId
        p[1], // name
        p[2], // type
        p[3], // brandId
        stoi(p[4]) // seats
    );
}
