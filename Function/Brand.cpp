#include "../Class/Brand.h"
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

Brand::Brand() {
    brandId = "";
    nameBrand = "";
    hotline = "";
    rating = 0;
}

Brand::Brand(string id, string name, string phone, float rate) {
    brandId = id;
    nameBrand = name;
    hotline = phone;
    rating = rate;
}

void Brand::display() const {
    cout << left << setw(8) << brandId
         << setw(20) << nameBrand
         << setw(15) << hotline
         << setw(5) << fixed << setprecision(1) << rating << endl;
}

string Brand::toCSV() const {
    return brandId + "," + nameBrand + "," + hotline + "," + to_string(rating);
}

Brand Brand::fromCSV(const string& line) {
    stringstream ss(line);
    string id, name, phone, rateStr;
    getline(ss, id, ',');
    getline(ss, name, ',');
    getline(ss, phone, ',');
    getline(ss, rateStr, ',');
    return Brand(id, name, phone, stof(rateStr));
}
