#include <string>
#include <vector>
#include <iostream>
using namespace std;

class Ticket {
    private:
        string ticketId;
        string tripId;
        string busId;
        string seatNo;
        unsigned long price;
        string bookedAt;
        bool ticketStatus; // True = Full; False = Available
    public:
        //
};