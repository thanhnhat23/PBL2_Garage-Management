#include <iostream>
#include "Class/Bus.h"
using namespace std;

int main() {
    vector<Bus> buses;
    Bus::loadFromFile("Data/Bus.txt", buses);

    cout << "\n=== DANH SACH XE ===\n";
    for (auto &b : buses)
        b.display();

    Bus newBus;
    newBus.input();

    buses.push_back(newBus);
    Bus::saveToFile("Data/Bus.txt", buses);

    return 0;
}