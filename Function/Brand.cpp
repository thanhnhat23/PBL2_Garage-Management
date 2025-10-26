#include "../Class/Brand.h"
#include <fstream>
#include <iomanip>
#include <cctype>

using namespace std;

// ==================== Constructors ====================
Brand::Brand() {
    brandId = "";
    nameBrand = "";
    hotline = "";
    rating = 0.0;
}

Brand::Brand(string id, string name, string phone, float rate) {
    brandId = id;
    nameBrand = name;
    hotline = phone;
    rating = rate;
}

// ==================== Input & Output ====================
void Brand::input() {
    cout << "\nNhap ten hang xe can tim: ";
    getline(cin, nameBrand);

    Brand found = findBrandByName(nameBrand, "Data/Brand.txt");

    if (found.getId().empty()) {
        cout << "Khong tim thay hang xe \"" << nameBrand << "\" trong he thong.\n";
    } else {
        cout << "\n=== Thong tin hang xe ===\n";
        found.display();
        *this = found; // sao chép dữ liệu tìm thấy vào đối tượng hiện tại
    }
}

void Brand::display() const {
    cout << left << setw(8) << brandId
         << setw(20) << nameBrand
         << setw(15) << hotline
         << setw(6) << fixed << setprecision(1) << rating << endl;
}

// ==================== File Handling ====================
void Brand::loadFromFile(const string& filename, vector<Brand>& list) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Khong mo duoc file " << filename << endl;
        return;
    }

    list.clear();
    string id, name, phone;
    float rate;
    string line;

    while (getline(file, line)) {
        if (line.empty()) continue;

        size_t p1 = line.find(',');
        size_t p2 = line.find(',', p1 + 1);
        size_t p3 = line.find(',', p2 + 1);

        if (p1 == string::npos || p2 == string::npos || p3 == string::npos)
            continue;

        id = line.substr(0, p1);
        name = line.substr(p1 + 1, p2 - p1 - 1);
        phone = line.substr(p2 + 1, p3 - p2 - 1);
        rate = stof(line.substr(p3 + 1));

        list.push_back(Brand(id, name, phone, rate));
    }

    file.close();
}

void Brand::saveToFile(const string& filename, const vector<Brand>& list) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Khong mo duoc file " << filename << " de ghi!" << endl;
        return;
    }

    for (auto& b : list) {
        file << b.brandId << ","
             << b.nameBrand << ","
             << b.hotline << ","
             << b.rating << "\n";
    }

    file.close();
}

// ==================== Static Search ====================
// Hàm chuyển chuỗi về chữ thường để so sánh không phân biệt hoa/thường
string toLowerCase(const string& str) {
    string result = str;
    for (auto& c : result) c = tolower(c);
    return result;
}

Brand Brand::findBrandByName(const string& name, const string& filename) {
    vector<Brand> list;
    Brand::loadFromFile(filename, list);

    string searchName = toLowerCase(name);
    for (auto& b : list) {
        if (toLowerCase(b.nameBrand) == searchName)
            return b;
    }
    return Brand(); // Trả về brand rỗng nếu không tìm thấy
}

// ==================== Getters ====================
string Brand::getId() const { return brandId; }
string Brand::getName() const { return nameBrand; }
string Brand::getHotline() const { return hotline; }
float Brand::getRating() const { return rating; }

// ==================== Setters ====================
void Brand::setId(string id) { brandId = id; }
void Brand::setName(string name) { nameBrand = name; }
void Brand::setHotline(string phone) { hotline = phone; }
void Brand::setRating(float rate) { rating = rate; }