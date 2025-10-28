#pragma once
#include <string>
#include <iostream>
#include "Ultil.h"
using namespace std;

class Ticket : public Ultil<Ticket> {
    private:
        string ticketId;
        string tripId;
        string busId;
        int seatNo;
        string passengerName;
        string phoneNumber;
        unsigned long price;
        string bookedAt;
        string paymentMethod;

    public:
        Ticket();
        Ticket(string id, string trip, string bus, int seat, string name,
            string phone, unsigned long price, string booked, string pay);

        // Override virtual methods
        void display() const override;
        string toCSV() const override;
        static Ticket fromCSV(const string& line);

        // Getters
        string getId() const { return ticketId; }
        string getTripId() const { return tripId; }
        string getBusId() const { return busId; }
        int getSeatNo() const { return seatNo; }
        string getPassengerName() const { return passengerName; }
        string getPhoneNumber() const { return phoneNumber; }
        unsigned long getPrice() const { return price; }
        string getBookedAt() const { return bookedAt; }
        string getPaymentMethod() const { return paymentMethod; }
        string getName() const { return passengerName; } // dùng trong findByName
};
