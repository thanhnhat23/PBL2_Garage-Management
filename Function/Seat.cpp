#include "../Class/Seat.h"
#include <sstream>
#include <iomanip>
#include <iostream>
using namespace std;

// Default constructor
Seat::Seat() {
    busId = "";
    seatNo = 0;
    seatStatus = false;
}

// Full constructor
Seat::Seat(string bus, int no, bool booked) {
    busId = bus;
    seatNo = no;
    seatStatus = booked;
}

// Display seat info
void Seat::display() const {
    cout << left
         << setw(10) << busId
         << setw(8) << seatNo
         << setw(10) << (seatStatus ? "Full" : "Available") << endl;
}

// Convert object -> CSV line
string Seat::toCSV() const {
    return 
        busId + "," + 
        to_string(seatNo) + "," + 
        to_string(seatStatus);
}

// Convert CSV line → object
Seat Seat::fromCSV(const string& line) {
    vector<string> p;
    string token;
    stringstream ss(line);

    // split all fields
    while (getline(ss, token, ',')) p.push_back(token);
    if (p.size() < 3) return Seat(); // skip invalid lines

    return Seat(
        p[0], // busId
        stoi(p[1]), // seatNo
        stoi(p[2])  // seatStatus (0 or 1)
    );
}