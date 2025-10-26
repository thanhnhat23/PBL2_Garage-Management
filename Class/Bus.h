#include <string>
#include <vector>
#include <iostream>
using namespace std;

class Bus {
    protected:
        string busId;
        string licensePlate;
        string brandId;
        int capacity;
        string type; // Bus type: VIP or Standard
        bool active; // True = Full; False = Available
    public:
        // Constructors
        Bus();
        Bus(string id, string brand, string plate, string type, int cap, bool active);

        // Input & Output
        void input();
        void display() const;

        // File handling
        static void loadFromFile(const string& filename, vector<Bus>& list);
        static void saveToFile(const string& filename, const vector<Bus>& list);

        // Getters
        string getId() const;
        string getBrandId() const;
        string getPlate() const;
        string getType() const;
        int getCapacity() const;
        bool isActive() const;

        // Setters
        void setId(string id);
        void setBrandId(string brand);
        void setPlate(string plate);
        void setType(string type);
        void setCapacity(int cap);
        void setActive(bool act);
};