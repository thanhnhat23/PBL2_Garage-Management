#include "UserWindow.h"
#include "../Class/FareCalculator.h"
#include "StyleHelper.h" 
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QLineEdit>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QSvgRenderer>
#include <QPainter>
#include <QPixmap>
#include <QFile>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <QButtonGroup>
#include <QDate>
#include <set>

UserWindow::UserWindow(QWidget *parent)
    : QWidget(parent)
{
    loadData();
    setupUI();
    populateTrips();
    populateMyTickets();
    
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &UserWindow::refreshData);
    refreshTimer->start(5000); 
}

UserWindow::~UserWindow() {
}

QIcon UserWindow::renderSvgIcon(const QString& resourcePath, const QSize& size, const QString& colorHex) {
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QIcon();
    }
    
    QString svgContent = QString::fromUtf8(file.readAll());
    file.close();
    
    svgContent.replace("currentColor", colorHex);
    
    QByteArray svgData = svgContent.toUtf8();
    QSvgRenderer renderer(svgData);
    
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    renderer.render(&painter);
    
    return QIcon(pixmap);
}

void UserWindow::setUser(User user) {
    currentUser = user;
}

void UserWindow::setupUI() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    QWidget *sidebarWidget = new QWidget(this);
    sidebarWidget->setFixedWidth(280);
    sidebarWidget->setStyleSheet("background: #1a1f2e;");
    
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebarWidget);
    sidebarLayout->setContentsMargins(0, 30, 0, 30);
    sidebarLayout->setSpacing(5);
    
    QLabel *lblLogo = new QLabel("USER PANEL", sidebarWidget);
    lblLogo->setStyleSheet("font-size: 20px; font-weight: 800; color: #f1f5f9; padding: 20px;");
    lblLogo->setAlignment(Qt::AlignCenter);
    sidebarLayout->addWidget(lblLogo);
    
    sidebarLayout->addSpacing(20);
    
    QStringList menuItems = {"Booking", "My Tickets"};
    QStringList menuIcons = {":/icons/icons/trip.svg", ":/icons/icons/ticket.svg"};
    
    for (int i = 0; i < menuItems.count(); i++) {
        QPushButton *btn = new QPushButton(menuItems[i], sidebarWidget);
        btn->setIcon(renderSvgIcon(menuIcons[i], QSize(24,24), "#cbd5e1"));
        btn->setMinimumHeight(45);
        btn->setMinimumWidth(150);
        btn->setStyleSheet("QPushButton { background: transparent; color: #cbd5e1; border: none; padding: 12px 20px; text-align: left; font-weight: 600; margin: 0 15px; } QPushButton:hover { background: rgba(59, 130, 246, 0.1); color: #f1f5f9; }");
        
        connect(btn, &QPushButton::clicked, [this, i]() {
            tabWidget->setCurrentIndex(i);
        });
        sidebarLayout->addWidget(btn);
    }
    
    sidebarLayout->addStretch();
    
    QPushButton *btnLogout = new QPushButton("Logout", sidebarWidget);
    btnLogout->setIcon(renderSvgIcon(":/icons/icons/logout.svg", QSize(20, 20), "#ef4444"));
    btnLogout->setStyleSheet("QPushButton { background: rgba(239, 68, 68, 0.15); color: #ef4444; border: 1px solid #7f1d1d; border-radius: 8px; padding: 12px 20px; font-weight: 600; margin: 0 15px 15px 15px; } QPushButton:hover { background: rgba(239, 68, 68, 0.25); }");
    connect(btnLogout, &QPushButton::clicked, this, &UserWindow::onLogoutClicked);
    
    sidebarLayout->addWidget(btnLogout);
    
    QWidget *contentWidget = new QWidget(this);
    contentWidget->setStyleSheet("background: #0f172a;");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(30, 30, 30, 30);
    contentLayout->setSpacing(20);

    QLabel *lblTitle = new QLabel("USER PANEL", contentWidget);
    lblTitle->setStyleSheet("font-size: 28px; font-weight: 800; color: #f1f5f9;");
    contentLayout->addWidget(lblTitle);
    
    tabWidget = new QTabWidget(contentWidget);
    tabWidget->tabBar()->hide();
    tabWidget->setStyleSheet("QTabWidget::pane { border: none; background: transparent; }");
    
    setupTabs();
    
    contentLayout->addWidget(tabWidget);
    
    mainLayout->addWidget(sidebarWidget);
    mainLayout->addWidget(contentWidget, 1);
    setLayout(mainLayout);
}

void UserWindow::setupTabs() {
    QWidget *tripsTab = new QWidget(this);
    QVBoxLayout *tLayout = new QVBoxLayout(tripsTab);
    tLayout->setSpacing(10);
    
    QHBoxLayout *filterLayout = new QHBoxLayout();
    QLabel *lblFilter = new QLabel("Select Route:", tripsTab);
    lblFilter->setStyleSheet("font-weight: bold; color: #e2e8f0; font-size: 14px;");
    
    QComboBox *cbRoute = new QComboBox(tripsTab);
    cbRoute->setObjectName("cbRouteFilter");
    cbRoute->setStyleSheet("QComboBox { padding: 8px; background: #1e293b; color: #e2e8f0; border: 1px solid #334155; border-radius: 4px; min-width: 250px; } QComboBox::drop-down { border: none; }");
    
    cbRoute->addItem("All Routes", "");
    for(const auto& r : routes) {
        cbRoute->addItem(QString::fromStdString(r.getName()), QString::fromStdString(r.getId()));
    }
    
    connect(cbRoute, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int){
        populateTrips();
    });
    
    filterLayout->addWidget(lblFilter);
    filterLayout->addWidget(cbRoute);
    filterLayout->addStretch();
    tLayout->addLayout(filterLayout);
    
    calendar = new QCalendarWidget(tripsTab);
    calendar->setMinimumDate(QDate::currentDate()); 
    calendar->setStyleSheet(
        "QCalendarWidget { background: #0f172a; color: #e2e8f0; border: 1px solid #1f2937; border-radius: 12px; selection-background-color: #2563eb; selection-color: #f8fafc; }"
        "QCalendarWidget QWidget#qt_calendar_navigationbar { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1f2937, stop:1 #0f172a); min-height: 46px; border-bottom: 1px solid #1f2937; }"
        "QCalendarWidget QToolButton { color: #e5e7eb; background: transparent; border: none; padding: 6px; font-weight: 600; }"
        "QCalendarWidget QAbstractItemView { background: #0b1220; alternate-background-color: #111827; selection-background-color: #2563eb; selection-color: #f8fafc; outline: 0; font-weight: 600; }"
        "QCalendarWidget QAbstractItemView::item { padding: 6px; margin: 2px; border-radius: 8px; }"
        "QCalendarWidget QAbstractItemView::item:selected { background: #2563eb; color: #f8fafc; }"
        "QCalendarWidget QAbstractItemView::item:hover { background: rgba(59,130,246,0.18); }"
        "QCalendarWidget QWidget { alternate-background-color: #0f172a; }"
    );
    tLayout->addWidget(calendar);
    
    tripTable = new QTableWidget(tripsTab);
    tripTable->setStyleSheet(StyleHelper::getTableStyle());
    tripTable->setColumnCount(5);
    tripTable->setHorizontalHeaderLabels({"ID", "Route", "Bus", "Departure Time", "Arrival Time"});
    tripTable->horizontalHeader()->setStretchLastSection(true);
    tripTable->verticalHeader()->setVisible(false);
    tripTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    tripTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tLayout->addWidget(tripTable);
    
    QPushButton *btnBook = new QPushButton("Book Tickets", tripsTab);
    btnBook->setIcon(renderSvgIcon(":/icons/icons/ticket-add.svg", QSize(16,16), "#22c55e"));
    btnBook->setStyleSheet(StyleHelper::getSuccessButtonStyle());
    connect(btnBook, &QPushButton::clicked, this, &UserWindow::onBookTicketClicked);
    
    QPushButton *btnViewSeat = new QPushButton("View Seat Map", tripsTab);
    btnViewSeat->setIcon(renderSvgIcon(":/icons/icons/seat-map.svg", QSize(16,16), "#3b82f6"));
    btnViewSeat->setStyleSheet(StyleHelper::getPrimaryButtonStyle());
    connect(btnViewSeat, &QPushButton::clicked, this, &UserWindow::onViewSeatMapClicked);
    
    QHBoxLayout *actions = new QHBoxLayout();
    actions->addWidget(btnBook);
    actions->addWidget(btnViewSeat);
    actions->addStretch();
    tLayout->addLayout(actions);
    tabWidget->addTab(tripsTab, "Trips");

    QWidget *myTab = new QWidget(this);
    QVBoxLayout *mLayout = new QVBoxLayout(myTab);
    myTicketsTable = new QTableWidget(myTab);
    myTicketsTable->setStyleSheet(StyleHelper::getTableStyle());
    myTicketsTable->setColumnCount(7);
    myTicketsTable->setHorizontalHeaderLabels({"ID","Trip","Seat","Passenger","Phone","Price","Time"});
    myTicketsTable->horizontalHeader()->setStretchLastSection(true);
    myTicketsTable->verticalHeader()->setVisible(false);
    myTicketsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    myTicketsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mLayout->addWidget(myTicketsTable);
    
    QPushButton *btnCancel = new QPushButton("Cancel Ticket", myTab);
    btnCancel->setIcon(renderSvgIcon(":/icons/icons/ticket-cancel.svg", QSize(16,16), "#ef4444"));
    btnCancel->setStyleSheet(StyleHelper::getDangerButtonStyle());
    connect(btnCancel, &QPushButton::clicked, this, &UserWindow::onCancelMyTicketClicked);

    QPushButton *btnExport = new QPushButton("Export CSV", myTab);
    btnExport->setIcon(renderSvgIcon(":/icons/icons/export.svg", QSize(16,16), "#22c55e"));
    btnExport->setIconSize(QSize(16,16));
    btnExport->setStyleSheet(StyleHelper::getSuccessButtonStyle());
    btnExport->setCursor(Qt::PointingHandCursor);
    connect(btnExport, &QPushButton::clicked, this, &UserWindow::onExportMyTicketsCsv);

    QHBoxLayout *btLayout = new QHBoxLayout();
    btLayout->addWidget(btnCancel);
    btLayout->addWidget(btnExport);
    btLayout->addStretch();
    mLayout->addLayout(btLayout);
    tabWidget->addTab(myTab, "My Tickets");
}

void UserWindow::loadData() {
    routes.clear(); buses.clear(); trips.clear(); tickets.clear();
    std::string line;
    
    std::ifstream routeFile("Data/Route.txt");
    while (std::getline(routeFile, line)) { if (!line.empty()) routes.push_back(Route::fromCSV(line)); }
    std::ifstream busFile("Data/Bus.txt");
    while (std::getline(busFile, line)) { if (!line.empty()) buses.push_back(Bus::fromCSV(line)); }
    std::ifstream tripFile("Data/Trip.txt");
    while (std::getline(tripFile, line)) { if (!line.empty()) trips.push_back(Trip::fromCSV(line)); }

    for (const auto& t : trips) {
        std::string path = "Data/Ticket/" + t.getId() + ".txt";
        std::ifstream tf(path);
        if (tf.is_open()) {
            while (std::getline(tf, line)) {
                if (!line.empty()) {
                    try { tickets.push_back(Ticket::fromCSV(line)); } catch(...) {}
                }
            }
            tf.close();
        }
    }
}

void UserWindow::populateTrips(QString routeId) {
    tripTable->setRowCount(0);
    
    if (routeId.isEmpty()) {
        QComboBox *cb = findChild<QComboBox*>("cbRouteFilter");
        if (cb) routeId = cb->currentData().toString();
    }
    
    std::map<std::string, Route> routeMap; for (auto &r: routes) routeMap[r.getId()]=r;
    std::map<std::string, Bus> busMap; for (auto &b: buses) busMap[b.getId()]=b;
    
    for (size_t i=0;i<trips.size();++i){ 
        const Trip &tr = trips[i];
        
        if (!routeId.isEmpty() && tr.getRouteId() != routeId.toStdString()) {
            continue;
        }
        
        int row = tripTable->rowCount(); tripTable->insertRow(row);
        tripTable->setItem(row,0,new QTableWidgetItem(QString::fromStdString(tr.getId())));
        
        QString rName = routeMap.count(tr.getRouteId()) ? QString::fromStdString(routeMap[tr.getRouteId()].getName()) : "";
        tripTable->setItem(row,1,new QTableWidgetItem(rName));
        
        QString bName = busMap.count(tr.getBusId()) ? QString::fromStdString(busMap[tr.getBusId()].getName()) : "";
        tripTable->setItem(row,2,new QTableWidgetItem(bName));
        
        tripTable->setItem(row,3,new QTableWidgetItem(QString::fromStdString(tr.getDepart())));
        tripTable->setItem(row,4,new QTableWidgetItem(QString::fromStdString(tr.getArrival())));
    }
}

void UserWindow::populateMyTickets() {
    myTicketsTable->setRowCount(0);
    std::string userName = currentUser.getUsername();
    std::string userPhone = currentUser.getPhoneNumber();
    if (userName.empty()) return;
    
    // Build quick lookups
    std::map<std::string, Trip> tripMap; for (const auto &tr : trips) tripMap[tr.getId()] = tr;
    std::map<std::string, Route> routeMap; for (const auto &rt : routes) routeMap[rt.getId()] = rt;
    auto formatVnd = [](unsigned long amount){ QString s = QString::number(amount); for (int i=s.length()-3;i>0;i-=3) s.insert(i, '.'); return s + "vnd"; };

    for (size_t idx = 0; idx < tickets.size(); idx++){ 
        const auto &tk = tickets[idx];
        bool matchName = (tk.getPassengerName() == userName);
        bool matchPhone = (!userPhone.empty() && tk.getPhoneNumber() == userPhone);

        if (matchName || matchPhone) {
            int row = myTicketsTable->rowCount(); 
            myTicketsTable->insertRow(row);
            myTicketsTable->setItem(row,0,new QTableWidgetItem(QString::fromStdString(tk.getId())));
            // Trip: show Start - End
            QString tripDisplay = QString::fromStdString(tk.getTripId());
            auto itTr = tripMap.find(tk.getTripId());
            if (itTr != tripMap.end()) {
                auto itRt = routeMap.find(itTr->second.getRouteId());
                if (itRt != routeMap.end()) {
                    tripDisplay = QString::fromStdString(itRt->second.getStart() + " - " + itRt->second.getEnd());
                }
            }
            myTicketsTable->setItem(row,1,new QTableWidgetItem(tripDisplay));
            myTicketsTable->setItem(row,2,new QTableWidgetItem(QString::number(tk.getSeatNo())));
            myTicketsTable->setItem(row,3,new QTableWidgetItem(QString::fromStdString(tk.getPassengerName())));
            myTicketsTable->setItem(row,4,new QTableWidgetItem(QString::fromStdString(tk.getPhoneNumber())));
            myTicketsTable->setItem(row,5,new QTableWidgetItem(formatVnd(tk.getPrice())));
            myTicketsTable->setItem(row,6,new QTableWidgetItem(QString::fromStdString(tk.getBookedAt())));
        }
    }
}

void UserWindow::onBookTicketClicked() {
    std::string userName = currentUser.getUsername();
    std::string userPhone = currentUser.getPhoneNumber(); 

    if (userName.empty()) { QMessageBox::warning(this, "Book Ticket", "Error: User not logged in"); return; }
    
    auto items = tripTable->selectedItems();
    if (items.isEmpty()) { QMessageBox::warning(this, "Book Ticket", "Select a trip to book"); return; }
    int row = items.first()->row();
    QString tripId = tripTable->item(row,0)->text();

    std::string busId; unsigned long tripPrice = 0;
    for (const auto &tr : trips){ if (tr.getId()==tripId.toStdString()) { busId = tr.getBusId(); break; } }
    if (busId.empty()) { QMessageBox::warning(this, "Book Ticket", "Error: Bus not found"); return; }
    
    int capacity = 40; for (const auto &b : buses){ if (b.getId()==busId) { capacity = b.getCapacity(); break; } }
    
    for (const auto &r : routes){
         for (const auto &tr : trips){ 
            if (tr.getId()==tripId.toStdString() && tr.getRouteId()==r.getId()){ 
                try{ tripPrice = FareCalculator::calculate(std::stol(r.getDistance())); } 
                catch(...){ tripPrice = FareCalculator::MIN_FARE; } 
                break; 
            } 
        } 
    }
    
    QString dateStr = calendar->selectedDate().toString("yyyy-MM-dd");
    std::set<int> booked;
    for (const auto &t: tickets){ if (t.getBusId()==busId){ std::string bAt = t.getBookedAt(); if (bAt.rfind(dateStr.toStdString(),0)==0){ booked.insert(t.getSeatNo()); } } }
    
    QDialog dlg(this);
    dlg.setWindowTitle("Book Ticket");
    dlg.setStyleSheet("background: #0f172a; color: #e5e7eb;");
    dlg.setMinimumWidth(400);
    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    
    layout->addWidget(new QLabel(QString("Date: %1").arg(dateStr), &dlg));
    
    QLineEdit *nameEdit = new QLineEdit(&dlg);
    nameEdit->setPlaceholderText("Passenger Name");
    nameEdit->setStyleSheet(StyleHelper::getInputStyle());
    layout->addWidget(nameEdit);

    QLineEdit *phoneEdit = new QLineEdit(&dlg);
    phoneEdit->setPlaceholderText("Phone Number");
    phoneEdit->setStyleSheet(StyleHelper::getInputStyle());
    phoneEdit->setText(QString::fromStdString(userPhone)); 
    layout->addWidget(phoneEdit);
    
    QComboBox *payment = new QComboBox(&dlg);
    payment->setStyleSheet(StyleHelper::getInputStyle());
    payment->addItems({"Cash","Momo","ZaloPay","Bank Transfer"});
    layout->addWidget(payment);
    
    QWidget *seatGrid = new QWidget(&dlg);
    QGridLayout *grid = new QGridLayout(seatGrid);
    QButtonGroup *seatGroup = new QButtonGroup(&dlg);
    seatGroup->setExclusive(false); 
    int cols = 4;
    std::set<int> selectedSeats;
    
    QLabel *pricePreview = new QLabel(QString("Total: 0 VND"), &dlg);
    pricePreview->setStyleSheet("font-size: 16px; font-weight: bold; color: #22c55e; padding: 10px;");
    pricePreview->setAlignment(Qt::AlignCenter);
    
    for (int i=1;i<=capacity;i++){
        QPushButton *btn = new QPushButton(QString::number(i), seatGrid);
        btn->setCheckable(true);
        btn->setFixedSize(45, 45);
        bool isBooked = booked.count(i)>0;
        if (isBooked) {
            btn->setEnabled(false);
            btn->setStyleSheet("background: #ef4444; color: white; border: none; border-radius: 4px;");
        } else {
            btn->setStyleSheet("QPushButton { background: #334155; color: white; border: none; border-radius: 4px; } QPushButton:checked { background: #22c55e; }");
            seatGroup->addButton(btn, i);
            connect(btn, &QPushButton::toggled, [&, i, pricePreview, tripPrice](bool checked) {
                if (checked) { selectedSeats.insert(i); } else { selectedSeats.erase(i); }
                unsigned long totalPrice = selectedSeats.size() * tripPrice;
                pricePreview->setText(QString("Total: %1 VND").arg(totalPrice));
            });
        }
        grid->addWidget(btn, (i-1)/cols, (i-1)%cols);
    }
    layout->addWidget(seatGrid);
    layout->addWidget(pricePreview);
    
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, [&](){ dlg.accept(); });
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    
    if (dlg.exec() != QDialog::Accepted) return;
    
    QString inputName = nameEdit->text().trimmed();
    QString inputPhone = phoneEdit->text().trimmed();
    QString pay = payment->currentText();
    
    if (inputName.isEmpty()) { QMessageBox::warning(this,"Book Ticket","Please enter passenger name"); return; }
    if (inputPhone.isEmpty()){ QMessageBox::warning(this,"Book Ticket","Please enter phone number"); return; }
    if (selectedSeats.empty()){ QMessageBox::warning(this,"Book Ticket","Please select at least one seat"); return; }
    
    // Determine file based on trip ID: T005 -> T005.txt (NOT TK005.txt!)
    std::string fileId = tripId.toStdString();
    std::string path = std::string("Data/Ticket/") + fileId + ".txt";
    
    // Find max ticket ID in THIS file only
    int maxTkNum = 0;
    {
        std::ifstream in(path);
        std::string ln;
        while (std::getline(in, ln)) {
            if (ln.empty()) continue;
            try {
                Ticket t = Ticket::fromCSV(ln);
                std::string id = t.getId();
                if (id.length() > 2 && id.substr(0, 2) == "TK") {
                    int num = std::stoi(id.substr(2));
                    if (num > maxTkNum) maxTkNum = num;
                }
            } catch(...) {}
        }
    }

    std::string nameStr = inputName.toStdString();
    std::string phoneStr = inputPhone.toStdString();
    std::string payStr = pay.toStdString();
    std::string bookedAtStr = (dateStr + " 00:00").toStdString();
    
    try {

        // Append all selected seats as separate tickets
        {
            std::fstream out(path, std::ios::in | std::ios::out | std::ios::app);
            if (!out.is_open()) { QMessageBox::critical(this, "Book ticket", "Cannot save ticket"); return; }
            out.seekg(0, std::ios::end);
            if (out.tellg() > 0) { out.seekg(-1, std::ios::end); char last; out.get(last); if (last != '\n') out << '\n'; }
            
            // Create and save tickets for each selected seat
            for (int seatNo : selectedSeats) {
                maxTkNum++;
                std::string newId = "TK" + std::string(3 - std::to_string(maxTkNum).length(), '0') + std::to_string(maxTkNum);
                Ticket tk(newId, tripId.toStdString(), busId, seatNo, nameStr, phoneStr, tripPrice, bookedAtStr, payStr);
                out << tk.toCSV() << "\n";

                // Add to memory
                tickets.push_back(tk);
                
                // Add row to table
                int row = myTicketsTable->rowCount();
                myTicketsTable->insertRow(row);
                myTicketsTable->setItem(row,0,new QTableWidgetItem(QString::fromStdString(tk.getId())));
                // Format Trip as Start - End
                QString tripDisplay = QString::fromStdString(tk.getTripId());
                for (const auto &tr : trips) {
                    if (tr.getId() == tk.getTripId()) {
                        for (const auto &rt : routes) {
                            if (rt.getId() == tr.getRouteId()) {
                                tripDisplay = QString::fromStdString(rt.getStart() + " - " + rt.getEnd());
                                break;
                            }
                        }
                        break;
                    }
                }
                myTicketsTable->setItem(row,1,new QTableWidgetItem(tripDisplay));
                myTicketsTable->setItem(row,2,new QTableWidgetItem(QString::number(tk.getSeatNo())));
                myTicketsTable->setItem(row,3,new QTableWidgetItem(QString::fromStdString(tk.getPassengerName())));
                myTicketsTable->setItem(row,4,new QTableWidgetItem(QString::fromStdString(tk.getPhoneNumber())));
                {
                    QString s = QString::number(tk.getPrice());
                    for (int i=s.length()-3;i>0;i-=3) s.insert(i, '.');
                    s += "vnd";
                    myTicketsTable->setItem(row,5,new QTableWidgetItem(s));
                }
                myTicketsTable->setItem(row,6,new QTableWidgetItem(QString::fromStdString(tk.getBookedAt())));
            }
            out.close();
        }
        unsigned long totalPrice = selectedSeats.size() * tripPrice;
        QMessageBox::information(this, "Book ticket", QString("Booked %1 tickets successfully! Total: %2 VND").arg(selectedSeats.size()).arg(totalPrice));
    } catch (...) {
        QMessageBox::critical(this, "Error", "Undefined error when saving ticket");
    }
}

void UserWindow::onViewSeatMapClicked() {
    auto items = tripTable->selectedItems();
    if (items.isEmpty()) { QMessageBox::warning(this, "Seat Map", "Select a trip"); return; }
    int row = items.first()->row();
    QString tripId = tripTable->item(row,0)->text();
    std::string busId; 
    for(const auto &tr : trips) if(tr.getId()==tripId.toStdString()) { busId = tr.getBusId(); break; }
    
    QString dateStr = calendar->selectedDate().toString("yyyy-MM-dd");
    std::set<int> booked;
    for(const auto &tk : tickets) {
        if(tk.getBusId()==busId && QString::fromStdString(tk.getBookedAt()).startsWith(dateStr)) 
            booked.insert(tk.getSeatNo());
    }
    
    QDialog dlg(this);
    dlg.setWindowTitle("Seat Map - " + dateStr);
    dlg.setStyleSheet("background: #0f172a; color: #e5e7eb;");
    QVBoxLayout *lay = new QVBoxLayout(&dlg);
    
    QTableWidget *tw = new QTableWidget(&dlg);
    tw->setColumnCount(3);
    tw->setHorizontalHeaderLabels({"Seat","Status","Info"});
    tw->setStyleSheet(StyleHelper::getTableStyle());
    lay->addWidget(tw);
    
    int capacity = 0;
    for(const auto &b : buses) if(b.getId()==busId) { capacity = b.getCapacity(); break; }
    
    tw->setRowCount(capacity);
    for(int i=1; i<=capacity; i++) {
        tw->setItem(i-1, 0, new QTableWidgetItem(QString::number(i)));
        bool isBooked = booked.count(i);
        QTableWidgetItem *st = new QTableWidgetItem(isBooked ? "Booked" : "Available");
        st->setForeground(isBooked ? Qt::red : Qt::green);
        tw->setItem(i-1, 1, st);
    }
    
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    lay->addWidget(box);
    
    dlg.exec();
}

void UserWindow::onCancelMyTicketClicked() {
    auto items = myTicketsTable->selectedItems();
    if (items.isEmpty()) return;
    int row = items.first()->row();
    
    // Get phone from table directly (column 4 is phone)
    std::string selectedPhone = myTicketsTable->item(row, 4)->text().toStdString();
    std::string userPhone = currentUser.getPhoneNumber();
    
    // Trim both for comparison
    auto trim = [](const std::string& s) {
        auto start = s.find_first_not_of(" \t\r\n");
        auto end = s.find_last_not_of(" \t\r\n");
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    };
    
    selectedPhone = trim(selectedPhone);
    userPhone = trim(userPhone);
    
    // Check ownership
    if(selectedPhone != userPhone) {
        QMessageBox::warning(this, "Error", 
            QString("This ticket doesn't belong to you!\nTicket Phone: %1\nYour Phone: %2")
                .arg(QString::fromStdString(selectedPhone))
                .arg(QString::fromStdString(userPhone)));
        return;
    }
    
    std::string id = myTicketsTable->item(row, 0)->text().toStdString();
    std::string tripId = myTicketsTable->item(row, 1)->text().toStdString();
    
    // Debug: Find actual trip ID from trips list (display shows "Start - End", not trip ID)
    std::string actualTripId = "";
    for (const auto& tr : trips) {
        for (const auto& rt : routes) {
            if (rt.getId() == tr.getRouteId()) {
                std::string display = rt.getStart() + " - " + rt.getEnd();
                if (display == tripId) {
                    actualTripId = tr.getId();
                    break;
                }
            }
        }
        if (!actualTripId.empty()) break;
    }
    
    // If we couldn't find trip by display, search by ticket ID in memory
    if (actualTripId.empty()) {
        auto it = std::find_if(tickets.begin(), tickets.end(), [&](const Ticket& t){ return t.getId() == id; });
        if (it == tickets.end()) {
            QMessageBox::warning(this, "Error", "Ticket not found!");
            return;
        }
        actualTripId = it->getTripId();
        tickets.erase(it);
    } else {
        // Also remove from memory
        auto it = std::find_if(tickets.begin(), tickets.end(), [&](const Ticket& t){ return t.getId() == id; });
        if (it != tickets.end()) {
            tickets.erase(it);
        }
    }
    
    // Delete from file
    if(actualTripId.length() > 0) {
        std::string path = "Data/Ticket/" + actualTripId + ".txt";
        
        // Check if file exists
        std::ifstream test(path);
        if (!test.good()) {
            test.close();
            QMessageBox::warning(this, "Error", QString("File not found: %1").arg(QString::fromStdString(path)));
            populateMyTickets();
            return;
        }
        test.close();
        
        // Read and filter out the ticket
        std::vector<std::string> lines;
        std::ifstream in(path); 
        std::string ln;
        bool found = false;
        while(std::getline(in, ln)) {
            if(ln.empty()) continue;
            try {
                Ticket t = Ticket::fromCSV(ln);
                if(t.getId() == id) {
                    found = true;
                    continue; // Skip this line (delete it)
                }
                lines.push_back(ln);
            } catch(...) { 
                lines.push_back(ln); 
            }
        }
        in.close();
        
        // Write back to file
        std::ofstream out(path, std::ios::trunc);
        for(const auto &l : lines) out << l << "\n";
        out.close();
        
        if (!found) {
            QMessageBox::warning(this, "Warning", "Ticket ID not found in file!");
        }
    }
    
    populateMyTickets();
    QMessageBox::information(this, "Success", "Ticket cancelled successfully!");
}

void UserWindow::onLogoutClicked() {
    emit logout();
}

void UserWindow::onExportMyTicketsCsv() {
    if (!myTicketsTable) return;
    QDir().mkpath("export");
    QString user = QString::fromStdString(currentUser.getUsername());
    if (user.isEmpty()) user = "guest";
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString path = QString("export/user_tickets_%1_%2.csv").arg(user, ts);

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Export CSV", "Cannot write export file.");
        return;
    }
    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);

    // Header
    QStringList headers;
    for (int c=0;c<myTicketsTable->columnCount();++c) headers << myTicketsTable->horizontalHeaderItem(c)->text();
    out << headers.join(',') << "\n";

    // Rows
    for (int r=0;r<myTicketsTable->rowCount();++r){
        QStringList cols;
        for (int c=0;c<myTicketsTable->columnCount();++c){
            QTableWidgetItem *it = myTicketsTable->item(r,c);
            QString val = it ? it->text() : QString();
            if (val.contains('"')) val.replace('"', """");
            if (val.contains(',') || val.contains('"')) val = '"' + val + '"';
            cols << val;
        }
        out << cols.join(',') << "\n";
    }
    f.close();
    QMessageBox::information(this, "Export CSV", QString("Exported %1 rows to %2").arg(myTicketsTable->rowCount()).arg(path));
}

void UserWindow::refreshData() {
    loadData();
    populateTrips();
    populateMyTickets();
}

void UserWindow::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    refreshData();
}