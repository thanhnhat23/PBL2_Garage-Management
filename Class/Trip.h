#include <string>
#include <vector>
#include <iostream>
using namespace std;

class Trip {
    private:
        string tripId;
        string busId;
        string driverId;
        string routeId;
        string departTime;
        string arrivalTime;
        bool TripStatus; // True = Full; False = Available

};