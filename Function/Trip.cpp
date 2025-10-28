#include "../Class/Trip.h"
#include <sstream>
#include <iomanip>
using namespace std;

Trip::Trip() {
    tripId = routeId = busId = driverId = departTime = arrivalTime = "";
}

Trip::Trip(string id, string route, string bus, string driver,
           string depart, string arrive)
    : tripId(id), routeId(route), busId(bus),
      driverId(driver), departTime(depart), arrivalTime(arrive) {}

void Trip::display() const {
    static bool headerShown = false;
    if (!headerShown) {
        cout << left
             << setw(8) << "MA TRIP"
             << setw(8) << "TUYEN"
             << setw(8) << "XE"
             << setw(10) << "TAI XE"
             << setw(20) << "KHOI HANH"
             << setw(20) << "DEN NOI" << endl;
        headerShown = true;
    }
    cout << left
         << setw(8) << tripId
         << setw(8) << routeId
         << setw(8) << busId
         << setw(10) << driverId
         << setw(20) << departTime
         << setw(20) << arrivalTime << endl;
}

string Trip::toCSV() const {
    return tripId + "," + routeId + "," + busId + "," + driverId + "," +
           departTime + "," + arrivalTime;
}

Trip Trip::fromCSV(const string& line) {
    stringstream ss(line);
    string id, route, bus, driver, depart, arrive;
    getline(ss, id, ',');
    getline(ss, route, ',');
    getline(ss, bus, ',');
    getline(ss, driver, ',');
    getline(ss, depart, ',');
    getline(ss, arrive, ',');
    return Trip(id, route, bus, driver, depart, arrive);
}

string Trip::getId() const { return tripId; }
string Trip::getName() const { return routeId; }
