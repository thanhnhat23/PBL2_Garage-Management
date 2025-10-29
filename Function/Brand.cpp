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
    return 
        brandId + "," + 
        nameBrand + "," + 
        hotline + "," + 
        to_string(rating);
}

Brand Brand::fromCSV(const string& line) {
    vector<string> p;
    string token;
    stringstream ss(line);

    // split all fields
    while (getline(ss, token, ',')) p.push_back(token);
    if (p.size() < 4) return Brand(); // skip invalid lines

    return Brand(
        p[0], // brandId
        p[1], // brandName
        p[2], // address
        stof(p[3]) // rating
    );
}
