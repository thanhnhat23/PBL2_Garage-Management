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

// Convert object → CSV line
string Seat::toCSV() const {
    return busId + "," + to_string(seatNo) + "," + to_string(seatStatus);
}

// Convert CSV line → object
Seat Seat::fromCSV(const string& line) {
    stringstream ss(line);
    string bus, seatStr, bookedStr;
    getline(ss, bus, ',');
    getline(ss, seatStr, ',');
    getline(ss, bookedStr, ',');
    return Seat(bus, stoi(seatStr), stoi(bookedStr));
}