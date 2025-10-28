#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

template <typename T>
class Ultil {
    public:
        virtual void display() const = 0;
        virtual string toCSV() const = 0;
        static T fromCSV(const string& line);

        static void loadFromFile(const string& filename, vector<T>& list);
        static void saveToFile(const string& filename, const vector<T>& list);
        static void findByName(const string& filename, const string& searchName);
};