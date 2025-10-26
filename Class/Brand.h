#include <string>
#include <vector>
#include <iostream>
using namespace std;

class Brand {
    private:
        string brandId;
        string nameBrand;
        string hotline;
        double rating;
    public:
        // Constructors
        Brand();
        Brand(string id, string name, string phone, float rate);

        // Input & Output
        void input();
        void display() const;

        // File Handling
        static void loadFromFile(const string& filename, vector<Brand>& list);
        static void saveToFile(const string& filename, const vector<Brand>& list);

        // Search
        static Brand findBrandByName(const string& name, const string& filename);

        // Getters
        string getId() const;
        string getName() const;
        string getHotline() const;
        float getRating() const;

        // Setters
        void setId(string id);
        void setName(string name);
        void setHotline(string phone);
        void setRating(float rate);
};