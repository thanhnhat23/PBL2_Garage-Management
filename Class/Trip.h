#pragma once
#include <string>
#include <iostream>
#include <vector>
#include "Ultil.h"
using namespace std;

class Trip {
    private:
        string tripId;
        string routeId;
        string busId;
        string driverId;
        string departTime;
        string arrivalTime;

    public:
        Trip();
        Trip(string id, string route, string bus, string driver,
            string depart, string arrive);

        // Input & Output
        void display() const;

        // CSV Conversion
        string toCSV() const;
        static Trip fromCSV(const string& line);

        // Getter
        string getId() const;
        string getName() const;
};