#pragma once
#include <vector>
#include <string>
#include "Brand.h"
#include "Route.h"
#include "Bus.h"
#include "Driver.h"
#include "Seat.h"
#include "Ticket.h"
#include "Trip.h"

namespace ui {
    void DrawPanel(const std::string& title);
    void PrintBrands(const std::vector<Brand>& brands);
    void PrintRoutes(const std::vector<Route>& routes);
    void PrintBuses(const std::vector<Bus>& buses);
    void PrintDrivers(const std::vector<Driver>& drivers);
    void PrintSeats(const std::vector<Seat>& seats);
    void PrintTickets(const std::vector<Ticket>& tickets,
                    const std::vector<Trip>& trips,
                    const std::vector<Route>& routes,
                    const std::vector<Bus>& buses);
}
