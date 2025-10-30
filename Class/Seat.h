#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "Ultil.h"
using namespace std;

class Seat: public Ultil<Seat> {
    private:
        string busId;
        int seatNo;
        bool seatStatus; // True = isBooked; False = Available;
    public:
        Seat();
        Seat(string bus, int no, bool booked);

        // Override virtual functions from Ultil
        void display() const override;
        string toCSV() const override;
        static Seat fromCSV(const string& line);

        // Getters
        string getBusId() const { return busId; }
        int getSeatNo() const { return seatNo; }
        bool getStatus() const { return seatStatus; }
        string getName() const { return to_string(seatNo); }

        // Setters
        void setStatus(bool s) { seatStatus = s; }
};
