#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "Ultil.h"
using namespace std;

class Bus: public Ultil<Bus> {
    protected:
        string busId;
        string licensePlate;
        string brandId;
        int capacity;
        string type; // Bus type: VIP or Standard
    public:
        Bus();
        Bus(string id, string brId, string plate, string t, int c);

        void display() const override;
        string toCSV() const override;
        static Bus fromCSV(const string& line);

        // Getters
        string getId() const { return busId; }
        string getBrandId() const { return brandId; }
        string getName() const { return licensePlate; } // for findByName
};