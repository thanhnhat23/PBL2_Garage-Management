#include "../Class/Driver.h"
#include <sstream>
#include <iomanip>
#include <iostream>
using namespace std;

Driver::Driver() {
    driverId = "";
    nameDriver = "";
    phoneDriver = "";
    experienceYears = 0;
}

Driver::Driver(string id, string name, string phone, int exp) {
    driverId = id;
    nameDriver = name;
    phoneDriver = phone;
    experienceYears = exp;
}

void Driver::display() const {
    static bool headerShown = false;
    if (!headerShown) {
        cout << left
             << setw(8) << "ID"
             << setw(20) << "Driver"
             << setw(15) << "Phone"
             << setw(10) << "Exp" << endl;
        headerShown = true;
    }

    cout << left << setw(8) << driverId
         << setw(20) << nameDriver
         << setw(15) << phoneDriver
         << setw(5) << (to_string(experienceYears) + " year") << endl;
}

string Driver::toCSV() const {
    return driverId + "," + nameDriver + "," + phoneDriver + "," + to_string(experienceYears);
}

Driver Driver::fromCSV(const string& line) {
    stringstream ss(line);
    string id, name, phone, expStr;
    getline(ss, id, ',');
    getline(ss, name, ',');
    getline(ss, phone, ',');
    getline(ss, expStr, ',');
    return Driver(id, name, phone, stoi(expStr));
}
