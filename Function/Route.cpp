#include "../Class/Route.h"
#include <sstream>
#include <iomanip>
#include <iostream>
using namespace std;

// Default constructor
Route::Route() {
    routeId = "";
    nameRoute = "";
    distanceKm = "";
    estDurationMin = "";
}

// Full constructor
Route::Route(string id, string name, string dist, string dur) {
    routeId = id;
    nameRoute = name;
    distanceKm = dist;
    estDurationMin = dur;
}

// Display route info
void Route::display() const {
    static bool headerShown = false;
    if (!headerShown) {
        cout << left
            << setw(8)  << "ID"
            << setw(25) << "Route"
            << setw(10) << "Distance"
            << setw(10) << "Minutes" << endl;
        headerShown = true;
    }

    cout << left
         << setw(8) << routeId
         << setw(25) << nameRoute
         << setw(10) << (distanceKm + "km")
         << setw(10) << (estDurationMin + "p") << endl;
}

// Convert object → CSV line
string Route::toCSV() const {
    return routeId + "," + nameRoute + "," + distanceKm + "," + estDurationMin;
}

// Convert CSV line → object
Route Route::fromCSV(const string& line) {
    stringstream ss(line);
    string id, name, dist, dur;
    getline(ss, id, ',');
    getline(ss, name, ',');
    getline(ss, dist, ',');
    getline(ss, dur, ',');
    return Route(id, name, dist, dur);
}

