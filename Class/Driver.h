#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "Ultil.h"
using namespace std;

class Driver: public Ultil<Driver> {
    private:
        string driverId;
        string nameDriver;
        string phoneDriver;
        int experienceYears;
    public:
        Driver();
        Driver(string id, string name, string phone, int exp);

        void display() const override;
        string toCSV() const override;
        static Driver fromCSV(const string& line);

        // Getters
        string getId() const { return driverId; }
        string getName() const { return nameDriver; }
        int getExp() const { return experienceYears; }
};