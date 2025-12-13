#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "Ultil.h"
using namespace std;

class Brand: public Ultil<Brand> {
    private:
        string brandId;
        string nameBrand;
        string hotline;
        double rating;
    public:
        Brand();
        Brand(string id, string name, string phone, float rate);

        // Implemented from Ultil
        void display() const override;
        string toCSV() const override;
        static Brand fromCSV(const string& line);

        // Getter
        string getId() const { return brandId; }
        string getName() const { return nameBrand; }
        string getHotline() const { return hotline; }
        double getRating() const { return rating; }
        
        // Setter
        void setName(const string& name) { nameBrand = name; }
        void setHotline(const string& phone) { hotline = phone; }
        void setRating(double rate) { rating = rate; }
};
