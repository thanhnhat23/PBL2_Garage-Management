#include "../Class/Ticket.h"
#include <sstream>
#include <iomanip>
#include <iostream>
using namespace std;

// Default constructor
Ticket::Ticket() {
    ticketId = "";
    tripId = "";
    busId = "";
    seatNo = 0;
    passengerName = "";
    phoneNumber = "";
    price = 0;
    bookedAt = "";
    paymentMethod = "";
}

// Full constructor
Ticket::Ticket(string id, string trip, string bus, int seat, string name,
    string phone, unsigned long p, string booked, string pay) {
    ticketId = id;
    tripId = trip;
    busId = bus;
    seatNo = seat;
    passengerName = name;
    phoneNumber = phone;
    price = p;
    bookedAt = booked;
    paymentMethod = pay;
}

// Display ticket info
void Ticket::display() const {
    static bool headerShown = false;
    if (!headerShown) {
        cout << left
             << setw(8) << "MA VE"
             << setw(8) << "CHUYEN"
             << setw(8) << "XE"
             << setw(6) << "GHE"
             << setw(20) << "HANH KHACH"
             << setw(13) << "SDT"
             << setw(10) << "GIA"
             << setw(20) << "DAT LUC"
             << setw(15) << "THANH TOAN"
             << endl;
        headerShown = true;
    }

    cout << left
         << setw(8) << ticketId
         << setw(8) << tripId
         << setw(8) << busId
         << setw(6) << seatNo
         << setw(20) << passengerName
         << setw(13) << phoneNumber
         << setw(10) << price
         << setw(20) << bookedAt
         << setw(15) << paymentMethod
         << endl;
}

// Convert object → CSV
string Ticket::toCSV() const {
    // Convert Ticket object to a CSV line
    return
        ticketId + "," +        // ticketId
        tripId + "," +          // tripId
        busId + "," +           // busId
        to_string(seatNo) + "," +   // seatNo
        passengerName + "," +       // passengerName
        phoneNumber + "," +         // phoneNumber
        to_string(price) + "," +    // price
        bookedAt + "," +            // bookedAt
        paymentMethod;              // paymentMethod
}

// Convert CSV → object
Ticket Ticket::fromCSV(const string& line) {
    vector<string> p;
    string token;
    stringstream ss(line);

    // split all fields
    while (getline(ss, token, ',')) p.push_back(token);
    if (p.size() < 9) return Ticket(); // skip invalid lines

    return Ticket(
        p[0], // ticketId
        p[1], // tripId
        p[2], // busId
        stoi(p[3]), // seatNo
        p[4], // passengerName
        p[5], // phoneNumber
        stoul(p[6]), // price
        p[7], // bookedAt
        p[8]  // paymentMethod
    );
}
