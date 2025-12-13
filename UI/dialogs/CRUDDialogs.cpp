#include "CRUDDialogs.h"
#include "../Class/Ultil.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QGroupBox>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <fstream>

// ============= Route Dialog =============
RouteDialog::RouteDialog(QWidget *parent, Route *editRoute)
    : QDialog(parent), existingRoute(editRoute)
{
    setWindowTitle(editRoute ? "Edit Route" : "Add New Route");
    setMinimumWidth(500);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();
    
    txtName = new QLineEdit(this);
    txtName->setPlaceholderText("e.g., Da Nang - Hue");
    
    txtStart = new QLineEdit(this);
    txtStart->setPlaceholderText("e.g., Da Nang");
    
    txtEnd = new QLineEdit(this);
    txtEnd->setPlaceholderText("e.g., Hue");
    
    txtDistance = new QLineEdit(this);
    txtDistance->setPlaceholderText("e.g., 100");
    
    txtDuration = new QLineEdit(this);
    txtDuration->setPlaceholderText("e.g., 180 (in minutes)");
    
    if (editRoute) {
        routeId = editRoute->getId();
        // Don't load name - it will be auto-generated from Start - End
        txtStart->setText(QString::fromStdString(editRoute->getStart()));
        txtEnd->setText(QString::fromStdString(editRoute->getEnd()));
        txtDistance->setText(QString::fromStdString(editRoute->getDistance()));
        txtDuration->setText(QString::fromStdString(editRoute->getDuration()));
    }
    
    // Only show Start, End, Distance, Duration - Name will be auto-generated
    formLayout->addRow("Start Location:", txtStart);
    formLayout->addRow("End Location:", txtEnd);
    formLayout->addRow("Distance (km):", txtDistance);
    formLayout->addRow("Duration (minutes):", txtDuration);
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnSave = new QPushButton(editRoute ? "Update" : "Add", this);
    QPushButton *btnCancel = new QPushButton("Cancel", this);
    
    btnSave->setStyleSheet("background: #FDB515; color: white; padding: 8px 20px; border-radius: 5px; font-weight: bold;");
    btnCancel->setStyleSheet("background: #EEEEEE; color: #333; padding: 8px 20px; border-radius: 5px;");
    
    connect(btnSave, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    
    btnLayout->addStretch();
    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnCancel);
    
    mainLayout->addLayout(formLayout);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(btnLayout);
}

Route RouteDialog::getRoute() const {
    std::string start = txtStart->text().toStdString();
    std::string end = txtEnd->text().toStdString();
    // Auto-format name as "Start - End"
    std::string name = start + " - " + end;
    std::string distance = txtDistance->text().toStdString();
    std::string duration = txtDuration->text().toStdString();
    
    return Route(routeId, name, start, end, distance, duration);
}

// ============= Bus Dialog =============
BusDialog::BusDialog(QWidget *parent, const std::vector<Brand> &brands, Bus *editBus)
    : QDialog(parent), brandList(brands), existingBus(editBus), createDriver(!editBus)
{
    setWindowTitle(editBus ? "Edit Bus" : "Add New Bus");
    setMinimumWidth(600);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Bus Information Group
    QGroupBox *busGroup = new QGroupBox("Bus Information", this);
    QFormLayout *busForm = new QFormLayout(busGroup);
    
    cbBrand = new QComboBox(this);
    for (const auto &brand : brands) {
        cbBrand->addItem(QString::fromStdString(brand.getName()), 
                         QString::fromStdString(brand.getId()));
    }
    
    txtPlate = new QLineEdit(this);
    txtPlate->setPlaceholderText("e.g., 43A-99999");
    
    cbType = new QComboBox(this);
    cbType->addItem("VIP");
    cbType->addItem("Standard");
    
    spnCapacity = new QSpinBox(this);
    spnCapacity->setRange(10, 60);
    spnCapacity->setValue(30);
    
    if (editBus) {
        busId = editBus->getId();
        txtPlate->setText(QString::fromStdString(editBus->getName()));
        cbType->setCurrentText(QString::fromStdString(editBus->getType()));
        spnCapacity->setValue(editBus->getCapacity());
        
        // Find brand index
        for (int i = 0; i < cbBrand->count(); ++i) {
            if (cbBrand->itemData(i).toString().toStdString() == editBus->getBrandId()) {
                cbBrand->setCurrentIndex(i);
                break;
            }
        }
    }
    
    busForm->addRow("Brand:", cbBrand);
    busForm->addRow("License Plate:", txtPlate);
    busForm->addRow("Type:", cbType);
    busForm->addRow("Capacity:", spnCapacity);
    
    mainLayout->addWidget(busGroup);
    
    // Driver Information Group (only for new buses)
    if (!editBus) {
        QGroupBox *driverGroup = new QGroupBox("Driver Information (Optional for new bus)", this);
        QFormLayout *driverForm = new QFormLayout(driverGroup);
        
        txtDriverName = new QLineEdit(this);
        txtDriverName->setPlaceholderText("Driver's full name");
        
        txtDriverPhone = new QLineEdit(this);
        txtDriverPhone->setPlaceholderText("Phone number");
        
        spnExperience = new QSpinBox(this);
        spnExperience->setRange(0, 50);
        spnExperience->setValue(5);
        spnExperience->setSuffix(" years");
        
        driverForm->addRow("Driver Name:", txtDriverName);
        driverForm->addRow("Phone:", txtDriverPhone);
        driverForm->addRow("Experience:", spnExperience);
        
        mainLayout->addWidget(driverGroup);
    }
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnSave = new QPushButton(editBus ? "Update" : "Add", this);
    QPushButton *btnCancel = new QPushButton("Cancel", this);
    
    btnSave->setStyleSheet("background: #FDB515; color: white; padding: 8px 20px; border-radius: 5px; font-weight: bold;");
    btnCancel->setStyleSheet("background: #EEEEEE; color: #333; padding: 8px 20px; border-radius: 5px;");
    
    connect(btnSave, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    
    btnLayout->addStretch();
    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnCancel);
    
    mainLayout->addLayout(btnLayout);
}

Bus BusDialog::getBus() const {
    std::string brandId = cbBrand->currentData().toString().toStdString();
    std::string plate = txtPlate->text().toStdString();
    std::string type = cbType->currentText().toStdString();
    int capacity = spnCapacity->value();
    
    return Bus(busId, brandId, plate, type, capacity);
}

Driver BusDialog::getDriver() const {
    if (!createDriver) return Driver();
    
    std::string name = txtDriverName->text().toStdString();
    std::string phone = txtDriverPhone->text().toStdString();
    int exp = spnExperience->value();
    
    return Driver(driverId, busId, name, phone, exp);
}

int BusDialog::getCapacity() const {
    return spnCapacity->value();
}

// ============= Trip Dialog =============
TripDialog::TripDialog(QWidget *parent, const std::vector<Route> &routes,
                      const std::vector<Bus> &buses, const std::vector<Driver> &drivers,
                      const std::vector<Trip> &allTrips, Trip *editTrip)
    : QDialog(parent), routeList(routes), busList(buses), 
      driverList(drivers), tripList(allTrips), existingTrip(editTrip)
{
    setWindowTitle(editTrip ? "Edit Trip" : "Add New Trip");
    setMinimumWidth(600);
    
    if (editTrip) {
        tripId = editTrip->getId();
    }
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();
    
    // Route Selection
    cbRoute = new QComboBox(this);
    for (const auto &route : routes) {
        QString display = QString::fromStdString(route.getId() + ": " + route.getName());
        cbRoute->addItem(display, QString::fromStdString(route.getId()));
    }
    connect(cbRoute, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &TripDialog::onRouteChanged);
    
    // Bus Selection
    cbBus = new QComboBox(this);
    for (const auto &bus : buses) {
        QString display = QString::fromStdString(bus.getId() + ": " + bus.getName() + 
                                                " (" + bus.getType() + ")");
        cbBus->addItem(display, QString::fromStdString(bus.getId()));
    }
    connect(cbBus, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &TripDialog::onBusChanged);
    
    // Driver Selection
    cbDriver = new QComboBox(this);
    
    // Departure Time
    timeDepart = new QTimeEdit(this);
    timeDepart->setDisplayFormat("HH:mm");
    timeDepart->setTime(QTime(8, 0));
    connect(timeDepart, &QTimeEdit::timeChanged, this, &TripDialog::onDepartureChanged);
    
    // Arrival Time (calculated)
    lblArrival = new QLabel("--:--", this);
    lblArrival->setStyleSheet("font-weight: bold; color: #FDB515;");
    
    // Conflict Warning
    lblConflict = new QLabel("", this);
    lblConflict->setStyleSheet("color: red; font-weight: bold;");
    lblConflict->setWordWrap(true);
    
    formLayout->addRow("Route:", cbRoute);
    formLayout->addRow("Bus:", cbBus);
    formLayout->addRow("Driver:", cbDriver);
    formLayout->addRow("Departure:", timeDepart);
    formLayout->addRow("Arrival (Calculated):", lblArrival);
    formLayout->addRow("", lblConflict);
    
    // Load existing data if editing
    if (editTrip) {
        // Set route
        for (int i = 0; i < cbRoute->count(); ++i) {
            if (cbRoute->itemData(i).toString().toStdString() == editTrip->getRouteId()) {
                cbRoute->setCurrentIndex(i);
                break;
            }
        }
        
        // Set bus
        for (int i = 0; i < cbBus->count(); ++i) {
            if (cbBus->itemData(i).toString().toStdString() == editTrip->getBusId()) {
                cbBus->setCurrentIndex(i);
                break;
            }
        }
        
        // Set departure
        std::string dep = editTrip->getDepart();
        if (dep.length() >= 5) {
            int h = std::stoi(dep.substr(0, 2));
            int m = std::stoi(dep.substr(3, 2));
            timeDepart->setTime(QTime(h, m));
        }
        
        // Set arrival (show existing value)
        lblArrival->setText(QString::fromStdString(editTrip->getArrival()));
    } else {
        onRouteChanged(0);
        onBusChanged(0);
    }
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnSave = new QPushButton(editTrip ? "Update" : "Add", this);
    QPushButton *btnCancel = new QPushButton("Cancel", this);
    
    btnSave->setStyleSheet("background: #FDB515; color: white; padding: 8px 20px; border-radius: 5px; font-weight: bold;");
    btnCancel->setStyleSheet("background: #EEEEEE; color: #333; padding: 8px 20px; border-radius: 5px;");
    
    connect(btnSave, &QPushButton::clicked, [this]() {
        if (validateConflict()) {
            accept();
        }
    });
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    
    btnLayout->addStretch();
    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnCancel);
    
    mainLayout->addLayout(formLayout);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(btnLayout);
}

void TripDialog::onRouteChanged(int index) {
    if (index < 0 || index >= (int)routeList.size()) return;
    onDepartureChanged(timeDepart->time());
}

void TripDialog::onBusChanged(int index) {
    if (index < 0 || index >= (int)busList.size()) return;
    
    std::string selectedBusId = cbBus->currentData().toString().toStdString();
    
    // Update driver list for this bus
    cbDriver->clear();
    for (const auto &driver : driverList) {
        if (driver.getBus() == selectedBusId) {
            QString display = QString::fromStdString(driver.getId() + ": " + driver.getName());
            cbDriver->addItem(display, QString::fromStdString(driver.getId()));
        }
    }
    
    if (cbDriver->count() == 0) {
        cbDriver->addItem("No driver assigned", "");
    }
    
    validateConflict();
}

void TripDialog::onDepartureChanged(const QTime &time) {
    // Calculate arrival time based on route duration
    int routeIdx = cbRoute->currentIndex();
    if (routeIdx < 0 || routeIdx >= (int)routeList.size()) return;
    
    const Route &route = routeList[routeIdx];
    int duration = 0;
    try {
        duration = std::stoi(route.getDuration());
    } catch (...) {}
    
    int depMin = time.hour() * 60 + time.minute();
    int arrMin = depMin + duration;
    
    int arrHour = (arrMin / 60) % 24;
    int arrMinute = arrMin % 60;
    
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << arrHour << ":" 
       << std::setw(2) << arrMinute;
    
    lblArrival->setText(QString::fromStdString(ss.str()));
    
    validateConflict();
}

bool TripDialog::validateConflict() {
    std::string busId = cbBus->currentData().toString().toStdString();
    std::string routeId = cbRoute->currentData().toString().toStdString();
    
    if (busId.empty() || routeId.empty()) return true;
    
    // Get departure and arrival times
    QTime dep = timeDepart->time();
    int depMin = dep.hour() * 60 + dep.minute();
    
    QString arrText = lblArrival->text();
    QTime arr = QTime::fromString(arrText, "HH:mm");
    int arrMin = arr.hour() * 60 + arr.minute();
    
    // Find route duration for return buffer
    int routeDuration = 0;
    for (const auto &r : routeList) {
        if (r.getId() == routeId) {
            try { routeDuration = std::stoi(r.getDuration()); } catch (...) {}
            break;
        }
    }
    
    int returnBuffer = routeDuration / 2;
    
    // Check conflicts with other trips
    for (const auto &trip : tripList) {
        // Skip if editing and this is the same trip
        if (existingTrip && trip.getId() == existingTrip->getId()) continue;
        
        // Only check same bus
        if (trip.getBusId() != busId) continue;
        
        // Get trip times
        int otherDep = timeToMinutes(trip.getDepart());
        int otherArr = timeToMinutes(trip.getArrival());
        
        // Get route duration for this trip's return buffer
        int otherRouteDur = 0;
        for (const auto &r : routeList) {
            if (r.getId() == trip.getRouteId()) {
                try { otherRouteDur = std::stoi(r.getDuration()); } catch (...) {}
                break;
            }
        }
        int otherReturnBuffer = otherRouteDur / 2;
        
        // Conflict check: new trip departs before other trip returns
        if (depMin >= otherDep && depMin <= otherArr + otherReturnBuffer) {
            lblConflict->setText("⚠ CONFLICT: Bus busy with Trip " + 
                                QString::fromStdString(trip.getId()) + 
                                " until " + QString::fromStdString(trip.getArrival()) + 
                                " + " + QString::number(otherReturnBuffer) + " min return");
            return false;
        }
        
        // Conflict check: new trip still running when other trip departs
        if (otherDep >= depMin && otherDep <= arrMin + returnBuffer) {
            lblConflict->setText("⚠ CONFLICT: This trip conflicts with Trip " + 
                                QString::fromStdString(trip.getId()) + 
                                " (Departs " + QString::fromStdString(trip.getDepart()) + ")");
            return false;
        }
    }
    
    lblConflict->setText("✓ No conflicts detected");
    lblConflict->setStyleSheet("color: green; font-weight: bold;");
    return true;
}

int TripDialog::timeToMinutes(const std::string &time) const {
    if (time.length() < 5) return 0;
    try {
        int h = std::stoi(time.substr(0, 2));
        int m = std::stoi(time.substr(3, 2));
        return h * 60 + m;
    } catch (...) {
        return 0;
    }
}

std::string TripDialog::minutesToTime(int minutes) const {
    int h = (minutes / 60) % 24;
    int m = minutes % 60;
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << h << ":" << std::setw(2) << m;
    return ss.str();
}

Trip TripDialog::getTrip() const {
    std::string routeId = cbRoute->currentData().toString().toStdString();
    std::string busId = cbBus->currentData().toString().toStdString();
    std::string driverId = cbDriver->currentData().toString().toStdString();
    
    QTime dep = timeDepart->time();
    std::stringstream ssDep;
    ssDep << std::setfill('0') << std::setw(2) << dep.hour() << ":" 
          << std::setw(2) << dep.minute();
    
    std::string arrStr = lblArrival->text().toStdString();
    
    return Trip(tripId, routeId, busId, driverId, ssDep.str(), arrStr);
}