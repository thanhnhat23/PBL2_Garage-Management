#include "../Class/Route.h"
#include <sstream>
#include <iomanip>
#include <iostream>
using namespace std;

// Default constructor
Route::Route() {
    routeId = "";
    nameRoute = "";
    startPoint = "";
    endPoint = "";
    distanceKm = "";
    estDurationMin = "";
}

// Full constructor
Route::Route(string id, string name, string start, string end, string dist, string dur) {
    routeId = id;
    nameRoute = name;
    startPoint = start;
    endPoint = end;
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
    return 
        routeId + "," + 
        nameRoute + "," + 
        startPoint + "," + 
        endPoint + "," + 
        distanceKm + "," + 
        estDurationMin;
}

// Convert CSV line → object
Route Route::fromCSV(const string& line) {
    vector<string> p;
    string token;
    stringstream ss(line);

    // split all fields
    while (getline(ss, token, ',')) p.push_back(token);
    if (p.size() < 6) return Route(); // skip invalid lines

    return Route(
        p[0], // routeId
        p[1], // routeName
        p[2], // startPoint
        p[3], // endPoint
        p[4], // distanceKm
        p[5]  // estDurationMin
    );
}

