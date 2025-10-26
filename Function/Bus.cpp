#include "../Class/Bus.h"
#include "../Class/Brand.h"
#include <fstream>
#include <iomanip>
#include <iostream>

using namespace std;

// Constructors
Bus::Bus() {
    busId = "";
    licensePlate = "";
    brandId = "";
    capacity = 0;
    type = "";
    active = true;
}

Bus::Bus(string id, string brand, string plate, string t, int cap, bool act) {
    busId = id;
    licensePlate = plate;
    brandId = brand;
    capacity = cap;
    type = t;
    active = act;
}

// Input
void Bus::input() {
    cout << "\nNhap ten hang xe: ";
    string brandName;
    getline(cin, brandName);

    Brand found = Brand::findBrandByName(brandName, "Data/Brand.txt");
    if (found.getId().empty()) {
        cout << "Khong tim thay hang xe \"" << brandName << "\".\n";
        return;
    }

    string typeChoice;
    cout << "Nhap loai xe (VIP / Standard): ";
    getline(cin, typeChoice);

    vector<Bus> allBuses;
    Bus::loadFromFile("Data/Bus.txt", allBuses);

    cout << "\n=== Danh sach xe cua hang " << found.getName()
         << " (" << typeChoice << ") ===\n";
    cout << left << setw(8) << "MaXe"
         << setw(15) << "BienSo"
         << setw(10) << "Loai"
         << setw(8) << "SoGhe"
         << "TrangThai" << endl;

    bool hasResult = false;
    for (auto &b : allBuses) {
        if (b.getBrandId() == found.getId() &&
            (typeChoice.empty() || b.getType() == typeChoice)) {
            b.display();
            hasResult = true;
        }
    }

    if (!hasResult) {
        cout << "Khong co xe nao phu hop voi tieu chi tim kiem.\n";
    }
}

// Display
void Bus::display() const {
    cout << left << setw(8) << busId
         << setw(15) << licensePlate
         << setw(10) << type
         << setw(8) << capacity
         << (active ? "Hoat dong" : "Bao duong") << endl;
}

// File Handling
void Bus::loadFromFile(const string& filename, vector<Bus>& list) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Khong mo duoc file " << filename << endl;
        return;
    }

    list.clear();
    string id, brand, plate, type;
    int cap;
    bool act;
    string line;

    while (getline(file, line)) {
        if (line.empty()) continue;

        size_t p1 = line.find(',');
        size_t p2 = line.find(',', p1 + 1);
        size_t p3 = line.find(',', p2 + 1);
        size_t p4 = line.find(',', p3 + 1);
        size_t p5 = line.find(',', p4 + 1);

        if (p5 == string::npos) continue;

        id    = line.substr(0, p1);
        brand = line.substr(p1 + 1, p2 - p1 - 1);
        plate = line.substr(p2 + 1, p3 - p2 - 1);
        type  = line.substr(p3 + 1, p4 - p3 - 1);
        string capStr = line.substr(p4 + 1, p5 - p4 - 1);
        string actStr = line.substr(p5 + 1);

        try {
            cap = stoi(capStr);
        } catch (...) {
            cerr << "Loi stoi() tai dong: " << line << endl;
            continue;
        }

        act = (actStr == "1");
        list.push_back(Bus(id, brand, plate, type, cap, act));
    }

    file.close();
}

void Bus::saveToFile(const string& filename, const vector<Bus>& list) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Khong mo duoc file " << filename << " de ghi!" << endl;
        return;
    }

    for (auto& b : list) {
        file << b.busId << ","
             << b.brandId << ","
             << b.licensePlate << ","
             << b.type << ","
             << b.capacity << ","
             << (b.active ? "1" : "0") << "\n";
    }

    file.close();
}

// Getters
string Bus::getId() const { return busId; }
string Bus::getBrandId() const { return brandId; }
string Bus::getPlate() const { return licensePlate; }
string Bus::getType() const { return type; }
int Bus::getCapacity() const { return capacity; }
bool Bus::isActive() const { return active; }

// Setters
void Bus::setId(string id) { busId = id; }
void Bus::setBrandId(string brand) { brandId = brand; }
void Bus::setPlate(string plate) { licensePlate = plate; }
void Bus::setType(string t) { type = t; }
void Bus::setCapacity(int cap) { capacity = cap; }
void Bus::setActive(bool act) { active = act; }
