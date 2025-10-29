#include "../Class/Ultil.h"
#include <sstream>
#include <filesystem>
#include <iomanip>
#include <iostream>
using namespace std;
namespace fs = std::filesystem;

// loadFromFile
template <typename T>
void Ultil<T>::loadFromFile(const string& filename, vector<T>& list) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Khong mo duoc file: " << filename << endl;
        return;
    }

    list.clear();
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        list.push_back(T::fromCSV(line));
    }
    file.close();
}

// saveToFile
template <typename T>
void Ultil<T>::saveToFile(const string& filename, const vector<T>& list) {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "Khong the ghi file: " << filename << endl;
        return;
    }

    for (auto &item : list)
        file << item.toCSV() << "\n";
    file.close();
}

// loadFromFolder
template <typename T>
void Ultil<T>::loadFromFolder(const string& folderPath, vector<T>& list) {
    list.clear();
    if (!fs::exists(folderPath)) {
        cout << "Thu muc khong ton tai: " << folderPath << endl;
        return;
    }

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".txt") {
            ifstream file(entry.path());
            if (!file.is_open()) continue;

            string line;
            while (getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;
                list.push_back(T::fromCSV(line));
            }
            file.close();
        }
    }
}

// saveToFolder
template <typename T>
void Ultil<T>::saveToFolder(const string& folderPath, const vector<T>& list) {
    fs::create_directories(folderPath);
    string outFile = folderPath + "/AllData.txt";
    ofstream file(outFile);
    if (!file.is_open()) {
        cout << "Khong the ghi vao: " << outFile << endl;
        return;
    }

    for (auto &item : list)
        file << item.toCSV() << "\n";
    file.close();
}

// findByName
template <typename T>
void Ultil<T>::findByName(const string& filename, const string& searchName) {
    vector<T> list;
    loadFromFile(filename, list);

    bool found = false;
    cout << "\nKet qua tim kiem voi tu khoa: \"" << searchName << "\"\n";
    for (auto &item : list) {
        auto toLower = [](string s) {
            for (auto &c : s) c = tolower(c);
            return s;
        };

        if (toLower(item.getName()) == toLower(searchName)) {
            item.display();
            found = true;
        }
    }

    if (!found)
        cout << "Khong tim thay ket qua phu hop.\n";
}

// Explicit instantiation
#include "../Class/Brand.h"
#include "../Class/Bus.h"
#include "../Class/Driver.h"
#include "../Class/Route.h"
#include "../Class/Seat.h"
#include "../Class/Ticket.h"
#include "../Class/Trip.h"

template class Ultil<Brand>;
template class Ultil<Bus>;
template class Ultil<Driver>;
template class Ultil<Route>;
template class Ultil<Seat>;
template class Ultil<Ticket>;
template class Ultil<Trip>;
