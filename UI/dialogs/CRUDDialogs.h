#ifndef CRUDDIALOGS_H
#define CRUDDIALOGS_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QTimeEdit>
#include <QTextEdit>
#include <QLabel>
#include <vector>
#include <string>
#include "../Class/Route.h"
#include "../Class/Bus.h"
#include "../Class/Trip.h"
#include "../Class/Brand.h"
#include "../Class/Driver.h"

// Route Dialog
class RouteDialog : public QDialog {
    Q_OBJECT
public:
    explicit RouteDialog(QWidget *parent = nullptr, Route *editRoute = nullptr);
    Route getRoute() const;
    
private:
    QLineEdit *txtName;
    QLineEdit *txtStart;
    QLineEdit *txtEnd;
    QLineEdit *txtDistance;
    QLineEdit *txtDuration;
    Route *existingRoute;
    std::string routeId;
};

// Bus Dialog
class BusDialog : public QDialog {
    Q_OBJECT
public:
    explicit BusDialog(QWidget *parent = nullptr, 
                      const std::vector<Brand> &brands = {},
                      Bus *editBus = nullptr);
    Bus getBus() const;
    Driver getDriver() const;
    bool hasDriver() const { return createDriver; }
    int getCapacity() const;
    
private:
    QComboBox *cbBrand;
    QLineEdit *txtPlate;
    QComboBox *cbType;
    QSpinBox *spnCapacity;
    
    // Driver fields
    QLineEdit *txtDriverName;
    QLineEdit *txtDriverPhone;
    QSpinBox *spnExperience;
    
    std::vector<Brand> brandList;
    Bus *existingBus;
    std::string busId;
    std::string driverId;
    bool createDriver;
};

// Trip Dialog
class TripDialog : public QDialog {
    Q_OBJECT
public:
    explicit TripDialog(QWidget *parent = nullptr,
                       const std::vector<Route> &routes = {},
                       const std::vector<Bus> &buses = {},
                       const std::vector<Driver> &drivers = {},
                       const std::vector<Trip> &allTrips = {},
                       Trip *editTrip = nullptr);
    Trip getTrip() const;
    
private slots:
    void onRouteChanged(int index);
    void onBusChanged(int index);
    void onDepartureChanged(const QTime &time);
    
private:
    bool validateConflict();
    int timeToMinutes(const std::string &time) const;
    std::string minutesToTime(int minutes) const;
    
    QComboBox *cbRoute;
    QComboBox *cbBus;
    QComboBox *cbDriver;
    QTimeEdit *timeDepart;
    QLabel *lblArrival;
    QLabel *lblConflict;
    
    std::vector<Route> routeList;
    std::vector<Bus> busList;
    std::vector<Driver> driverList;
    std::vector<Trip> tripList;
    Trip *existingTrip;
    std::string tripId;
};

#endif // CRUDDIALOGS_H
