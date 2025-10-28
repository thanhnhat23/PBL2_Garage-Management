#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "Ultil.h"
using namespace std;

class Route: public Ultil<Route> {
    private:
        string routeId;
        string nameRoute;
        string distanceKm;
        string estDurationMin;
    public:
        Route();
        Route(string id, string name, string dist, string dur);

        // Virtual function overrides
        void display() const override;
        string toCSV() const override;
        static Route fromCSV(const string& line);

        // Getters
        string getId() const { return routeId; }
        string getName() const { return nameRoute; } // for search
        string getDistance() const { return distanceKm; }
        string getDuration() const { return estDurationMin; }

        // Setters
        void setName(string name) { nameRoute = name; }
        void setDistance(string dist) { distanceKm = dist; }
        void setDuration(string dur) { estDurationMin = dur; }
};