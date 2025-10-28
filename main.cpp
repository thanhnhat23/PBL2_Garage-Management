#include <iostream>
#include <vector>
#include <string>
#include "Class/Brand.h"
#include "Class/Bus.h"
#include "Class/Seat.h"
#include "Class/Ultil.h"
using namespace std;

void menu() {
    cout << "\n===== HE THONG QUAN LY NHA XE =====" << endl;
    cout << "1. Quan ly hang xe" << endl;
    cout << "2. Quan ly tai xe" << endl;
    cout << "3. Quan ly tuyen duong" << endl;
    cout << "4. Quan ly ve xe" << endl;
    cout << "0. Thoat" << endl;
    cout << "===================================" << endl;
    cout << "Chon chuc nang: ";
}

// -----------------------------------------------------------
// Hien thi danh sach xe thuoc hang
// -----------------------------------------------------------
void manageBrand() {
    vector<Brand> brands;
    Ultil<Brand>::loadFromFile("Data/Brand.txt", brands);

    cout << "\n=== DANH SACH HANG XE ===\n";
    for (auto &b : brands) b.display();

    string name;
    cout << "\nNhap ten hang xe can xem: ";
    getline(cin, name);

    // Chuyển về chữ thường để so sánh
    auto toLower = [](string s) {
        for (auto &c : s) c = tolower(c);
        return s;
    };

    bool foundBrand = false;
    string brandId;

    for (auto &b : brands) {
        if (toLower(b.getName()) == toLower(name)) {
            foundBrand = true;
            brandId = b.getId();
            cout << "\n=== THONG TIN HANG XE ===\n";
            b.display();

            // Tìm danh sách xe thuộc hãng
            vector<Bus> buses;
            Ultil<Bus>::loadFromFile("Data/Bus.txt", buses);

            cout << "\n=== DANH SACH XE CUA HANG ===\n";
            bool foundBus = false;
            for (auto &bus : buses) {
                if (bus.getBrandId() == brandId) {
                    bus.display();
                    foundBus = true;
                }
            }
            if (!foundBus) {
                cout << "Khong co xe nao thuoc hang nay.\n";
                return;
            }

            // Chọn xe để xem ghế
            string busSelect;
            cout << "\nNhap MA XE de xem danh sach ghe: ";
            getline(cin, busSelect);

            vector<Seat> seats;
            Ultil<Seat>::loadFromFile("Data/Seat.txt", seats);

            cout << "\n=== DANH SACH GHE CUA XE " << busSelect << " ===\n";
            bool hasSeat = false;
            for (auto &s : seats) {
                if (s.getBusId() == busSelect) {
                    s.display();
                    hasSeat = true;
                }
            }

            if (!hasSeat)
                cout << "Khong co thong tin ghe cho xe nay.\n";
            return;
        }
    }

    if (!foundBrand)
        cout << "Khong tim thay hang xe ten \"" << name << "\".\n";
}

int main() {
    int choice;
    do {
        menu();
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1:
                manageBrand();
                break;

            case 2:
                cout << "\n(Chuc nang quan ly tai xe se duoc bo sung sau...)\n";
                break;

            case 3:
                cout << "\n(Chuc nang quan ly tuyen duong se duoc bo sung sau...)\n";
                break;

            case 4:
                cout << "\n(Chuc nang quan ly ve xe se duoc bo sung sau...)\n";
                break;

            case 0:
                cout << "\nDang thoat chuong trinh...\n";
                break;

            default:
                cout << "Lua chon khong hop le. Vui long nhap lai!\n";
        }
    } while (choice != 0);

    return 0;
}
