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
        string startPoint;
        string endPoint;
        string distanceKm;
        string estDurationMin;
    public:
        Route();
        Route(string id, string name, string start, string end, string dist, string dur);

        // Virtual function overrides
        void display() const override;
        string toCSV() const override;
        static Route fromCSV(const string& line);

        // Getters
        string getId() const { return routeId; }
        string getName() const { return nameRoute; } // for search
        string getStart() const { return startPoint; }
        string getEnd() const { return endPoint; }
        string getDistance() const { return distanceKm; }
        string getDuration() const { return estDurationMin; }

        // Setters
        void setName(string name) { nameRoute = name; }
        void setStart(string start) { startPoint = start; }
        void setEnd(string end) { endPoint = end; }
        void setDistance(string dist) { distanceKm = dist; }
        void setDuration(string dur) { estDurationMin = dur; }
};