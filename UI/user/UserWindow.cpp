#include "UserWindow.h"
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

UserWindow::UserWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupTabs();
    loadData();
    populateTrips();
    populateMyTickets();
}

UserWindow::~UserWindow() {
}

QIcon UserWindow::renderSvgIcon(const QString& resourcePath, const QSize& size, const QString& colorHex) {
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open SVG resource:" << resourcePath;
        return QIcon();
    }
    
    QString svgContent = QString::fromUtf8(file.readAll());
    file.close();
    
    svgContent.replace("currentColor", colorHex);
    
    QByteArray svgData = svgContent.toUtf8();
    QSvgRenderer renderer(svgData);
    
    if (!renderer.isValid()) {
        qWarning() << "Invalid SVG content for:" << resourcePath;
        return QIcon();
    }
    
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
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
    
    // ===== SIDEBAR (Left) =====
    QWidget *sidebarWidget = new QWidget(this);
    sidebarWidget->setFixedWidth(280);
    sidebarWidget->setStyleSheet(
        "QWidget {"
        "  background: #1a1f2e;"
        "}"
    );
    
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebarWidget);
    sidebarLayout->setContentsMargins(0, 30, 0, 30);
    sidebarLayout->setSpacing(5);
    
    // Logo/Title
    QLabel *lblLogo = new QLabel("USER PANEL", sidebarWidget);
    lblLogo->setStyleSheet(
        "font-size: 20px;"
        "font-weight: 800;"
        "color: #f1f5f9;"
        "padding: 20px;"
    );
    lblLogo->setAlignment(Qt::AlignCenter);
    sidebarLayout->addWidget(lblLogo);
    
    sidebarLayout->addSpacing(20);
    
    // Menu buttons
    QStringList menuItems = {"Chuyến", "Vé của tôi"};
    QStringList menuIcons = {":/icons/icons/trip.svg", ":/icons/icons/ticket.svg"};
    
    QString activeButtonStyle = 
        "QPushButton {"
        "  background: #2563eb;"
        "  color: #f1f5f9;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 12px 20px;"
        "  text-align: left;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "  margin: 0 15px;"
        "}"
        "QPushButton:hover {"
        "  background: #1d4ed8;"
        "}";
    
    QString inactiveButtonStyle = 
        "QPushButton {"
        "  background: transparent;"
        "  color: #cbd5e1;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 12px 20px;"
        "  text-align: left;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "  margin: 0 15px;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(59, 130, 246, 0.1);"
        "  color: #f1f5f9;"
        "}";
    
    for (int i = 0; i < menuItems.count(); i++) {
        QPushButton *btn = new QPushButton(menuItems[i], sidebarWidget);
        btn->setIcon(renderSvgIcon(menuIcons[i], QSize(24,24), QString("#cbd5e1")));
        btn->setIconSize(QSize(24, 24));
        btn->setMinimumHeight(45);
        btn->setMinimumWidth(150);
        btn->setFlat(true);
        btn->setStyleSheet(i == 0 ? activeButtonStyle : inactiveButtonStyle);
        btn->setCursor(Qt::PointingHandCursor);
        
        if (i == 0) {
            activeButton = btn;
        }
        
        connect(btn, &QPushButton::clicked, [this, btn, activeButtonStyle, inactiveButtonStyle, i]() {
            if (activeButton) {
                activeButton->setStyleSheet(inactiveButtonStyle);
            }
            btn->setStyleSheet(activeButtonStyle);
            activeButton = btn;
            tabWidget->setCurrentIndex(i);
        });
        sidebarLayout->addWidget(btn);
    }
    
    sidebarLayout->addSpacing(40);
    
    // Logout button
    QPushButton *btnLogout = new QPushButton("Logout", sidebarWidget);
    btnLogout->setIcon(renderSvgIcon(":/icons/icons/logout.svg", QSize(20, 20), "#ef4444"));
    btnLogout->setIconSize(QSize(20, 20));
    btnLogout->setStyleSheet(
        "QPushButton {"
        "  background: rgba(239, 68, 68, 0.15);"
        "  color: #ef4444;"
        "  border: 1px solid #7f1d1d;"
        "  border-radius: 8px;"
        "  padding: 12px 20px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "  margin: 0 15px 15px 15px;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(239, 68, 68, 0.25);"
        "}"
    );
    btnLogout->setMinimumHeight(40);
    btnLogout->setCursor(Qt::PointingHandCursor);
    connect(btnLogout, &QPushButton::clicked, this, &UserWindow::onLogoutClicked);
    
    sidebarLayout->addStretch();
    sidebarLayout->addWidget(btnLogout);
    
    // ===== MAIN CONTENT (Right) =====
    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(30, 30, 30, 30);
    contentLayout->setSpacing(20);
    
    // Header
    QLabel *lblTitle = new QLabel("USER PANEL", contentWidget);
    lblTitle->setStyleSheet(
        "font-size: 28px;"
        "font-weight: 800;"
        "color: #f1f5f9;"
    );
    contentLayout->addWidget(lblTitle);
    
    // Tab Widget (hidden tab bar)
    tabWidget = new QTabWidget(contentWidget);
    tabWidget->setStyleSheet(
        "QTabWidget::pane {"
        "  border: none;"
        "  background: transparent;"
        "}"
        "QTabBar::tab { height: 0px; width: 0px; }"
    );
    contentLayout->addWidget(tabWidget, 1);
    
    contentWidget->setStyleSheet("background: #0f172a;");
    
    // Add both to main layout
    mainLayout->addWidget(sidebarWidget);
    mainLayout->addWidget(contentWidget, 1);
    
    setLayout(mainLayout);
    setStyleSheet("background: #0f172a;");
}

void UserWindow::setupTabs() {
    // Trips tab with calendar and list
    QWidget *tripsTab = new QWidget(this);
    QVBoxLayout *tLayout = new QVBoxLayout(tripsTab);
    tLayout->setSpacing(10);
    calendar = new QCalendarWidget(tripsTab);
    calendar->setStyleSheet(
        "QCalendarWidget {"
        "  background: #0f172a;"
        "  color: #e2e8f0;"
        "  border: 1px solid #1f2937;"
        "  border-radius: 12px;"
        "  selection-background-color: #2563eb;"
        "  selection-color: #f8fafc;"
        "}"
        "QCalendarWidget QWidget#qt_calendar_navigationbar {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1f2937, stop:1 #0f172a);"
        "  min-height: 46px;"
        "  border-bottom: 1px solid #1f2937;"
        "}"
        "QCalendarWidget QToolButton {"
        "  color: #e5e7eb;"
        "  background: transparent;"
        "  border: none;"
        "  padding: 6px 12px;"
        "  font-weight: 600;"
        "  min-width: 20px;"
        "}"
        "QCalendarWidget QToolButton:hover { color: #3b82f6; }"
        "QCalendarWidget QAbstractItemView {"
        "  background: #0b1220;"
        "  alternate-background-color: #111827;"
        "  selection-background-color: #2563eb;"
        "  selection-color: #f8fafc;"
        "  outline: 0;"
        "  font-weight: 600;"
        "}"
        "QCalendarWidget QAbstractItemView::item {"
        "  padding: 6px;"
        "  margin: 2px;"
        "  border-radius: 8px;"
        "}"
        "QCalendarWidget QAbstractItemView::item:selected {"
        "  background: #2563eb;"
        "  color: #f8fafc;"
        "}"
        "QCalendarWidget QAbstractItemView::item:hover {"
        "  background: rgba(59,130,246,0.18);"
        "}"
        "QCalendarWidget QSpinBox {"
        "  background: #111827;"
        "  color: #e5e7eb;"
        "  border: 1px solid #1f2937;"
        "  border-radius: 6px;"
        "  padding: 2px 6px;"
        "}"
    );
    tLayout->addWidget(calendar);
    tripTable = new QTableWidget(tripsTab);
    tripTable->setStyleSheet(StyleHelper::getTableStyle());
    tripTable->setColumnCount(3);
    tripTable->setHorizontalHeaderLabels({"ID","Tuyến","Xe"});
    tripTable->horizontalHeader()->setStretchLastSection(true);
    tripTable->verticalHeader()->setVisible(false);
    tripTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    tLayout->addWidget(tripTable);
    QPushButton *btnBook = new QPushButton("Đặt vé", tripsTab);
    btnBook->setIcon(renderSvgIcon(":/icons/icons/ticket-add.svg", QSize(16,16), "#22c55e"));
    btnBook->setIconSize(QSize(16,16));
    btnBook->setStyleSheet(StyleHelper::getSuccessButtonStyle());
    btnBook->setCursor(Qt::PointingHandCursor);
    connect(btnBook, &QPushButton::clicked, this, &UserWindow::onBookTicketClicked);
    QPushButton *btnViewSeat = new QPushButton("Xem sơ đồ ghế", tripsTab);
    btnViewSeat->setIcon(renderSvgIcon(":/icons/icons/seat-map.svg", QSize(16,16), "#3b82f6"));
    btnViewSeat->setIconSize(QSize(16,16));
    btnViewSeat->setStyleSheet(StyleHelper::getPrimaryButtonStyle());
    btnViewSeat->setCursor(Qt::PointingHandCursor);
    connect(btnViewSeat, &QPushButton::clicked, this, &UserWindow::onViewSeatMapClicked);
    QHBoxLayout *actions = new QHBoxLayout();
    actions->addWidget(btnBook);
    actions->addWidget(btnViewSeat);
    actions->addStretch();
    tLayout->addLayout(actions);
    tabWidget->addTab(tripsTab, "Chuyến");

    // My Tickets tab
    QWidget *myTab = new QWidget(this);
    QVBoxLayout *mLayout = new QVBoxLayout(myTab);
    myTicketsTable = new QTableWidget(myTab);
    myTicketsTable->setStyleSheet(StyleHelper::getTableStyle());
    myTicketsTable->setColumnCount(7);
    myTicketsTable->setHorizontalHeaderLabels({"ID","Chuyến","Ghế","Khách","SĐT","Giá","Thời gian"});
    myTicketsTable->horizontalHeader()->setStretchLastSection(true);
    myTicketsTable->verticalHeader()->setVisible(false);
    myTicketsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mLayout->addWidget(myTicketsTable);
    QPushButton *btnCancel = new QPushButton("Hủy vé", myTab);
    btnCancel->setIcon(renderSvgIcon(":/icons/icons/ticket-cancel.svg", QSize(16,16), "#ef4444"));
    btnCancel->setIconSize(QSize(16,16));
    btnCancel->setStyleSheet(StyleHelper::getDangerButtonStyle());
    btnCancel->setCursor(Qt::PointingHandCursor);
    connect(btnCancel, &QPushButton::clicked, this, &UserWindow::onCancelMyTicketClicked);
    mLayout->addWidget(btnCancel, 0, Qt::AlignLeft);
    tabWidget->addTab(myTab, "Vé của tôi");
}

void UserWindow::loadData() {
    // Minimal data load for user
    routes.clear(); buses.clear(); trips.clear(); tickets.clear();
    std::string line;
    std::ifstream routeFile("Data/Route.txt");
    while (std::getline(routeFile, line)) { if (!line.empty()) routes.push_back(Route::fromCSV(line)); }
    std::ifstream busFile("Data/Bus.txt");
    while (std::getline(busFile, line)) { if (!line.empty()) buses.push_back(Bus::fromCSV(line)); }
    std::ifstream tripFile("Data/Trip.txt");
    while (std::getline(tripFile, line)) { if (!line.empty()) trips.push_back(Trip::fromCSV(line)); }
    for (int i=1;i<=100;i++){ std::string fid = "TK" + std::string(3-std::to_string(i).length(),'0') + std::to_string(i);
        std::ifstream tf("Data/Ticket/"+fid+".txt"); if (!tf.is_open()) continue; while (std::getline(tf,line)){
            if (!line.empty()) { try { tickets.push_back(Ticket::fromCSV(line)); } catch(...){} }
        }}
}

void UserWindow::populateTrips() {
    tripTable->setRowCount(0);
    std::map<std::string, Route> routeMap; for (auto &r: routes) routeMap[r.getId()]=r;
    std::map<std::string, Bus> busMap; for (auto &b: buses) busMap[b.getId()]=b;
    for (size_t i=0;i<trips.size();++i){ const Trip &tr = trips[i];
        int row = tripTable->rowCount(); tripTable->insertRow(row);
        tripTable->setItem(row,0,new QTableWidgetItem(QString::fromStdString(tr.getId())));
        auto rt = routeMap[tr.getRouteId()];
        tripTable->setItem(row,1,new QTableWidgetItem(QString::fromStdString(rt.getName())));
        auto bs = busMap[tr.getBusId()];
        tripTable->setItem(row,2,new QTableWidgetItem(QString::fromStdString(bs.getName())));
    }
}

void UserWindow::populateMyTickets() {
    std::cout << "DEBUG: populateMyTickets() called" << std::endl;
    myTicketsTable->setRowCount(0);
    std::cout << "DEBUG: Row count cleared" << std::endl;
    
    // Filter tickets by current user (name or phone match)
    std::string userName = currentUser.getUsername();
    std::cout << "DEBUG: Current user name: " << userName << std::endl;
    if (userName.empty()) {
        std::cout << "DEBUG: User name is empty, returning" << std::endl;
        return; // User not logged in
    }
    
    std::cout << "DEBUG: Total tickets to process: " << tickets.size() << std::endl;
    for (size_t idx = 0; idx < tickets.size(); idx++){ 
        const auto &tk = tickets[idx];
        std::cout << "DEBUG: Checking ticket " << idx << ": " << tk.getId() << " passenger: " << tk.getPassengerName() << std::endl;
        if (tk.getPassengerName()==userName){
            std::cout << "DEBUG: Ticket matches current user, adding to table" << std::endl;
            int row = myTicketsTable->rowCount(); 
            myTicketsTable->insertRow(row);
            myTicketsTable->setItem(row,0,new QTableWidgetItem(QString::fromStdString(tk.getId())));
            myTicketsTable->setItem(row,1,new QTableWidgetItem(QString::fromStdString(tk.getTripId())));
            myTicketsTable->setItem(row,2,new QTableWidgetItem(QString::number(tk.getSeatNo())));
            myTicketsTable->setItem(row,3,new QTableWidgetItem(QString::fromStdString(tk.getPassengerName())));
            myTicketsTable->setItem(row,4,new QTableWidgetItem(QString::fromStdString(tk.getPhoneNumber())));
            myTicketsTable->setItem(row,5,new QTableWidgetItem(QString::number(tk.getPrice())));
            myTicketsTable->setItem(row,6,new QTableWidgetItem(QString::fromStdString(tk.getBookedAt())));
            std::cout << "DEBUG: Row " << row << " added successfully" << std::endl;
        }
    }
    std::cout << "DEBUG: populateMyTickets() finished" << std::endl;
}

void UserWindow::onBookTicketClicked() {
    // Validate current user
    std::string userName = currentUser.getUsername();
    if (userName.empty()) { QMessageBox::warning(this, "Đặt vé", "Lỗi: Người dùng chưa đăng nhập"); return; }
    
    // Use selected trip
    auto items = tripTable->selectedItems();
    if (items.isEmpty()) { QMessageBox::warning(this, "Đặt vé", "Chọn một chuyến để đặt vé"); return; }
    int row = items.first()->row();
    QString tripId = tripTable->item(row,0)->text();

    // Find trip and busId, get price
    std::string busId; unsigned long tripPrice = 0;
    for (const auto &tr : trips){ if (tr.getId()==tripId.toStdString()) { busId = tr.getBusId(); break; } }
    if (busId.empty()) { QMessageBox::warning(this, "Đặt vé", "Lỗi: Không tìm thấy xe"); return; }
    int capacity = 40; for (const auto &b : buses){ if (b.getId()==busId) { capacity = b.getCapacity(); break; } }
    for (const auto &r : routes){ for (const auto &tr : trips){ if (tr.getId()==tripId.toStdString() && tr.getRouteId()==r.getId()){ try { tripPrice = std::stol(r.getDistance()) * 1000; } catch(...) { tripPrice = 100000; } break; } } }
    
    QString dateStr = calendar->selectedDate().toString("yyyy-MM-dd");
    std::set<int> booked;
    for (const auto &t: tickets){ if (t.getBusId()==busId){ std::string bAt = t.getBookedAt(); if (bAt.rfind(dateStr.toStdString(),0)==0){ booked.insert(t.getSeatNo()); } } }
    
    // Create dialog with all inputs
    QDialog dlg(this);
    dlg.setWindowTitle("Đặt vé");
    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    
    QLabel *lblDate = new QLabel(QString("Ngày: %1").arg(dateStr), &dlg);
    layout->addWidget(lblDate);
    
    QLineEdit *phoneEdit = new QLineEdit(&dlg);
    phoneEdit->setPlaceholderText("Số điện thoại");
    phoneEdit->setStyleSheet(StyleHelper::getInputStyle());
    layout->addWidget(phoneEdit);
    
    QComboBox *payment = new QComboBox(&dlg);
    payment->addItems({"Tien mat","Momo","ZaloPay","VNPay"});
    layout->addWidget(payment);
    
    // Seat selection grid
    QWidget *seatGrid = new QWidget(&dlg);
    QGridLayout *grid = new QGridLayout(seatGrid);
    grid->setSpacing(6);
    QButtonGroup *seatGroup = new QButtonGroup(&dlg);
    seatGroup->setExclusive(true);
    int cols = 4;
    int selectedSeat = 0;
    
    for (int i=1;i<=capacity;i++){
        QPushButton *btn = new QPushButton(QString::number(i), seatGrid);
        btn->setCheckable(true);
        btn->setFixedSize(50, 50);
        bool isBooked = booked.count(i)>0;
        if (isBooked) {
            btn->setEnabled(false);
            btn->setStyleSheet("QPushButton { background: #dc2626; color: #fff; border: 1px solid #991b1b; border-radius: 6px; font-weight: 600; }");
        } else {
            btn->setStyleSheet("QPushButton { background: #1f2937; color: #e5e7eb; border: 1px solid #334155; border-radius: 6px; } QPushButton:checked { background: #2563eb; color: #f8fafc; font-weight: 700; }");
            seatGroup->addButton(btn, i);
        }
        int r = (i-1)/cols, c = (i-1)%cols; grid->addWidget(btn, r, c);
    }
    layout->addWidget(seatGrid);
    
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, [&](){ 
        selectedSeat = seatGroup->checkedId();
        std::cout << "DEBUG: Seat selected = " << selectedSeat << std::endl;
        dlg.accept();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    
    if (dlg.exec() != QDialog::Accepted) return;
    
    int seatNo = selectedSeat;
    QString phone = phoneEdit->text().trimmed();
    QString pay = payment->currentText();
    
    if (seatNo<=0){ QMessageBox::warning(this,"Đặt vé","Chọn một ghế"); return; }
    if (phone.isEmpty()){ QMessageBox::warning(this,"Đặt vé","Nhập số điện thoại"); return; }
    
    // Determine file based on trip ID: T005 -> TK005.txt
    std::string fileId = "TK" + tripId.mid(1).toStdString();  // T005 -> 005 -> TK005

    // Scan trip file once to find max ticket ID
    int maxTkNum = 0;
    {
        std::ifstream in("Data/Ticket/" + fileId + ".txt");
        std::string ln;
        while (std::getline(in, ln)) {
            if (ln.empty()) continue;
            try {
                Ticket t = Ticket::fromCSV(ln);
                std::string id = t.getId();
                if (id.rfind("TK", 0) == 0) {
                    try {
                        int num = std::stoi(id.substr(2));
                        if (num > maxTkNum) maxTkNum = num;
                    } catch (...) {}
                }
            } catch (...) {}
        }
    }

    // Create new ticket ID: if TK012 is max, next is TK013
    std::string newId = "TK" + std::string(3 - std::to_string(maxTkNum + 1).length(), '0') + std::to_string(maxTkNum + 1);
    std::string phoneStr = phone.toStdString();
    std::string payStr = pay.toStdString();
    std::string bookedAtStr = (dateStr + " 00:00").toStdString();
    Ticket tk(newId, tripId.toStdString(), busId, seatNo, userName, phoneStr, tripPrice, bookedAtStr, payStr);
    
    try {
        std::cout << "DEBUG: Saving ticket to file: " << fileId << std::endl;

        // Append safely to trip file: ensure newline before appending
        std::string path = std::string("Data/Ticket/") + fileId + ".txt";
        {
            std::fstream out(path, std::ios::in | std::ios::out | std::ios::app);
            if (!out.is_open()) {
                std::cout << "DEBUG: Failed to open file" << std::endl;
                QMessageBox::critical(this, "Đặt vé", "Không thể lưu vé");
                return;
            }
            out.seekg(0, std::ios::end);
            std::streampos sz = out.tellg();
            if (sz > 0) {
                out.seekg(-1, std::ios::end);
                char last; out.get(last);
                if (last != '\n') out << '\n';
            }
            out << tk.toCSV() << "\n";
            out.close();
        }
        std::cout << "DEBUG: File written successfully" << std::endl;
        
        std::cout << "DEBUG: Adding ticket to memory" << std::endl;
        tickets.push_back(tk);
        std::cout << "DEBUG: Total tickets now: " << tickets.size() << std::endl;
        
        // Directly add row to table instead of calling populateMyTickets()
        std::cout << "DEBUG: Adding row to table directly" << std::endl;
        int row = myTicketsTable->rowCount();
        myTicketsTable->insertRow(row);
        myTicketsTable->setItem(row,0,new QTableWidgetItem(QString::fromStdString(tk.getId())));
        myTicketsTable->setItem(row,1,new QTableWidgetItem(QString::fromStdString(tk.getTripId())));
        myTicketsTable->setItem(row,2,new QTableWidgetItem(QString::number(tk.getSeatNo())));
        myTicketsTable->setItem(row,3,new QTableWidgetItem(QString::fromStdString(tk.getPassengerName())));
        myTicketsTable->setItem(row,4,new QTableWidgetItem(QString::fromStdString(tk.getPhoneNumber())));
        myTicketsTable->setItem(row,5,new QTableWidgetItem(QString::number(tk.getPrice())));
        myTicketsTable->setItem(row,6,new QTableWidgetItem(QString::fromStdString(tk.getBookedAt())));
        std::cout << "DEBUG: Row added successfully" << std::endl;
        
        QMessageBox::information(this, "Đặt vé", "Đặt vé thành công");
    } catch (const std::exception &e) {
        std::cout << "DEBUG: Exception: " << e.what() << std::endl;
        QMessageBox::critical(this, "Lỗi", QString("Lỗi khi lưu vé: %1").arg(e.what()));
    } catch (...) {
        std::cout << "DEBUG: Unknown exception" << std::endl;
        QMessageBox::critical(this, "Lỗi", "Lỗi không xác định khi lưu vé");
    }
}

void UserWindow::onViewSeatMapClicked() {
    auto items = tripTable->selectedItems();
    if (items.isEmpty()) { QMessageBox::warning(this, "Sơ đồ ghế", "Chọn một chuyến"); return; }
    int row = items.first()->row();
    QString tripId = tripTable->item(row,0)->text();
    std::string busId; for (const auto &tr : trips){ if (tr.getId()==tripId.toStdString()) { busId = tr.getBusId(); break; } }
    if (busId.empty()) { QMessageBox::warning(this, "Sơ đồ ghế", "Không tìm thấy xe"); return; }
    QString dateStr = calendar->selectedDate().toString("yyyy-MM-dd");
    std::set<int> booked;
    for (const auto &t: tickets){ if (t.getBusId()==busId){ std::string bAt = t.getBookedAt(); if (bAt.rfind(dateStr.toStdString(),0)==0){ booked.insert(t.getSeatNo()); } } }
    
    // Show visual seat map
    QDialog dlg(this); dlg.setWindowTitle("Sơ đồ ghế"); dlg.setStyleSheet("background: #0f172a; color: #e5e7eb;");
    QVBoxLayout *lay = new QVBoxLayout(&dlg);
    lay->addWidget(new QLabel(QString("Ngày %1 - Xe %2").arg(dateStr).arg(QString::fromStdString(busId))));
    
    QWidget *seatWidget = new QWidget(); QGridLayout *gl = new QGridLayout(seatWidget);
    int busCapacity = 0; for (const auto &b: buses){ if (b.getId()==busId) { busCapacity=b.getCapacity(); break; } }
    int cols = 4;
    
    for (int i=1; i<=busCapacity; i++){
        QPushButton *btn = new QPushButton(QString::number(i)); btn->setMinimumSize(50,50);
        QString style;
        if (booked.count(i)) style = "background: #dc2626; color: white; border: none; border-radius: 4px; font-weight: bold;";
        else style = "background: #1f2937; color: white; border: 1px solid #374151; border-radius: 4px;";
        btn->setStyleSheet(style); btn->setDisabled(true);
        gl->addWidget(btn, (i-1)/cols, (i-1)%cols);
    }
    lay->addWidget(seatWidget);
    QDialogButtonBox *btns = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg); 
    lay->addWidget(btns); QObject::connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    dlg.exec();
}

void UserWindow::onCancelMyTicketClicked() {
    auto items = myTicketsTable->selectedItems();
    if (items.isEmpty()){ QMessageBox::warning(this,"Hủy vé","Chọn một vé để hủy"); return; }
    int row = items.first()->row();
    QString id = myTicketsTable->item(row,0)->text();
    std::string idStr = id.toStdString();

    // Find ticket in memory to know its trip (and file)
    auto it = std::find_if(tickets.begin(), tickets.end(), [&](const Ticket &t){ return t.getId() == idStr; });
    if (it == tickets.end()) { QMessageBox::warning(this,"Hủy vé","Không tìm thấy vé"); return; }

    std::string tripIdStr = it->getTripId();
    if (tripIdStr.empty() || tripIdStr.size() < 2 || tripIdStr[0] != 'T') {
        QMessageBox::warning(this,"Hủy vé","Không xác định được file vé");
        return;
    }

    // File mapping: T005 -> TK005.txt
    std::string fileId = "TK" + tripIdStr.substr(1) + ".txt";

    // Read file for this trip and remove the target ticket by ID
    std::vector<std::string> keptLines;
    {
        std::ifstream in(std::string("Data/Ticket/") + fileId);
        std::string ln;
        while (std::getline(in, ln)) {
            if (ln.empty()) continue;
            try {
                Ticket t = Ticket::fromCSV(ln);
                if (t.getId() == idStr) continue; // skip cancelled
            } catch (...) {
                // If parse fails, keep line to avoid data loss
            }
            keptLines.push_back(ln);
        }
    }

    std::ofstream out(std::string("Data/Ticket/") + fileId, std::ios::trunc);
    for (const auto &ln : keptLines) out << ln << "\n";
    out.close();

    // Update memory: remove the cancelled ticket globally
    tickets.erase(std::remove_if(tickets.begin(), tickets.end(), [&](const Ticket&t){ return t.getId()==idStr; }), tickets.end());
    populateMyTickets();
    QMessageBox::information(this, "Hủy vé", "Đã hủy vé");
}

void UserWindow::onLogoutClicked() {
    emit logout();
}
