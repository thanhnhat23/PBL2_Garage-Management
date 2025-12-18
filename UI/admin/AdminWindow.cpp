#include "AdminWindow.h"
#include "../Class/FareCalculator.h"
#include "StyleHelper.h"
#include "CRUDDialogs.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSvgRenderer>
#include <QPainter>
#include <QFormLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QGroupBox>
#include <QLabel>
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>
#include <QComboBox>
#include <QDateEdit>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QCalendarWidget>
#include <QPixmap>
#include <QFile>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <set>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem;

AdminWindow::AdminWindow(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("QWidget { background: #0f1419; color: #e2e8f0; font-family: 'Segoe UI', sans-serif; }");
    setupUI();
    QTimer::singleShot(100, this, &AdminWindow::loadAllData);
}

AdminWindow::~AdminWindow() {
}

QIcon AdminWindow::renderSvgIcon(const QString& resourcePath, const QSize& size, const QString& colorHex) {
    // Load SVG content from Qt resource
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "ERROR: Cannot open SVG resource:" << resourcePath;
        QPixmap px(size); px.fill(Qt::transparent); QPainter p(&px);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(QPen(QColor("#3b82f6"), 2));
        p.drawEllipse(QPointF(size.width()/2.0, size.height()/2.0), size.width()/2.5, size.height()/2.5);
        p.drawLine(QPointF(size.width()/2.0, size.height()/2.0 - 4), QPointF(size.width()/2.0, size.height()/2.0 + 4));
        return QIcon(px);
    }
    QByteArray data = file.readAll();
    file.close();
    // Replace currentColor with desired hex
    QString svg = QString::fromUtf8(data);
    svg.replace("currentColor", colorHex, Qt::CaseInsensitive);
    QByteArray patched = svg.toUtf8();

    QSvgRenderer renderer(patched);
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    if (!renderer.isValid()) {
        qDebug() << "ERROR: SVG invalid after patch:" << resourcePath << "— using fallback";
        painter.setPen(QPen(QColor(colorHex), 2));
        painter.drawEllipse(QPointF(size.width()/2.0, size.height()/2.0), size.width()/2.5, size.height()/2.5);
        painter.drawLine(QPointF(size.width()/2.0, size.height()/2.0 - 4), QPointF(size.width()/2.0, size.height()/2.0 + 4));
        return QIcon(pixmap);
    }
    renderer.render(&painter);
    return QIcon(pixmap);
}

void AdminWindow::setUser(User user) {
    currentUser = user;
    loadAllData();
}

void AdminWindow::setupUI() {
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
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);
    
    // Logo
    QLabel *lblLogo = new QLabel("GARAGE", sidebarWidget);
    lblLogo->setStyleSheet(
        "color: #3b82f6;"
        "font-size: 20px;"
        "font-weight: 800;"
        "padding: 25px 20px;"
    );
    sidebarLayout->addWidget(lblLogo);
    
    // Menu buttons
    QStringList menuItems = {"Brand", "Route", "Bus", "Driver", "Trip", "Ticket", "Statistics"};
    QStringList menuIcons = {":/icons/icons/brand.svg", ":/icons/icons/route.svg", ":/icons/icons/bus.svg", ":/icons/icons/driver.svg", ":/icons/icons/trip.svg", ":/icons/icons/ticket.svg", ":/icons/icons/stats.svg"};
    
    // Style for active button
    QString activeButtonStyle =
        "QPushButton {"
        "  background: rgba(59, 130, 246, 0.2);"
        "  color: #3b82f6;"
        "  border: none;"
        "  text-align: left;"
        "  padding: 15px 20px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "  border-right: 3px solid #3b82f6;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(59, 130, 246, 0.15);"
        "}"
        "QPushButton:pressed {"
        "  background: rgba(59, 130, 246, 0.25);"
        "}";
    
    // Style for inactive button
    QString inactiveButtonStyle =
        "QPushButton {"
        "  background: transparent;"
        "  color: #cbd5e1;"
        "  border: none;"
        "  text-align: left;"
        "  padding: 15px 20px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(59, 130, 246, 0.1);"
        "  color: #3b82f6;"
        "}"
        "QPushButton:pressed {"
        "  background: rgba(59, 130, 246, 0.2);"
        "}";
    
    for (int i = 0; i < menuItems.count(); i++) {
        QPushButton *btn = new QPushButton(menuItems[i], sidebarWidget);
        // Use inactive color for default icons; active button may be re-rendered later if needed
        btn->setIcon(renderSvgIcon(menuIcons[i], QSize(24,24), QString("#cbd5e1")));
        btn->setIconSize(QSize(24, 24));
        btn->setMinimumHeight(45);
        btn->setMinimumWidth(150);
        btn->setFlat(true);
        btn->setStyleSheet(i == 0 ? activeButtonStyle : inactiveButtonStyle);
        btn->setCursor(Qt::PointingHandCursor);
        
        // Store first button as active
        if (i == 0) {
            activeButton = btn;
        }
        
        connect(btn, &QPushButton::clicked, [this, btn, activeButtonStyle, inactiveButtonStyle, i]() {
            // Remove highlight from previous active button
            if (activeButton) {
                activeButton->setStyleSheet(inactiveButtonStyle);
            }
            // Highlight new active button
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
    connect(btnLogout, &QPushButton::clicked, this, &AdminWindow::onLogoutClicked);
    
    sidebarLayout->addStretch();
    sidebarLayout->addWidget(btnLogout);
    
    // ===== MAIN CONTENT (Right) =====
    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(30, 30, 30, 30);
    contentLayout->setSpacing(20);
    
    // Header
    QLabel *lblTitle = new QLabel("ADMIN PANEL", contentWidget);
    lblTitle->setStyleSheet(
        "font-size: 28px;"
        "font-weight: 800;"
        "color: #f1f5f9;"
    );
    contentLayout->addWidget(lblTitle);
    
    // Tab Widget (hidden tab bar - navigation via sidebar only)
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(StyleHelper::getTabStyle());
    tabWidget->tabBar()->hide();  // Hide top navbar
    
    // Create tabs - matching console menu order
    setupBrandsTab();
    setupRoutesTab();
    setupBusesTab();
    setupDriversTab();
    setupTripsTab();
    setupTicketsTab();
    setupStatsTab();
    
    contentLayout->addWidget(tabWidget);

    // Lazy-load tickets when the tickets tab is activated
    connect(tabWidget, &QTabWidget::currentChanged, this, [this](int idx){
        if (idx == ticketsTabIndex) {
            loadTicketsData();
        }
    });
    
    mainLayout->addWidget(sidebarWidget);
    mainLayout->addWidget(contentWidget, 1);
    setLayout(mainLayout);
}

void AdminWindow::setupBrandsTab() {
    QWidget *brandTab = new QWidget();
    brandTab->setStyleSheet("background: #0f172a;");
    QVBoxLayout *layout = new QVBoxLayout(brandTab);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(15);
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    
    QPushButton *btnAdd = new QPushButton("Add Brand", brandTab);
    btnAdd->setIcon(renderSvgIcon(":/icons/icons/add.svg", QSize(16,16), "#22c55e"));
    btnAdd->setIconSize(QSize(16,16));
    btnAdd->setStyleSheet(StyleHelper::getSuccessButtonStyle());
    btnAdd->setCursor(Qt::PointingHandCursor);
    connect(btnAdd, &QPushButton::clicked, this, &AdminWindow::onAddBrandClicked);
    
    QPushButton *btnDelete = new QPushButton("Remove", brandTab);
    btnDelete->setIcon(renderSvgIcon(":/icons/icons/delete.svg", QSize(16,16), "#ef4444"));
    btnDelete->setIconSize(QSize(16,16));
    btnDelete->setStyleSheet(StyleHelper::getDangerButtonStyle());
    btnDelete->setCursor(Qt::PointingHandCursor);
    connect(btnDelete, &QPushButton::clicked, this, &AdminWindow::onDeleteBrandClicked);
    
    QPushButton *btnRefresh = new QPushButton("Refresh", brandTab);
    btnRefresh->setIcon(renderSvgIcon(":/icons/icons/refresh.svg", QSize(16,16), "#64748b"));
    btnRefresh->setIconSize(QSize(16,16));
    btnRefresh->setStyleSheet(StyleHelper::getSecondaryButtonStyle());
    btnRefresh->setCursor(Qt::PointingHandCursor);
    connect(btnRefresh, &QPushButton::clicked, this, &AdminWindow::onRefreshClicked);
    
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnDelete);
    btnLayout->addWidget(btnRefresh);

    // Search (left) + buttons (right)
    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLabel *lblSearch = new QLabel("Search:", brandTab);
    lblSearch->setStyleSheet("font-size: 14px; font-weight: 600; color: #e5e7eb;");
    QLineEdit *txtSearch = new QLineEdit(brandTab);
    txtSearch->setPlaceholderText("Enter your input...");
    txtSearch->setStyleSheet(StyleHelper::getInputStyle());
    txtSearch->setMinimumHeight(36);
    connect(txtSearch, &QLineEdit::textChanged, this, [this](const QString& text){
        brandSearch = text;
        populateBrandsTable();
    });
    searchLayout->addWidget(lblSearch);
    searchLayout->addWidget(txtSearch);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addLayout(searchLayout);
    topLayout->addStretch();
    topLayout->addLayout(btnLayout);
    
    // Table
    QTableWidget *table = new QTableWidget(brandTab);
    table->setObjectName("brandTable");
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"ID", "Brand Name", "Hotline", "Rating"});
    table->setStyleSheet(StyleHelper::getTableStyle());
    table->horizontalHeader()->setStretchLastSection(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setFocusPolicy(Qt::StrongFocus);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    auto brandHeader = table->horizontalHeader();
    brandHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    brandHeader->setSectionResizeMode(1, QHeaderView::Stretch);
    
    layout->addLayout(topLayout);
    layout->addWidget(table);
    layout->setStretch(0, 0);
    layout->setStretch(1, 1);
    
    connect(table, &QTableWidget::cellDoubleClicked, this, [this](int, int){ onEditBrandClicked(); });
    
    tabWidget->addTab(brandTab, "Brand");
}

void AdminWindow::setupDriversTab() {
    QWidget *driverTab = new QWidget();
    driverTab->setStyleSheet("background: #0f172a;");
    QVBoxLayout *layout = new QVBoxLayout(driverTab);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(15);
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    
    QPushButton *btnAdd = new QPushButton("Add Driver", driverTab);
    btnAdd->setIcon(renderSvgIcon(":/icons/icons/add.svg", QSize(16,16), "#22c55e"));
    btnAdd->setIconSize(QSize(16,16));
    btnAdd->setStyleSheet(StyleHelper::getSuccessButtonStyle());
    btnAdd->setCursor(Qt::PointingHandCursor);
    connect(btnAdd, &QPushButton::clicked, this, &AdminWindow::onAddDriverClicked);

    QPushButton *btnDelete = new QPushButton("Remove", driverTab);
    btnDelete->setIcon(renderSvgIcon(":/icons/icons/delete.svg", QSize(16,16), "#ef4444"));
    btnDelete->setIconSize(QSize(16,16));
    btnDelete->setStyleSheet(StyleHelper::getDangerButtonStyle());
    btnDelete->setCursor(Qt::PointingHandCursor);
    connect(btnDelete, &QPushButton::clicked, this, &AdminWindow::onDeleteDriverClicked);
    
    QPushButton *btnRefresh = new QPushButton("Refresh", driverTab);
    btnRefresh->setIcon(renderSvgIcon(":/icons/icons/refresh.svg", QSize(16,16), "#64748b"));
    btnRefresh->setIconSize(QSize(16,16));
    btnRefresh->setStyleSheet(StyleHelper::getSecondaryButtonStyle());
    btnRefresh->setCursor(Qt::PointingHandCursor);
    connect(btnRefresh, &QPushButton::clicked, this, &AdminWindow::onRefreshClicked);
    
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnDelete);
    btnLayout->addWidget(btnRefresh);
    
    // Table
    QTableWidget *table = new QTableWidget(driverTab);
    table->setObjectName("driverTable");
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"ID", "Name", "Bus", "Phone", "Experience"});
    table->setStyleSheet(StyleHelper::getTableStyle());
    table->horizontalHeader()->setStretchLastSection(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setFocusPolicy(Qt::StrongFocus);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    auto driverHeader = table->horizontalHeader();
    driverHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    driverHeader->setSectionResizeMode(1, QHeaderView::Stretch);
    driverHeader->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    driverHeader->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    driverHeader->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    
    // Search (left) + buttons (right)
    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLabel *lblSearch = new QLabel("Search:", driverTab);
    lblSearch->setStyleSheet("font-size: 14px; font-weight: 600; color: #e5e7eb;");
    QLineEdit *txtSearch = new QLineEdit(driverTab);
    txtSearch->setPlaceholderText("Enter your input...");
    txtSearch->setStyleSheet(StyleHelper::getInputStyle());
    txtSearch->setMinimumHeight(36);
    connect(txtSearch, &QLineEdit::textChanged, this, [this](const QString& text){
        driverSearch = text;
        populateDriversTable();
    });
    searchLayout->addWidget(lblSearch);
    searchLayout->addWidget(txtSearch);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addLayout(searchLayout);
    topLayout->addStretch();
    topLayout->addLayout(btnLayout);

    layout->addLayout(topLayout);
    layout->addWidget(table);
    layout->setStretch(0, 0);
    layout->setStretch(1, 1);
    
    connect(table, &QTableWidget::cellDoubleClicked, this, [this](int, int){ onEditDriverClicked(); });
    
    tabWidget->addTab(driverTab, "Driver");
}

void AdminWindow::setupSeatsTab() {
    QWidget *seatTab = new QWidget();
    seatTab->setStyleSheet("background: #0f172a;");
    QVBoxLayout *layout = new QVBoxLayout(seatTab);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(15);
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    
    QPushButton *btnView = new QPushButton("View Seat Map", seatTab);
    btnView->setIcon(renderSvgIcon(":/icons/icons/seat-map.svg", QSize(16,16), "#3b82f6"));
    btnView->setIconSize(QSize(16, 16));
    btnView->setStyleSheet(StyleHelper::getPrimaryButtonStyle());
    btnView->setCursor(Qt::PointingHandCursor);
    connect(btnView, &QPushButton::clicked, this, &AdminWindow::onViewSeatMapClicked);
    
    QPushButton *btnRefresh = new QPushButton("Refresh", seatTab);
    btnRefresh->setIcon(renderSvgIcon(":/icons/icons/refresh.svg", QSize(16,16), "#64748b"));
    btnRefresh->setIconSize(QSize(16,16));
    btnRefresh->setStyleSheet(StyleHelper::getSecondaryButtonStyle());
    btnRefresh->setCursor(Qt::PointingHandCursor);
    connect(btnRefresh, &QPushButton::clicked, this, &AdminWindow::onRefreshClicked);
    
    btnLayout->addWidget(btnView);
    btnLayout->addWidget(btnRefresh);
    
    // Table - Display buses instead of seats
    QTableWidget *table = new QTableWidget(seatTab);
    table->setObjectName("seatTable");
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Bus ID", "License Plate", "Brand", "Type", "Seats"});
    table->setStyleSheet(StyleHelper::getTableStyle());
    table->horizontalHeader()->setStretchLastSection(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setFocusPolicy(Qt::StrongFocus);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    auto seatHeader = table->horizontalHeader();
    seatHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    seatHeader->setSectionResizeMode(1, QHeaderView::Stretch);
    seatHeader->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    seatHeader->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    seatHeader->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    
    layout->addLayout(btnLayout);
    layout->addWidget(table);
    
    tabWidget->addTab(seatTab, "Seats");
}

void AdminWindow::setupRoutesTab() {
    QWidget *routeTab = new QWidget();
    routeTab->setStyleSheet("background: #0f172a;");
    QVBoxLayout *layout = new QVBoxLayout(routeTab);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(15);
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    
    QPushButton *btnAdd = new QPushButton("Add Route", routeTab);
    btnAdd->setIcon(renderSvgIcon(":/icons/icons/add.svg", QSize(16,16), "#22c55e"));
    btnAdd->setIconSize(QSize(16,16));
    btnAdd->setStyleSheet(StyleHelper::getSuccessButtonStyle());
    btnAdd->setCursor(Qt::PointingHandCursor);
    connect(btnAdd, &QPushButton::clicked, this, &AdminWindow::onAddRouteClicked);

    QPushButton *btnDelete = new QPushButton("Remove", routeTab);
    btnDelete->setIcon(renderSvgIcon(":/icons/icons/delete.svg", QSize(16,16), "#ef4444"));
    btnDelete->setIconSize(QSize(16,16));
    btnDelete->setStyleSheet(StyleHelper::getDangerButtonStyle());
    btnDelete->setCursor(Qt::PointingHandCursor);
    connect(btnDelete, &QPushButton::clicked, this, &AdminWindow::onDeleteRouteClicked);
    
    QPushButton *btnRefresh = new QPushButton("Refresh", routeTab);
    btnRefresh->setIcon(renderSvgIcon(":/icons/icons/refresh.svg", QSize(16,16), "#64748b"));
    btnRefresh->setIconSize(QSize(16,16));
    btnRefresh->setStyleSheet(StyleHelper::getSecondaryButtonStyle());
    btnRefresh->setCursor(Qt::PointingHandCursor);
    connect(btnRefresh, &QPushButton::clicked, this, &AdminWindow::onRefreshClicked);
    
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnDelete);
    btnLayout->addWidget(btnRefresh);

    // Search (left) + buttons (right)
    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLabel *lblSearch = new QLabel("Search:", routeTab);
    lblSearch->setStyleSheet("font-size: 14px; font-weight: 600; color: #e5e7eb;");
    QLineEdit *txtSearch = new QLineEdit(routeTab);
    txtSearch->setPlaceholderText("Enter your input...");
    txtSearch->setStyleSheet(StyleHelper::getInputStyle());
    txtSearch->setMinimumHeight(36);
    connect(txtSearch, &QLineEdit::textChanged, this, [this](const QString& text){
        routeSearch = text;
        populateRoutesTable();
    });
    searchLayout->addWidget(lblSearch);
    searchLayout->addWidget(txtSearch);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addLayout(searchLayout);
    topLayout->addStretch();
    topLayout->addLayout(btnLayout);
    
    // Table
    QTableWidget *table = new QTableWidget(routeTab);
    table->setObjectName("routeTable");
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"ID", "Route Name", "Start Point", "End Point", "Km", "Duration (minutes)"});
    table->setStyleSheet(StyleHelper::getTableStyle());
    table->horizontalHeader()->setStretchLastSection(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setFocusPolicy(Qt::StrongFocus);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    auto routeHeader = table->horizontalHeader();
    routeHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    routeHeader->setSectionResizeMode(1, QHeaderView::Stretch);
    routeHeader->setSectionResizeMode(2, QHeaderView::Stretch);
    routeHeader->setSectionResizeMode(3, QHeaderView::Stretch);
    routeHeader->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    routeHeader->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    
    layout->addLayout(topLayout);
    layout->addWidget(table);
    layout->setStretch(0, 0);
    layout->setStretch(1, 1);
    
    connect(table, &QTableWidget::cellDoubleClicked, this, [this](int, int){ onEditRouteClicked(); });
    
    tabWidget->addTab(routeTab, "📍 Tuyến");
}

void AdminWindow::setupBusesTab() {
    QWidget *busTab = new QWidget();
    busTab->setStyleSheet("background: #0f172a;");
    QVBoxLayout *layout = new QVBoxLayout(busTab);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(15);
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    
    QPushButton *btnAdd = new QPushButton("Add Bus", busTab);
    btnAdd->setIcon(renderSvgIcon(":/icons/icons/add.svg", QSize(16,16), "#22c55e"));
    btnAdd->setIconSize(QSize(16,16));
    btnAdd->setStyleSheet(StyleHelper::getSuccessButtonStyle());
    btnAdd->setCursor(Qt::PointingHandCursor);
    connect(btnAdd, &QPushButton::clicked, this, &AdminWindow::onAddBusClicked);

    QPushButton *btnDelete = new QPushButton("Remove", busTab);
    btnDelete->setIcon(renderSvgIcon(":/icons/icons/delete.svg", QSize(16,16), "#ef4444"));
    btnDelete->setIconSize(QSize(16,16));
    btnDelete->setStyleSheet(StyleHelper::getDangerButtonStyle());
    btnDelete->setCursor(Qt::PointingHandCursor);
    connect(btnDelete, &QPushButton::clicked, this, &AdminWindow::onDeleteBusClicked);
    
    QPushButton *btnSeats = new QPushButton("View Seats", busTab);
    btnSeats->setIcon(renderSvgIcon(":/icons/icons/seat-map.svg", QSize(16,16), "#3b82f6"));
    btnSeats->setIconSize(QSize(16, 16));
    btnSeats->setStyleSheet(StyleHelper::getPrimaryButtonStyle());
    btnSeats->setCursor(Qt::PointingHandCursor);
    connect(btnSeats, &QPushButton::clicked, this, &AdminWindow::onViewSeatMapClicked);
    
    QPushButton *btnRefresh = new QPushButton("Refresh", busTab);
    btnRefresh->setIcon(renderSvgIcon(":/icons/icons/refresh.svg", QSize(16,16), "#64748b"));
    btnRefresh->setIconSize(QSize(16,16));
    btnRefresh->setStyleSheet(StyleHelper::getSecondaryButtonStyle());
    btnRefresh->setCursor(Qt::PointingHandCursor);
    connect(btnRefresh, &QPushButton::clicked, this, &AdminWindow::onRefreshClicked);
    
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnDelete);
    btnLayout->addWidget(btnSeats);
    btnLayout->addWidget(btnRefresh);

    // Search (left) + buttons (right)
    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLabel *lblSearch = new QLabel("Search:", busTab);
    lblSearch->setStyleSheet("font-size: 14px; font-weight: 600; color: #e5e7eb;");
    QLineEdit *txtSearch = new QLineEdit(busTab);
    txtSearch->setPlaceholderText("Enter your input...");
    txtSearch->setStyleSheet(StyleHelper::getInputStyle());
    txtSearch->setMinimumHeight(36);
    connect(txtSearch, &QLineEdit::textChanged, this, [this](const QString& text){
        busSearch = text;
        populateBusesTable();
    });
    searchLayout->addWidget(lblSearch);
    searchLayout->addWidget(txtSearch);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addLayout(searchLayout);
    topLayout->addStretch();
    topLayout->addLayout(btnLayout);
    
    // Table
    QTableWidget *table = new QTableWidget(busTab);
    table->setObjectName("busTable");
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Bus ID", "License Plate", "Brand", "Type", "Seats"});
    table->setStyleSheet(StyleHelper::getTableStyle());
    table->horizontalHeader()->setStretchLastSection(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    auto busHeader = table->horizontalHeader();
    busHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    busHeader->setSectionResizeMode(1, QHeaderView::Stretch);
    busHeader->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    busHeader->setSectionResizeMode(3, QHeaderView::Stretch);
    busHeader->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    
    layout->addLayout(topLayout);
    layout->addWidget(table);
    layout->setStretch(0, 0);
    layout->setStretch(1, 1);
    
    connect(table, &QTableWidget::cellDoubleClicked, this, [this](int, int){ onEditBusClicked(); });
    
    tabWidget->addTab(busTab, "🚌 Xe");
}

void AdminWindow::setupTripsTab() {
    QWidget *tripTab = new QWidget();
    tripTab->setStyleSheet("background: #0f172a;");
    QVBoxLayout *layout = new QVBoxLayout(tripTab);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(15);
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    
    QPushButton *btnAdd = new QPushButton("Add Trip", tripTab);
    btnAdd->setIcon(renderSvgIcon(":/icons/icons/add.svg", QSize(16,16), "#22c55e"));
    btnAdd->setIconSize(QSize(16,16));
    btnAdd->setStyleSheet(StyleHelper::getSuccessButtonStyle());
    btnAdd->setCursor(Qt::PointingHandCursor);
    connect(btnAdd, &QPushButton::clicked, this, &AdminWindow::onAddTripClicked);

    QPushButton *btnDelete = new QPushButton("Remove", tripTab);
    btnDelete->setIcon(renderSvgIcon(":/icons/icons/delete.svg", QSize(16,16), "#ef4444"));
    btnDelete->setIconSize(QSize(16,16));
    btnDelete->setStyleSheet(StyleHelper::getDangerButtonStyle());
    btnDelete->setCursor(Qt::PointingHandCursor);
    connect(btnDelete, &QPushButton::clicked, this, &AdminWindow::onDeleteTripClicked);
    
    QPushButton *btnRefresh = new QPushButton("Refresh", tripTab);
    btnRefresh->setIcon(renderSvgIcon(":/icons/icons/refresh.svg", QSize(16,16), "#64748b"));
    btnRefresh->setIconSize(QSize(16,16));
    btnRefresh->setStyleSheet(StyleHelper::getSecondaryButtonStyle());
    btnRefresh->setCursor(Qt::PointingHandCursor);
    connect(btnRefresh, &QPushButton::clicked, this, &AdminWindow::onRefreshClicked);
    
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnDelete);
    btnLayout->addWidget(btnRefresh);

    // Search (left) + buttons (right)
    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLabel *lblSearch = new QLabel("Search:", tripTab);
    lblSearch->setStyleSheet("font-size: 14px; font-weight: 600; color: #e5e7eb;");
    QLineEdit *txtSearch = new QLineEdit(tripTab);
    txtSearch->setPlaceholderText("Enter your input...");
    txtSearch->setStyleSheet(StyleHelper::getInputStyle());
    txtSearch->setMinimumHeight(36);
    connect(txtSearch, &QLineEdit::textChanged, this, [this](const QString& text){
        tripSearch = text;
        populateTripsTable();
    });
    searchLayout->addWidget(lblSearch);
    searchLayout->addWidget(txtSearch);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addLayout(searchLayout);
    topLayout->addStretch();
    topLayout->addLayout(btnLayout);

    // Table
    QTableWidget *table = new QTableWidget(tripTab);
    table->setObjectName("tripTable");
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"ID", "Route", "Bus", "Driver", "Departure Time", "Arrival Time"});
    table->setStyleSheet(StyleHelper::getTableStyle());
    table->horizontalHeader()->setStretchLastSection(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    auto tripHeader = table->horizontalHeader();
    tripHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tripHeader->setSectionResizeMode(1, QHeaderView::Stretch);
    tripHeader->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tripHeader->setSectionResizeMode(3, QHeaderView::Stretch);
    tripHeader->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    tripHeader->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    
    layout->addLayout(topLayout);
    layout->addWidget(table);
    layout->setStretch(0, 0);
    layout->setStretch(1, 1);
    
    connect(table, &QTableWidget::cellDoubleClicked, this, [this](int, int){ onEditTripClicked(); });
    
    tabWidget->addTab(tripTab, "🚗 Chuyến");
}

void AdminWindow::setupTicketsTab() {
    QWidget *ticketTab = new QWidget();
    ticketTab->setStyleSheet("background: #0f172a;");
    QVBoxLayout *layout = new QVBoxLayout(ticketTab);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(15);
    
    // Action buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    
    QPushButton *btnBook = new QPushButton("Book Ticket", ticketTab);
    btnBook->setIcon(renderSvgIcon(":/icons/icons/ticket-add.svg", QSize(16,16), "#22c55e"));
    btnBook->setIconSize(QSize(16, 16));
    btnBook->setStyleSheet(StyleHelper::getSuccessButtonStyle());
    btnBook->setCursor(Qt::PointingHandCursor);
    connect(btnBook, &QPushButton::clicked, this, &AdminWindow::onBookTicketClicked);
    
    QPushButton *btnCancel = new QPushButton("Cancel Ticket", ticketTab);
    btnCancel->setIcon(renderSvgIcon(":/icons/icons/ticket-cancel.svg", QSize(16,16), "#ef4444"));
    btnCancel->setIconSize(QSize(16, 16));
    btnCancel->setStyleSheet(StyleHelper::getDangerButtonStyle());
    btnCancel->setCursor(Qt::PointingHandCursor);
    connect(btnCancel, &QPushButton::clicked, this, &AdminWindow::onCancelTicketClicked);
    
    QPushButton *btnRefresh = new QPushButton("Refresh", ticketTab);
    btnRefresh->setIcon(renderSvgIcon(":/icons/icons/refresh.svg", QSize(16,16), "#64748b"));
    btnRefresh->setIconSize(QSize(16,16));
    btnRefresh->setStyleSheet(StyleHelper::getSecondaryButtonStyle());
    btnRefresh->setCursor(Qt::PointingHandCursor);
    connect(btnRefresh, &QPushButton::clicked, this, &AdminWindow::onRefreshClicked);
    
    btnLayout->addWidget(btnBook);
    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnRefresh);
    
    // Master-Detail: left ticket file summary, right tickets for selected file
    QSplitter *split = new QSplitter(Qt::Horizontal, ticketTab);
    split->setStyleSheet("QSplitter { background: #0f172a; } QSplitter::handle { background: #1f2937; width: 6px; }");

    // Left: Ticket file summary table
    QTableWidget *fileTable = new QTableWidget(split);
    fileTable->setObjectName("ticketFileTable");
    fileTable->setColumnCount(3);
    fileTable->setHorizontalHeaderLabels({"Ticket File", "Bus", "Number of Tickets"});
    fileTable->setStyleSheet(StyleHelper::getTableStyle());
    fileTable->horizontalHeader()->setStretchLastSection(true);
    fileTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    fileTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    fileTable->setAlternatingRowColors(true);
    fileTable->verticalHeader()->setVisible(false);
    auto fileHeader = fileTable->horizontalHeader();
    fileHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    fileHeader->setSectionResizeMode(1, QHeaderView::Stretch);
    fileHeader->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    // Right: Tickets detail table for selected brand
    QTableWidget *ticketTable = new QTableWidget(split);
    ticketTable->setObjectName("ticketDetailTable");
        ticketTable->setColumnCount(7);
    ticketTable->setHorizontalHeaderLabels({"ID", "Trip", "Seat", "Passenger", "Phone", "Price", "Time", "Payment"});
        ticketTable->setHorizontalHeaderLabels({"ID", "Trip", "Seat", "Passenger", "Phone", "Price", "Time", "Payment"});
    ticketTable->setStyleSheet(StyleHelper::getTableStyle());
    ticketTable->horizontalHeader()->setStretchLastSection(false);
    ticketTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ticketTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ticketTable->setAlternatingRowColors(true);
    ticketTable->verticalHeader()->setVisible(false);
    auto tHeader = ticketTable->horizontalHeader();
    tHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tHeader->setSectionResizeMode(1, QHeaderView::Stretch);
    tHeader->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tHeader->setSectionResizeMode(3, QHeaderView::Stretch);
        ticketTable->setHorizontalHeaderLabels({"ID", "Trip", "Seat", "Passenger", "Phone", "Price", "Time", "Payment"});
    tHeader->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    tHeader->setSectionResizeMode(6, QHeaderView::Stretch);
    tHeader->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    
    // Search bar for tickets
    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLabel *lblSearch = new QLabel("Search:", ticketTab);
    lblSearch->setStyleSheet("font-size: 14px; font-weight: 600; color: #e5e7eb;");
    QLineEdit *txtSearch = new QLineEdit(ticketTab);
    txtSearch->setPlaceholderText("Enter your input...");
    txtSearch->setStyleSheet(StyleHelper::getInputStyle());
    txtSearch->setMinimumHeight(36);

    // Debounce search: wait 500ms after user stops typing before filtering
    ticketSearchTimer = new QTimer(ticketTab);
    ticketSearchTimer->setSingleShot(true);
    ticketSearchTimer->setInterval(500);
    connect(ticketSearchTimer, &QTimer::timeout, this, [this]() {
        populateTicketsTable();
    });
    connect(txtSearch, &QLineEdit::textChanged, this, [this](const QString& text){
        ticketSearch = text;
        if (ticketSearchTimer) {
            ticketSearchTimer->stop();
            ticketSearchTimer->start();
        }
    });
    searchLayout->addWidget(lblSearch);
    searchLayout->addWidget(txtSearch);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addLayout(searchLayout);
    topLayout->addStretch();
    topLayout->addLayout(btnLayout);

    layout->addLayout(topLayout);
    layout->addWidget(split);
    layout->setStretch(0, 0);
    layout->setStretch(1, 1);
    
    tabWidget->addTab(ticketTab, "Ticket");
    ticketsTabIndex = tabWidget->indexOf(ticketTab);

    // Do not populate immediately; will lazy-load on tab activation
    // Hook selection to load tickets for selected file
    connect(fileTable, &QTableWidget::itemSelectionChanged, this, [this, fileTable](){
        auto items = fileTable->selectedItems();
        if (!items.isEmpty()) {
            QTableWidgetItem *first = fileTable->item(items.first()->row(), 0);
            if (first) {
                selectedTicketFile = first->data(Qt::UserRole).toString().toStdString();
                populateTicketsForFile(selectedTicketFile);
            }
        }
    });
}

void AdminWindow::setupStatsTab() {
    QWidget *statsTab = new QWidget();
    statsTab->setStyleSheet("background: #0f172a;");
    QVBoxLayout *layout = new QVBoxLayout(statsTab);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(15);
    
    // Filter Controls
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(10);
    
    QLabel *lblFilter = new QLabel("Filter by:", statsTab);
    lblFilter->setStyleSheet("font-size: 14px; font-weight: 600; color: #e5e7eb;");
    
    QPushButton *btnByDay = new QPushButton("By Day", statsTab);
    btnByDay->setIcon(renderSvgIcon(":/icons/icons/calendar.svg", QSize(16,16), "#3b82f6"));
    btnByDay->setIconSize(QSize(16, 16));
    btnByDay->setStyleSheet(StyleHelper::getPrimaryButtonStyle());
    btnByDay->setCursor(Qt::PointingHandCursor);
    connect(btnByDay, &QPushButton::clicked, this, &AdminWindow::onFilterByDay);
    
    QPushButton *btnByMonth = new QPushButton("By Month", statsTab);
    btnByMonth->setIcon(renderSvgIcon(":/icons/icons/calendar.svg", QSize(16,16), "#3b82f6"));
    btnByMonth->setIconSize(QSize(16, 16));
    btnByMonth->setStyleSheet(StyleHelper::getPrimaryButtonStyle());
    btnByMonth->setCursor(Qt::PointingHandCursor);
    connect(btnByMonth, &QPushButton::clicked, this, &AdminWindow::onFilterByMonth);
    
    QPushButton *btnByYear = new QPushButton("By Year", statsTab);
    btnByYear->setIcon(renderSvgIcon(":/icons/icons/calendar.svg", QSize(16,16), "#3b82f6"));
    btnByYear->setIconSize(QSize(16, 16));
    btnByYear->setStyleSheet(StyleHelper::getPrimaryButtonStyle());
    btnByYear->setCursor(Qt::PointingHandCursor);
    connect(btnByYear, &QPushButton::clicked, this, &AdminWindow::onFilterByYear);
    
    QPushButton *btnShowAll = new QPushButton("Show All", statsTab);
    btnShowAll->setIcon(renderSvgIcon(":/icons/icons/calendar.svg", QSize(16,16), "#3b82f6"));
    btnShowAll->setIconSize(QSize(16, 16));
    btnShowAll->setStyleSheet(StyleHelper::getPrimaryButtonStyle());
    btnShowAll->setCursor(Qt::PointingHandCursor);
    connect(btnShowAll, &QPushButton::clicked, this, &AdminWindow::onShowAllStats);
    
    filterLayout->addWidget(lblFilter);
    filterLayout->addWidget(btnByDay);
    filterLayout->addWidget(btnByMonth);
    filterLayout->addWidget(btnByYear);
    filterLayout->addWidget(btnShowAll);
    filterLayout->addStretch();
    
    layout->addLayout(filterLayout);
    
    // Summary cards
    QHBoxLayout *summaryLayout = new QHBoxLayout();
    summaryLayout->setSpacing(15);
    
    // Total tickets card
    QWidget *card1 = new QWidget(statsTab);
    card1->setStyleSheet("background: #0b1220; border: 1px solid #1f2937; border-radius: 12px; padding: 18px;");
    QVBoxLayout *card1Layout = new QVBoxLayout(card1);
    QLabel *lblTotalTickets = new QLabel("Total Tickets: 0", card1);
    lblTotalTickets->setObjectName("lblTotalTickets");
    lblTotalTickets->setStyleSheet("font-size: 18px; font-weight: 700; color: #e5e7eb;");
    card1Layout->addWidget(lblTotalTickets);
    
    // Total revenue card
    QWidget *card2 = new QWidget(statsTab);
    card2->setStyleSheet("background: #0b1220; border: 1px solid #1f2937; border-radius: 12px; padding: 18px;");
    QVBoxLayout *card2Layout = new QVBoxLayout(card2);
    QLabel *lblTotalRevenue = new QLabel("Total Revenue: 0 VND", card2);
    lblTotalRevenue->setObjectName("lblTotalRevenue");
    lblTotalRevenue->setStyleSheet("font-size: 18px; font-weight: 700; color: #e5e7eb;");
    card2Layout->addWidget(lblTotalRevenue);
    
    // Average price card
    QWidget *card3 = new QWidget(statsTab);
    card3->setStyleSheet("background: #0b1220; border: 1px solid #1f2937; border-radius: 12px; padding: 18px;");
    QVBoxLayout *card3Layout = new QVBoxLayout(card3);
    QLabel *lblAvgPrice = new QLabel("Average: 0 VND", card3);
    lblAvgPrice->setObjectName("lblAvgPrice");
    lblAvgPrice->setStyleSheet("font-size: 18px; font-weight: 700; color: #e5e7eb;");
    card3Layout->addWidget(lblAvgPrice);
    
    summaryLayout->addWidget(card1, 1);
    summaryLayout->addWidget(card2, 1);
    summaryLayout->addWidget(card3, 1);
    
    // Table
    QTableWidget *table = new QTableWidget(statsTab);
    table->setObjectName("statsTable");
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({"ID", "Passenger", "Route", "Bus", "Seat", "Price", "Time", "Payment Method"});
    table->setStyleSheet(StyleHelper::getTableStyle());
    table->horizontalHeader()->setStretchLastSection(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    auto statsHeader = table->horizontalHeader();
    statsHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    statsHeader->setSectionResizeMode(1, QHeaderView::Stretch);
    statsHeader->setSectionResizeMode(2, QHeaderView::Stretch);
    statsHeader->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    statsHeader->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    statsHeader->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    statsHeader->setSectionResizeMode(6, QHeaderView::Stretch);
    statsHeader->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    
    layout->addLayout(summaryLayout);
    layout->addWidget(table);
    
    tabWidget->addTab(statsTab, "Statistics");
}

void AdminWindow::loadAllData() {
    // Load brands
    brands.clear();
    std::ifstream brandFile("Data/Brand.txt");
    std::string line;
    while (std::getline(brandFile, line)) {
        if (!line.empty()) brands.push_back(Brand::fromCSV(line));
    }
    
    // Load routes
    routes.clear();
    std::ifstream routeFile("Data/Route.txt");
    while (std::getline(routeFile, line)) {
        if (!line.empty()) routes.push_back(Route::fromCSV(line));
    }
    
    // Load buses
    buses.clear();
    std::ifstream busFile("Data/Bus.txt");
    while (std::getline(busFile, line)) {
        if (!line.empty()) buses.push_back(Bus::fromCSV(line));
    }
    
    // Load trips
    trips.clear();
    std::ifstream tripFile("Data/Trip.txt");
    while (std::getline(tripFile, line)) {
        if (!line.empty()) trips.push_back(Trip::fromCSV(line));
    }
    
    // Load drivers
    drivers.clear();
    std::ifstream driverFile("Data/Driver.txt");
    if (driverFile.is_open()) {
        while (std::getline(driverFile, line)) {
            if (!line.empty()) drivers.push_back(Driver::fromCSV(line));
        }
    }
    
    // Load seats
    seats.clear();
    for (int i = 1; i <= 100; i++) {
        std::string filename = "Data/Seat/B" + std::string(3 - std::to_string(i).length(), '0') + std::to_string(i) + ".txt";
        std::ifstream seatFile(filename);
        if (seatFile.is_open()) {
            while (std::getline(seatFile, line)) {
                if (!line.empty()) {
                    try {
                        seats.push_back(Seat::fromCSV(line));
                    } catch (...) {}
                }
            }
        }
    }
    
    // Populate tables
    populateBrandsTable();
    populateRoutesTable();
    populateBusesTable();
    populateDriversTable();
    // Ticket cache will be built when tickets are loaded lazily
    populateTripsTable();
    // Ticket UI population will occur after lazy load
    populateStatsTable();
}

void AdminWindow::loadTicketsData() {
    if (ticketsLoaded) return;
    // Load all tickets from Ticket folder, keeping per-file grouping for master-detail
    tickets.clear();
    ticketsByFile.clear();
    ticketFileOrder.clear();
    std::string line;
    for (int i = 1; i <= 100; i++) {
        std::string fileId = "TK" + std::string(3 - std::to_string(i).length(), '0') + std::to_string(i);
        std::string filename = "Data/Ticket/" + fileId + ".txt";
        if (!fs::exists(filename)) continue;

        std::ifstream ticketFile(filename);
        std::vector<Ticket> fileTickets;
        if (ticketFile.is_open()) {
            while (std::getline(ticketFile, line)) {
                if (!line.empty() && line.find("<<<<<<<") == std::string::npos && line.find(">>>>>>>") == std::string::npos && line.find("=======") == std::string::npos) {
                    try {
                        Ticket tk = Ticket::fromCSV(line);
                        tickets.push_back(tk);
                        fileTickets.push_back(tk);
                    } catch (...) {
                        // Skip invalid lines
                    }
                }
            }
        }
        ticketsByFile[fileId] = fileTickets;
        ticketFileOrder.push_back(fileId);
    }
    rebuildTicketCache();
    populateTicketFileSummary();
    populateTicketsTable();
    ticketsLoaded = true;
}

void AdminWindow::rebuildTicketCache() {
    ticketCache.clear();

    // Format money helper
    auto formatMoney = [](unsigned long amount) -> QString {
        QString str = QString::number(amount);
        int len = str.length();
        for (int i = len - 3; i > 0; i -= 3) {
            str.insert(i, ',');
        }
        return str;
    };

    // Create lookup maps for fast access
    std::map<std::string, const Trip*> tripMap;
    for (const auto& trip : trips) {
        tripMap[trip.getId()] = &trip;
    }

    std::map<std::string, const Route*> routeMap;
    for (const auto& route : routes) {
        routeMap[route.getId()] = &route;
    }

    std::map<std::string, const Bus*> busMap;
    for (const auto& bus : buses) {
        busMap[bus.getId()] = &bus;
    }

    // Precompute brand name per busId
    std::map<std::string, std::string> busBrandName;
    for (const auto& bus : buses) {
        for (const auto& brand : brands) {
            if (brand.getId() == bus.getBrandId()) {
                busBrandName[bus.getId()] = brand.getName();
                break;
            }
        }
        if (busBrandName.find(bus.getId()) == busBrandName.end()) {
            busBrandName[bus.getId()] = bus.getName();
        }
    }

    ticketCache.reserve(tickets.size());
    for (const auto& tk : tickets) {
        TicketRowCache row;
        row.id = QString::fromStdString(tk.getId());

        // Route name via trip
        row.routeName = QString::fromStdString(tk.getTripId());
        auto tIt = tripMap.find(tk.getTripId());
        if (tIt != tripMap.end()) {
            auto rIt = routeMap.find(tIt->second->getRouteId());
            if (rIt != routeMap.end()) {
                row.routeName = QString::fromStdString(rIt->second->getStart() + " - " + rIt->second->getEnd());
            }
        }

        // Bus brand name
        auto bbIt = busBrandName.find(tk.getBusId());
        row.busName = bbIt != busBrandName.end() ? QString::fromStdString(bbIt->second)
                                                 : QString::fromStdString(tk.getBusId());

        row.seat = QString::number(tk.getSeatNo());
        row.passenger = QString::fromStdString(tk.getPassengerName());
        row.phone = QString::fromStdString(tk.getPhoneNumber());
        row.price = formatMoney(tk.getPrice());
        row.bookedAt = QString::fromStdString(tk.getBookedAt());
        row.payment = QString::fromStdString(tk.getPaymentMethod());

        // Build a lowercased concatenated search key
        QString key = row.id + " " + row.routeName + " " + row.busName + " " + row.seat + " " +
                      row.passenger + " " + row.phone + " " + row.price + " " + row.bookedAt + " " + row.payment;
        row.searchKey = key.toLower();

        ticketCache.push_back(std::move(row));
    }
}

void AdminWindow::populateBrandsTable() {
    QTableWidget *table = findChild<QTableWidget*>("brandTable");
    if (!table) return;

    const QString term = brandSearch.trimmed().toLower();
    int row = 0;
    table->setRowCount(static_cast<int>(brands.size()));
    for (const auto& brand : brands) {
        const QString id = QString::fromStdString(brand.getId());
        const QString name = QString::fromStdString(brand.getName());
        const QString hotline = QString::fromStdString(brand.getHotline());
        const QString rating = QString::number(brand.getRating(), 'f', 1);

        const bool matches = term.isEmpty() ||
            id.toLower().contains(term) ||
            name.toLower().contains(term) ||
            hotline.toLower().contains(term) ||
            rating.toLower().contains(term);
        if (!matches) continue;

        auto idItem = new QTableWidgetItem(id);
        idItem->setData(Qt::UserRole, id); // Store actual ID
        table->setItem(row, 0, idItem);
        table->setItem(row, 1, new QTableWidgetItem(name));
        table->setItem(row, 2, new QTableWidgetItem(hotline));
        table->setItem(row, 3, new QTableWidgetItem(rating));
        ++row;
    }
    table->setRowCount(row);
}

void AdminWindow::populateRoutesTable() {
    QTableWidget *table = findChild<QTableWidget*>("routeTable");
    if (!table) return;

    const QString term = routeSearch.trimmed().toLower();
    int row = 0;
    table->setRowCount(static_cast<int>(routes.size()));
    for (const auto& route : routes) {
        const QString id = QString::fromStdString(route.getId());
        const QString name = QString::fromStdString(route.getName());
        const QString start = QString::fromStdString(route.getStart());
        const QString end = QString::fromStdString(route.getEnd());
        const QString distance = QString::fromStdString(route.getDistance());
        const QString duration = QString::fromStdString(route.getDuration());

        const bool matches = term.isEmpty() ||
            id.toLower().contains(term) ||
            name.toLower().contains(term) ||
            start.toLower().contains(term) ||
            end.toLower().contains(term) ||
            distance.toLower().contains(term) ||
            duration.toLower().contains(term);
        if (!matches) continue;

        auto idItem = new QTableWidgetItem(id);
        idItem->setData(Qt::UserRole, id);
        table->setItem(row, 0, idItem);
        table->setItem(row, 1, new QTableWidgetItem(name));
        table->setItem(row, 2, new QTableWidgetItem(start));
        table->setItem(row, 3, new QTableWidgetItem(end));
        table->setItem(row, 4, new QTableWidgetItem(distance));
        table->setItem(row, 5, new QTableWidgetItem(duration));
        ++row;
    }
    table->setRowCount(row);
}

void AdminWindow::populateBusesTable() {
    QTableWidget *table = findChild<QTableWidget*>("busTable");
    if (!table) return;

    const QString term = busSearch.trimmed().toLower();
    int row = 0;
    table->setRowCount(static_cast<int>(buses.size()));
    for (const auto& bus : buses) {
        const QString id = QString::fromStdString(bus.getId());
        const QString name = QString::fromStdString(bus.getName());

        QString brandName = QString::fromStdString(bus.getBrandId());
        for (const auto& brand : brands) {
            if (brand.getId() == bus.getBrandId()) {
                brandName = QString::fromStdString(brand.getName());
                break;
            }
        }

        const QString type = QString::fromStdString(bus.getType());
        const QString capacity = QString::number(bus.getCapacity());

        const bool matches = term.isEmpty() ||
            id.toLower().contains(term) ||
            name.toLower().contains(term) ||
            brandName.toLower().contains(term) ||
            type.toLower().contains(term) ||
            capacity.toLower().contains(term);
        if (!matches) continue;

        auto idItem = new QTableWidgetItem(id);
        idItem->setData(Qt::UserRole, id);
        table->setItem(row, 0, idItem);
        table->setItem(row, 1, new QTableWidgetItem(name));
        table->setItem(row, 2, new QTableWidgetItem(brandName));
        table->setItem(row, 3, new QTableWidgetItem(type));
        table->setItem(row, 4, new QTableWidgetItem(capacity));
        ++row;
    }
    table->setRowCount(row);
}

void AdminWindow::populateDriversTable() {
    QTableWidget *table = findChild<QTableWidget*>("driverTable");
    if (!table) return;
    
    // Build lookup: busId -> busName for display
    std::map<std::string, std::string> busNameMap;
    for (const auto& bus : buses) {
        busNameMap[bus.getId()] = bus.getName();
    }
    
    const QString term = driverSearch.trimmed().toLower();
    int row = 0;
    table->setRowCount(static_cast<int>(drivers.size()));
    for (const auto& driver : drivers) {
        const QString id = QString::fromStdString(driver.getId());
        const QString name = QString::fromStdString(driver.getName());
        const auto& busId = driver.getBus();
        auto it = busNameMap.find(busId);
        QString busDisplay = it != busNameMap.end() ? QString::fromStdString(it->second)
                                                   : QString::fromStdString(busId);
        const QString phone = QString::fromStdString(driver.getPhone());
        const QString exp = QString::number(driver.getExp());

        const bool matches = term.isEmpty() ||
            id.toLower().contains(term) ||
            name.toLower().contains(term) ||
            busDisplay.toLower().contains(term) ||
            phone.toLower().contains(term) ||
            exp.toLower().contains(term);
        if (!matches) continue;

        auto idItem = new QTableWidgetItem(id);
        idItem->setData(Qt::UserRole, id);
        table->setItem(row, 0, idItem);
        table->setItem(row, 1, new QTableWidgetItem(name));
        table->setItem(row, 2, new QTableWidgetItem(busDisplay));
        table->setItem(row, 3, new QTableWidgetItem(phone));
        table->setItem(row, 4, new QTableWidgetItem(exp));
        ++row;
    }
    table->setRowCount(row);
}

void AdminWindow::populateSeatsTable() {
    QTableWidget *table = findChild<QTableWidget*>("seatTable");
    if (!table) return;
    
    // Display buses instead of seats
    table->setRowCount(buses.size());
    for (size_t i = 0; i < buses.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(buses[i].getId())));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(buses[i].getName())));
        
        // Find brand name
        QString brandName = QString::fromStdString(buses[i].getBrandId());
        for (const auto& brand : brands) {
            if (brand.getId() == buses[i].getBrandId()) {
                brandName = QString::fromStdString(brand.getName());
                break;
            }
        }
        table->setItem(i, 2, new QTableWidgetItem(brandName));
        
        table->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(buses[i].getType())));
        table->setItem(i, 4, new QTableWidgetItem(QString::number(buses[i].getCapacity())));
    }
}

void AdminWindow::populateTripsTable() {
    QTableWidget *table = findChild<QTableWidget*>("tripTable");
    if (!table) return;
    
    // Create lookup maps for fast access
    std::map<std::string, const Route*> routeMap;
    for (const auto& route : routes) {
        routeMap[route.getId()] = &route;
    }
    
    std::map<std::string, std::string> busNameMap;
    for (const auto& bus : buses) {
        for (const auto& brand : brands) {
            if (brand.getId() == bus.getBrandId()) {
                busNameMap[bus.getId()] = brand.getName();
                break;
            }
        }
    }
    
    std::map<std::string, const Driver*> driverMap;
    for (const auto& driver : drivers) {
        driverMap[driver.getId()] = &driver;
    }
    
    const QString term = tripSearch.trimmed().toLower();
    int row = 0;
    table->setRowCount(static_cast<int>(trips.size()));
    for (const auto& trip : trips) {
        const QString id = QString::fromStdString(trip.getId());
        
        // Find route name using map
        QString routeName = QString::fromStdString(trip.getRouteId());
        auto routeIt = routeMap.find(trip.getRouteId());
        if (routeIt != routeMap.end()) {
            routeName = QString::fromStdString(routeIt->second->getStart() + " - " + routeIt->second->getEnd());
        }
        
        // Find bus name using map
        QString busName = QString::fromStdString(trip.getBusId());
        auto busIt = busNameMap.find(trip.getBusId());
        if (busIt != busNameMap.end()) {
            busName = QString::fromStdString(busIt->second);
        }
        
        // Find driver name using map
        QString driverName = QString::fromStdString(trip.getDriverId());
        auto driverIt = driverMap.find(trip.getDriverId());
        if (driverIt != driverMap.end()) {
            driverName = QString::fromStdString(driverIt->second->getName());
        }
        const QString depart = QString::fromStdString(trip.getDepart());
        const QString arrival = QString::fromStdString(trip.getArrival());

        const bool matches = term.isEmpty() ||
            id.toLower().contains(term) ||
            routeName.toLower().contains(term) ||
            busName.toLower().contains(term) ||
            driverName.toLower().contains(term) ||
            depart.toLower().contains(term) ||
            arrival.toLower().contains(term);
        if (!matches) continue;

        auto idItem = new QTableWidgetItem(id);
        idItem->setData(Qt::UserRole, id);
        table->setItem(row, 0, idItem);
        table->setItem(row, 1, new QTableWidgetItem(routeName));
        table->setItem(row, 2, new QTableWidgetItem(busName));
        table->setItem(row, 3, new QTableWidgetItem(driverName));
        table->setItem(row, 4, new QTableWidgetItem(depart));
        table->setItem(row, 5, new QTableWidgetItem(arrival));
        ++row;
    }
    table->setRowCount(row);
}

void AdminWindow::populateTicketsTable() {
    QTableWidget *table = findChild<QTableWidget*>("ticketDetailTable");
    if (!table) return;
    const QString term = ticketSearch.trimmed().toLower();

    // When searching, show across all tickets
    if (!term.isEmpty()) {
        int row = 0;
        table->setRowCount(static_cast<int>(ticketCache.size()));
        for (const auto& r : ticketCache) {
            if (!r.searchKey.contains(term)) continue;
            table->setItem(row, 0, new QTableWidgetItem(r.id));
            table->setItem(row, 1, new QTableWidgetItem(r.routeName));
            table->setItem(row, 2, new QTableWidgetItem(r.seat));
            table->setItem(row, 3, new QTableWidgetItem(r.passenger));
            table->setItem(row, 4, new QTableWidgetItem(r.phone));
            table->setItem(row, 5, new QTableWidgetItem(r.price));
            table->setItem(row, 6, new QTableWidgetItem(r.bookedAt));
            table->setItem(row, 7, new QTableWidgetItem(r.payment));
            ++row;
        }
        table->setRowCount(row);
        return;
    }

    // No search term: show tickets for selected file
    populateTicketsForFile(selectedTicketFile);
}

void AdminWindow::populateTicketFileSummary() {
    QTableWidget *fileTable = findChild<QTableWidget*>("ticketFileTable");
    if (!fileTable) return;

    fileTable->setRowCount(static_cast<int>(ticketFileOrder.size()));
    int row = 0;

    for (const auto& fileId : ticketFileOrder) {
        const auto it = ticketsByFile.find(fileId);
        const std::vector<Ticket> emptyVec;
        const std::vector<Ticket>& fileTickets = (it != ticketsByFile.end()) ? it->second : emptyVec;

        // Determine bus name: prefer first ticket's bus id -> bus name; fallback to trip mapping by file number
        QString busName = "(no tickets)";
        std::string busId;
        if (!fileTickets.empty()) {
            busId = fileTickets.front().getBusId();
        } else {
            // Try map file TKxxx -> trip Txxx -> busId
            if (fileId.size() >= 5) {
                std::string tripId = "T" + fileId.substr(2);
                for (const auto& trip : trips) {
                    if (trip.getId() == tripId) {
                        busId = trip.getBusId();
                        break;
                    }
                }
            }
        }

        if (!busId.empty()) {
            for (const auto& bus : buses) {
                if (bus.getId() == busId) {
                    busName = QString::fromStdString(bus.getName());
                    break;
                }
            }
            if (busName == "(chưa có vé)") {
                busName = QString::fromStdString(busId);
            }
        }

        auto itemFile = new QTableWidgetItem(QString::fromStdString(fileId));
        itemFile->setData(Qt::UserRole, QString::fromStdString(fileId));
        fileTable->setItem(row, 0, itemFile);
        fileTable->setItem(row, 1, new QTableWidgetItem(busName));
        fileTable->setItem(row, 2, new QTableWidgetItem(QString::number(static_cast<int>(fileTickets.size()))));
        ++row;
    }

    if (fileTable->rowCount() > 0) {
        fileTable->selectRow(0);
        QTableWidgetItem *first = fileTable->item(0, 0);
        if (first) {
            selectedTicketFile = first->data(Qt::UserRole).toString().toStdString();
            populateTicketsForFile(selectedTicketFile);
        }
    } else {
        selectedTicketFile.clear();
        QTableWidget *detail = findChild<QTableWidget*>("ticketDetailTable");
        if (detail) detail->setRowCount(0);
    }
}

void AdminWindow::populateTicketsForFile(const std::string& fileId) {
    QTableWidget *table = findChild<QTableWidget*>("ticketDetailTable");
    if (!table) return;
    selectedTicketFile = fileId;

    auto it = ticketsByFile.find(fileId);
    if (it == ticketsByFile.end()) {
        table->setRowCount(0);
        return;
    }

    const auto& fileTickets = it->second;
    table->setRowCount(static_cast<int>(fileTickets.size()));

    // Build lookups once
    std::map<std::string, const Trip*> tripMap;
    for (const auto& trip : trips) tripMap[trip.getId()] = &trip;
    std::map<std::string, const Route*> routeMap;
    for (const auto& route : routes) routeMap[route.getId()] = &route;
    auto fmtMoney = [](unsigned long amount){ QString s = QString::number(amount); for (int i=s.length()-3;i>0;i-=3) s.insert(i, ','); return s; };

    int row = 0;
    for (const auto& tk : fileTickets) {
        table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(tk.getId())));
        QString routeName = QString::fromStdString(tk.getTripId());
        auto tIt = tripMap.find(tk.getTripId());
        if (tIt != tripMap.end()) {
            auto rIt = routeMap.find(tIt->second->getRouteId());
            if (rIt != routeMap.end()) routeName = QString::fromStdString(rIt->second->getStart() + " - " + rIt->second->getEnd());
        }
        table->setItem(row, 1, new QTableWidgetItem(routeName));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(tk.getSeatNo())));
        table->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(tk.getPassengerName())));
        table->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(tk.getPhoneNumber())));
        table->setItem(row, 5, new QTableWidgetItem(fmtMoney(tk.getPrice())));
        table->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(tk.getBookedAt())));
        table->setItem(row, 7, new QTableWidgetItem(QString::fromStdString(tk.getPaymentMethod())));
        ++row;
    }
    table->setRowCount(row);
}

void AdminWindow::populateStatsTable() {
    populateStatsTableWithFilter(tickets);
}

void AdminWindow::populateStatsTableWithFilter(const std::vector<Ticket>& filteredTickets) {
    QTableWidget *table = findChild<QTableWidget*>("statsTable");
    if (!table) return;
    
    // Calculate total revenue and average
    unsigned long totalRevenue = 0;
    for (const auto& ticket : filteredTickets) {
        totalRevenue += ticket.getPrice();
    }
    double avgPrice = filteredTickets.empty() ? 0 : (double)totalRevenue / filteredTickets.size();
    
    // Format number with thousands separator
    auto formatMoney = [](unsigned long amount) -> QString {
        QString str = QString::number(amount);
        int len = str.length();
        for (int i = len - 3; i > 0; i -= 3) {
            str.insert(i, ',');
        }
        return str;
    };
    
    // Update summary labels
    QLabel *lblTotalTickets = findChild<QLabel*>("lblTotalTickets");
    QLabel *lblTotalRevenue = findChild<QLabel*>("lblTotalRevenue");
    QLabel *lblAvgPrice = findChild<QLabel*>("lblAvgPrice");
    
    if (lblTotalTickets) {
        lblTotalTickets->setText(QString("Total Tickets: %1").arg(filteredTickets.size()));
    }
    if (lblTotalRevenue) {
        lblTotalRevenue->setText(QString("Total Revenue: %1 VND").arg(formatMoney(totalRevenue)));
    }
    if (lblAvgPrice) {
        lblAvgPrice->setText(QString("Average: %1 VND").arg(formatMoney((unsigned long)avgPrice)));
    }
    
    // Display filtered tickets in stats table
    table->setRowCount(filteredTickets.size());
    for (size_t i = 0; i < filteredTickets.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(filteredTickets[i].getId())));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(filteredTickets[i].getPassengerName())));
        
        // Find route name from trip (Start - End format)
        QString routeName = QString::fromStdString(filteredTickets[i].getTripId());
        for (const auto& trip : trips) {
            if (trip.getId() == filteredTickets[i].getTripId()) {
                for (const auto& route : routes) {
                    if (route.getId() == trip.getRouteId()) {
                        routeName = QString::fromStdString(route.getStart() + " - " + route.getEnd());
                        break;
                    }
                }
                break;
            }
        }
        
        table->setItem(i, 2, new QTableWidgetItem(routeName));
        
        // Find bus brand name
        QString busName = QString::fromStdString(filteredTickets[i].getBusId());
        for (const auto& bus : buses) {
            if (bus.getId() == filteredTickets[i].getBusId()) {
                for (const auto& brand : brands) {
                    if (brand.getId() == bus.getBrandId()) {
                        busName = QString::fromStdString(brand.getName());
                        break;
                    }
                }
                break;
            }
        }
        table->setItem(i, 3, new QTableWidgetItem(busName));
        table->setItem(i, 4, new QTableWidgetItem(QString::number(filteredTickets[i].getSeatNo())));
        table->setItem(i, 5, new QTableWidgetItem(formatMoney(filteredTickets[i].getPrice())));
        table->setItem(i, 6, new QTableWidgetItem(QString::fromStdString(filteredTickets[i].getBookedAt())));
        table->setItem(i, 7, new QTableWidgetItem(QString::fromStdString(filteredTickets[i].getPaymentMethod())));
    }
}

void AdminWindow::onFilterByDay() {
    QDialog dialog(this);
    dialog.setWindowTitle("Filter by Day");
    dialog.setStyleSheet("background: #0f172a; color: #e5e7eb;");
    dialog.setMinimumWidth(300);
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QLabel *label = new QLabel("Select Day:", &dialog);
    label->setStyleSheet("font-size: 14px; color: #e5e7eb;");
    
    QDateEdit *dateEdit = new QDateEdit(&dialog);
    dateEdit->setCalendarPopup(true);
    dateEdit->setDate(QDate::currentDate());
    dateEdit->setStyleSheet("QDateEdit { background: #1e293b; color: #e5e7eb; border: 1px solid #334155; padding: 8px; border-radius: 6px; } QDateEdit::drop-down { border: none; }");
    
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    btnBox->setStyleSheet("QPushButton { background: #2563eb; color: white; padding: 8px 16px; border-radius: 6px; } QPushButton:hover { background: #1d4ed8; }");
    connect(btnBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    layout->addWidget(label);
    layout->addWidget(dateEdit);
    layout->addWidget(btnBox);
    
    if (dialog.exec() != QDialog::Accepted) return;
    
    QString dateStr = dateEdit->date().toString("yyyy-MM-dd");
    
    std::vector<Ticket> filtered;
    for (const auto& tk : tickets) {
        QString bookedDate = QString::fromStdString(tk.getBookedAt()).left(10);
        if (bookedDate == dateStr) {
            filtered.push_back(tk);
        }
    }
    
    if (filtered.empty()) {
        QMessageBox::information(this, "Result", "No tickets found for this day.");
        return;
    }
    
    populateStatsTableWithFilter(filtered);
}

void AdminWindow::onFilterByMonth() {
    QDialog dialog(this);
    dialog.setWindowTitle("Filter by Month");
    dialog.setStyleSheet("background: #0f172a; color: #e5e7eb;");
    dialog.setMinimumWidth(300);
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    QLabel *lblMonth = new QLabel("Select Month:", &dialog);
    lblMonth->setStyleSheet("font-size: 14px; color: #e5e7eb;");
    QComboBox *cmbMonth = new QComboBox(&dialog);
    cmbMonth->setStyleSheet("QComboBox { background: #1e293b; color: #e5e7eb; border: 1px solid #334155; padding: 8px; border-radius: 6px; }");
    for (int i = 1; i <= 12; i++) {
        cmbMonth->addItem(QString("Month %1").arg(i, 2, 10, QChar('0')), i);
    }
    cmbMonth->setCurrentIndex(QDate::currentDate().month() - 1);
    
    QLabel *lblYear = new QLabel("Select Year:", &dialog);
    lblYear->setStyleSheet("font-size: 14px; color: #e5e7eb; margin-top: 10px;");
    QSpinBox *spinYear = new QSpinBox(&dialog);
    spinYear->setStyleSheet("QSpinBox { background: #1e293b; color: #e5e7eb; border: 1px solid #334155; padding: 8px; border-radius: 6px; }");
    spinYear->setRange(2020, 2030);
    spinYear->setValue(QDate::currentDate().year());
    
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    btnBox->setStyleSheet("QPushButton { background: #2563eb; color: white; padding: 8px 16px; border-radius: 6px; } QPushButton:hover { background: #1d4ed8; }");
    connect(btnBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    layout->addWidget(lblMonth);
    layout->addWidget(cmbMonth);
    layout->addWidget(lblYear);
    layout->addWidget(spinYear);
    layout->addWidget(btnBox);
    
    if (dialog.exec() != QDialog::Accepted) return;
    
    QString monthStr = QString("%1-%2")
        .arg(spinYear->value())
        .arg(cmbMonth->currentData().toInt(), 2, 10, QChar('0'));
    
    std::vector<Ticket> filtered;
    for (const auto& tk : tickets) {
        QString bookedMonth = QString::fromStdString(tk.getBookedAt()).left(7);
        if (bookedMonth == monthStr) {
            filtered.push_back(tk);
        }
    }
    
    if (filtered.empty()) {
        QMessageBox::information(this, "Result", "No tickets found for this month.");
        return;
    }
    
    populateStatsTableWithFilter(filtered);
}

void AdminWindow::onFilterByYear() {
    QDialog dialog(this);
    dialog.setWindowTitle("Filter by Year");
    dialog.setStyleSheet("background: #0f172a; color: #e5e7eb;");
    dialog.setMinimumWidth(300);
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    QLabel *label = new QLabel("Select Year:", &dialog);
    label->setStyleSheet("font-size: 14px; color: #e5e7eb;");
    
    QSpinBox *spinYear = new QSpinBox(&dialog);
    spinYear->setStyleSheet("QSpinBox { background: #1e293b; color: #e5e7eb; border: 1px solid #334155; padding: 8px; border-radius: 6px; }");
    spinYear->setRange(2020, 2030);
    spinYear->setValue(QDate::currentDate().year());
    
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    btnBox->setStyleSheet("QPushButton { background: #2563eb; color: white; padding: 8px 16px; border-radius: 6px; } QPushButton:hover { background: #1d4ed8; }");
    connect(btnBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    layout->addWidget(label);
    layout->addWidget(spinYear);
    layout->addWidget(btnBox);
    
    if (dialog.exec() != QDialog::Accepted) return;
    
    QString yearStr = QString::number(spinYear->value());
    
    std::vector<Ticket> filtered;
    for (const auto& tk : tickets) {
        QString bookedYear = QString::fromStdString(tk.getBookedAt()).left(4);
        if (bookedYear == yearStr) {
            filtered.push_back(tk);
        }
    }
    
    if (filtered.empty()) {
        QMessageBox::information(this, "Result", "No tickets found for this year.");
        return;
    }
    
    populateStatsTableWithFilter(filtered);
}

void AdminWindow::onShowAllStats() {
    populateStatsTable();
}

void AdminWindow::onLogoutClicked() {
    emit logout();
}

void AdminWindow::onRefreshClicked() {
    // If currently on tickets tab, refresh tickets lazily; otherwise reload all
    int idx = tabWidget ? tabWidget->currentIndex() : -1;
    if (idx == ticketsTabIndex) {
        ticketsLoaded = false;
        loadTicketsData();
    } else {
        loadAllData();
    }
}

// ---------- ROUTE CRUD ----------
void AdminWindow::onAddRouteClicked() {
    RouteDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Route input = dlg.getRoute();
        
        if (input.getName().empty() || input.getDistance().empty() || input.getDuration().empty()) {
            QMessageBox::warning(this, "Input Error", "Please enter all route information (Name, Distance, Duration)!");
            return;
        }

        std::string lastId = routes.empty() ? "R000" : routes.back().getId();
        int next = 0; try { next = std::stoi(lastId.substr(1)) + 1; } catch(...) {}
        std::stringstream ss; ss << "R" << std::setw(3) << std::setfill('0') << next;
        std::string newId = ss.str();

        std::string start = input.getStart();
        std::string end = input.getEnd();
        if (!input.getName().empty()) {
            size_t pos = input.getName().find(" - ");
            if (pos != std::string::npos) {
                start = input.getName().substr(0, pos);
                end = input.getName().substr(pos + 3);
            }
        }

        Route r(newId, input.getName(), start, end, input.getDistance(), input.getDuration());
        routes.push_back(r);
        Ultil<Route>::saveToFile("Data/Route.txt", routes);
        
        routeSearch = ""; 
        QLineEdit *txtSearch = findChild<QLineEdit*>(); 
        if(txtSearch) txtSearch->clear();
        
        populateRoutesTable();
        QMessageBox::information(this, "Success", "Route added successfully!");
    }
}

void AdminWindow::onEditRouteClicked() {
    QTableWidget *table = findChild<QTableWidget*>("routeTable");
    if (!table) return;
    auto items = table->selectedItems();
    if (items.empty()) return;
    int row = table->row(items.first());
    QString routeId = table->item(row, 0)->data(Qt::UserRole).toString();
    
    auto it = std::find_if(routes.begin(), routes.end(),
        [&](const Route& r){ return r.getId() == routeId.toStdString(); });
    if (it == routes.end()) return;
    
    Route selected = *it;
    RouteDialog dlg(this, &selected);
    if (dlg.exec() == QDialog::Accepted) {
        Route updated = dlg.getRoute();
        std::string start = updated.getStart();
        std::string end = updated.getEnd();
        if (!updated.getName().empty()) {
            size_t pos = updated.getName().find(" - ");
            if (pos != std::string::npos) {
                start = updated.getName().substr(0, pos);
                end = updated.getName().substr(pos + 3);
            }
        }
        *it = Route(selected.getId(), updated.getName(), start, end, updated.getDistance(), updated.getDuration());
        Ultil<Route>::saveToFile("Data/Route.txt", routes);
        populateRoutesTable();
    }
}

void AdminWindow::onDeleteRouteClicked() {
    QTableWidget *table = findChild<QTableWidget*>("routeTable");
    if (!table) return;
    auto items = table->selectedItems();
    if (items.empty()) return;
    int row = table->row(items.first());
    QString routeId = table->item(row, 0)->data(Qt::UserRole).toString();
    
    auto reply = QMessageBox::question(this, "Confirm Delete", "Delete selected route?");
    if (reply == QMessageBox::Yes) {
        routes.erase(std::remove_if(routes.begin(), routes.end(),
            [&](const Route& r){ return r.getId() == routeId.toStdString(); }), routes.end());
        Ultil<Route>::saveToFile("Data/Route.txt", routes);
        populateRoutesTable();
    }
}

// ---------- BUS CRUD ----------
void AdminWindow::onAddBusClicked() {
    BusDialog dlg(this, brands);
    if (dlg.exec() == QDialog::Accepted) {
        Bus bInput = dlg.getBus();
        // Generate IDs
        std::string lastBusId = buses.empty() ? "B000" : buses.back().getId();
        int nextB = 0; try { nextB = std::stoi(lastBusId.substr(1)) + 1; } catch(...) {}
        std::stringstream sb; sb << "B" << std::setw(3) << std::setfill('0') << nextB;
        std::string newBusId = sb.str();

        Bus b(newBusId, bInput.getBrandId(), bInput.getName(), bInput.getType(), bInput.getCapacity());

        // Add driver if provided
        if (dlg.hasDriver()) {
            Driver dInput = dlg.getDriver();
            std::string lastDrvId = drivers.empty() ? "D000" : drivers.back().getId();
            int nextD = 0; try { nextD = std::stoi(lastDrvId.substr(1)) + 1; } catch(...) {}
            std::stringstream sd; sd << "D" << std::setw(3) << std::setfill('0') << nextD;
            std::string newDrvId = sd.str();
            Driver d(newDrvId, b.getId(), dInput.getName(), dInput.getPhone(), dInput.getExp());
            drivers.push_back(d);
            Ultil<Driver>::saveToFile("Data/Driver.txt", drivers);
        }
        
        // Create seats
        std::string sPath = "Data/Seat/" + b.getId() + ".txt";
        std::ofstream f(sPath);
        if (f.is_open()) {
            for(int i=1; i<=b.getCapacity(); ++i) {
                f << b.getId() << "," << i << ",0\n";
                seats.push_back(Seat(b.getId(), i, false));
            }
            f.close();
        }
        
        buses.push_back(b);
        Ultil<Bus>::saveToFile("Data/Bus.txt", buses);
        populateBusesTable();
    }
}

void AdminWindow::onEditBusClicked() {
    QTableWidget *table = findChild<QTableWidget*>("busTable");
    if (!table) return;
    auto items = table->selectedItems();
    if (items.empty()) return;
    int row = table->row(items.first());
    QString busId = table->item(row, 0)->data(Qt::UserRole).toString();
    
    auto it = std::find_if(buses.begin(), buses.end(),
        [&](const Bus& b){ return b.getId() == busId.toStdString(); });
    if (it == buses.end()) return;
    
    Bus selected = *it;
    BusDialog dlg(this, brands, &selected);
    if (dlg.exec() == QDialog::Accepted) {
        Bus updated = dlg.getBus();
        *it = Bus(selected.getId(), updated.getBrandId(), updated.getName(), updated.getType(), updated.getCapacity());
        Ultil<Bus>::saveToFile("Data/Bus.txt", buses);
        populateBusesTable();
    }
}

void AdminWindow::onDeleteBusClicked() {
    QTableWidget *table = findChild<QTableWidget*>("busTable");
    if (!table) return;
    auto items = table->selectedItems();
    if (items.empty()) return;
    int row = table->row(items.first());
    QString busId = table->item(row, 0)->data(Qt::UserRole).toString();
    
    auto reply = QMessageBox::question(this, "Confirm Delete", "Delete selected bus? This will remove drivers and seats.");
    if (reply == QMessageBox::Yes) {
        std::string targetId = busId.toStdString();
        
        // Remove Bus
        buses.erase(std::remove_if(buses.begin(), buses.end(),
            [&](const Bus& b){ return b.getId() == targetId; }), buses.end());
        Ultil<Bus>::saveToFile("Data/Bus.txt", buses);
        
        // Remove Drivers for this bus
        auto itD = std::remove_if(drivers.begin(), drivers.end(), [&](const Driver& d){ return d.getBus() == targetId; });
        drivers.erase(itD, drivers.end());
        Ultil<Driver>::saveToFile("Data/Driver.txt", drivers);
        
        // Remove seats in memory and file
        auto itS = std::remove_if(seats.begin(), seats.end(), [&](const Seat& s){ return s.getBusId() == targetId; });
        seats.erase(itS, seats.end());
        std::string sPath = "Data/Seat/" + targetId + ".txt";
        if (fs::exists(sPath)) fs::remove(sPath);
        
        populateBusesTable();
    }
}

// ---------- TRIP CRUD ----------
void AdminWindow::onAddTripClicked() {
    TripDialog dlg(this, routes, buses, drivers, trips);
    if (dlg.exec() == QDialog::Accepted) {
        Trip tInput = dlg.getTrip();
        // Generate ID
        std::string lastId = trips.empty() ? "T000" : trips.back().getId();
        int next = 0; try { next = std::stoi(lastId.substr(1)) + 1; } catch(...) {}
        std::stringstream ss; ss << "T" << std::setw(3) << std::setfill('0') << next;
        std::string newId = ss.str();
        Trip t(newId, tInput.getRouteId(), tInput.getBusId(), tInput.getDriverId(), tInput.getDepart(), tInput.getArrival());
        trips.push_back(t);
        Ultil<Trip>::saveToFile("Data/Trip.txt", trips);
        populateTripsTable();
    }
}

void AdminWindow::onEditTripClicked() {
    QTableWidget *table = findChild<QTableWidget*>("tripTable");
    if (!table) return;
    auto items = table->selectedItems();
    if (items.empty()) return;
    int row = table->row(items.first());
    QString tripId = table->item(row, 0)->data(Qt::UserRole).toString();
    
    auto it = std::find_if(trips.begin(), trips.end(),
        [&](const Trip& t){ return t.getId() == tripId.toStdString(); });
    if (it == trips.end()) return;
    
    Trip selected = *it;
    TripDialog dlg(this, routes, buses, drivers, trips, &selected);
    if (dlg.exec() == QDialog::Accepted) {
        Trip updated = dlg.getTrip();
        *it = Trip(selected.getId(), updated.getRouteId(), updated.getBusId(), updated.getDriverId(), updated.getDepart(), updated.getArrival());
        Ultil<Trip>::saveToFile("Data/Trip.txt", trips);
        populateTripsTable();
    }
}

void AdminWindow::onDeleteTripClicked() {
    QTableWidget *table = findChild<QTableWidget*>("tripTable");
    if (!table) return;
    auto items = table->selectedItems();
    if (items.empty()) return;
    int row = table->row(items.first());
    QString tripId = table->item(row, 0)->data(Qt::UserRole).toString();
    
    auto reply = QMessageBox::question(this, "Confirm Delete", "Delete selected trip?");
    if (reply == QMessageBox::Yes) {
        trips.erase(std::remove_if(trips.begin(), trips.end(),
            [&](const Trip& t){ return t.getId() == tripId.toStdString(); }), trips.end());
        Ultil<Trip>::saveToFile("Data/Trip.txt", trips);
        populateTripsTable();
    }
}

// ---------- BRAND CRUD ----------
void AdminWindow::onAddBrandClicked() {
    bool ok;
    QString name = QInputDialog::getText(this, "Add brand", "Brand name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;
    
    QString hotline = QInputDialog::getText(this, "Add brand", "Hotline:", QLineEdit::Normal, "", &ok);
    if (!ok || hotline.isEmpty()) return;
    
    double rating = QInputDialog::getDouble(this, "Add brand", "Rating (0.0-5.0):", 4.0, 0.0, 5.0, 1, &ok);
    if (!ok) return;
    
    // Generate new ID
    int maxId = 0;
    for (const auto& b : brands) {
        string id = b.getId();
        if (id.length() > 2 && id.substr(0, 2) == "BR") {
            int num = stoi(id.substr(2));
            if (num > maxId) maxId = num;
        }
    }
    string newId = "BR" + string(3 - to_string(maxId + 1).length(), '0') + to_string(maxId + 1);
    
    Brand newBrand(newId, name.toStdString(), hotline.toStdString(), rating);
    brands.push_back(newBrand);
    Ultil<Brand>::saveToFile("Data/Brand.txt", brands);
    populateBrandsTable();
}

void AdminWindow::onEditBrandClicked() {
    QTableWidget *table = findChild<QTableWidget*>("brandTable");
    if (!table) return;
    auto items = table->selectedItems();
    if (items.empty()) return;
    int row = table->row(items.first());
    QString brandId = table->item(row, 0)->data(Qt::UserRole).toString();
    
    // Find brand by ID
    auto it = std::find_if(brands.begin(), brands.end(), 
        [&](const Brand& b){ return b.getId() == brandId.toStdString(); });
    if (it == brands.end()) return;
    
    Brand& b = *it;
    bool ok;
    QString name = QInputDialog::getText(this, "Edit brand", "Brand name:", QLineEdit::Normal, QString::fromStdString(b.getName()), &ok);
    if (ok && !name.isEmpty()) b.setName(name.toStdString());
    
    QString hotline = QInputDialog::getText(this, "Edit brand", "Hotline:", QLineEdit::Normal, QString::fromStdString(b.getHotline()), &ok);
    if (ok && !hotline.isEmpty()) b.setHotline(hotline.toStdString());
    
    double rating = QInputDialog::getDouble(this, "Edit brand", "Rating (0.0-5.0):", b.getRating(), 0.0, 5.0, 1, &ok);
    if (ok) b.setRating(rating);
    
    Ultil<Brand>::saveToFile("Data/Brand.txt", brands);
    populateBrandsTable();
}

void AdminWindow::onDeleteBrandClicked() {
    QTableWidget *table = findChild<QTableWidget*>("brandTable");
    if (!table) return;
    auto items = table->selectedItems();
    if (items.empty()) return;
    int row = table->row(items.first());
    QString brandId = table->item(row, 0)->data(Qt::UserRole).toString();
    
    auto reply = QMessageBox::question(this, "Confirm", "Delete this brand?");
    if (reply == QMessageBox::Yes) {
        brands.erase(std::remove_if(brands.begin(), brands.end(),
            [&](const Brand& b){ return b.getId() == brandId.toStdString(); }), brands.end());
        Ultil<Brand>::saveToFile("Data/Brand.txt", brands);
        populateBrandsTable();
    }
}

// ---------- DRIVER CRUD ----------
void AdminWindow::onAddDriverClicked() {
    if (buses.empty()) {
        QMessageBox::warning(this, "Error", "No buses available! Please add a bus first.");
        return;
    }
    
    bool ok;
    QStringList busIds;
    for (const auto& b : buses) busIds << QString::fromStdString(b.getId() + " - " + b.getName());
    QString busChoice = QInputDialog::getItem(this, "Select bus", "Bus:", busIds, 0, false, &ok);
    if (!ok) return;
    QString busId = busChoice.split(" - ").first();
    
    QString name = QInputDialog::getText(this, "Add driver", "Name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;
    
    QString phone = QInputDialog::getText(this, "Add driver", "Phone:", QLineEdit::Normal, "", &ok);
    if (!ok || phone.isEmpty()) return;
    
    int exp = QInputDialog::getInt(this, "Add driver", "Experience (years):", 0, 0, 50, 1, &ok);
    if (!ok) return;
    
    std::string lastId = drivers.empty() ? "D000" : drivers.back().getId();
    int next = 0;
    try { next = std::stoi(lastId.substr(1)) + 1; } catch(...) {}
    std::stringstream ss; 
    ss << "D" << std::setw(3) << std::setfill('0') << next;
    
    Driver d(ss.str(), busId.toStdString(), name.toStdString(), phone.toStdString(), exp);
    drivers.push_back(d);
    Ultil<Driver>::saveToFile("Data/Driver.txt", drivers);
    populateDriversTable();
}

void AdminWindow::onEditDriverClicked() {
    QTableWidget *table = findChild<QTableWidget*>("driverTable");
    if (!table) return;
    auto items = table->selectedItems();
    if (items.empty()) return;
    int row = table->row(items.first());
    QString driverId = table->item(row, 0)->data(Qt::UserRole).toString();
    
    auto it = std::find_if(drivers.begin(), drivers.end(),
        [&](const Driver& d){ return d.getId() == driverId.toStdString(); });
    if (it == drivers.end()) return;
    
    Driver& d = *it;
    bool ok;
    QString name = QInputDialog::getText(this, "Edit driver", "Name:", QLineEdit::Normal, QString::fromStdString(d.getName()), &ok);
    if (ok && !name.isEmpty()) d.setName(name.toStdString());
    
    QString phone = QInputDialog::getText(this, "Edit driver", "Phone:", QLineEdit::Normal, QString::fromStdString(d.getPhone()), &ok);
    if (ok && !phone.isEmpty()) d.setPhone(phone.toStdString());
    
    int exp = QInputDialog::getInt(this, "Driver's advice", "Experience (years):", d.getExp(), 0, 50, 1, &ok);
    if (ok) d.setExp(exp);
    
    Ultil<Driver>::saveToFile("Data/Driver.txt", drivers);
    populateDriversTable();
}

void AdminWindow::onDeleteDriverClicked() {
    QTableWidget *table = findChild<QTableWidget*>("driverTable");
    if (!table) return;
    auto items = table->selectedItems();
    if (items.empty()) return;
    int row = table->row(items.first());
    QString driverId = table->item(row, 0)->data(Qt::UserRole).toString();
    
    auto reply = QMessageBox::question(this, "Confirm", "Remove this driver?");
    if (reply == QMessageBox::Yes) {
        drivers.erase(std::remove_if(drivers.begin(), drivers.end(),
            [&](const Driver& d){ return d.getId() == driverId.toStdString(); }), drivers.end());
        Ultil<Driver>::saveToFile("Data/Driver.txt", drivers);
        populateDriversTable();
    }
}

// ---------- SEAT MANAGEMENT ----------
void AdminWindow::onViewSeatMapClicked() {
    QTableWidget *busTable = findChild<QTableWidget*>("busTable");
    if (!busTable) return;
    
    auto items = busTable->selectedItems();
    if (items.empty()) {
        QMessageBox::warning(this, "Error", "Please select a bus from the table!");
        return;
    }
    
    int row = busTable->row(items.first());
    if (row < 0 || row >= (int)buses.size()) return;
    
    QString busId = QString::fromStdString(buses[row].getId());
    
    // Ask for date via calendar
    QDialog dateDlg(this);
    dateDlg.setWindowTitle("Choose a date to view seat map");
    dateDlg.setStyleSheet("background: #0f172a; color: #e5e7eb;");
    dateDlg.setMinimumSize(560, 520);
    QVBoxLayout vbox(&dateDlg);
    QLabel lbl("Choose a date:", &dateDlg);
    lbl.setVisible(false);
    QCalendarWidget calendar(&dateDlg);
    calendar.setGridVisible(true);
    calendar.setSelectedDate(QDate::currentDate());
    calendar.setStyleSheet(
        "QCalendarWidget {"
        "  background: #0f172a;"
        "  color: #e2e8f0;"
        "  border: 1px solid #1f2937;"
        "  border-radius: 12px;"
        "  selection-background-color: #2563eb;"
        "  selection-color: #f8fafc;"
        "}"
        "QCalendarWidget QWidget#qt_calendar_navigationbar {"
        "  background: #0f172a;"
        "  min-height: 46px;"
        "  border-bottom: 1px solid #1f2937;"
        "}"
        "QCalendarWidget QToolButton {"
        "  color: transparent;"
        "  background: transparent;"
        "  border: none;"
        "  padding: 6px 12px;"
        "  font-weight: 600;"
        "  min-width: 20px;"
        "}"
        "QCalendarWidget QToolButton:hover { color: transparent; }"
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
        "  background: #0f172a;"
        "  color: transparent;"
        "  border: 1px solid #1f2937;"
        "  border-radius: 6px;"
        "  padding: 2px 6px;"
        "}"
        "QCalendarWidget QSpinBox::up-button,"
        "QCalendarWidget QSpinBox::down-button {"
        "  width: 0px;"
        "}"
    );
    QDialogButtonBox box(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dateDlg);
    box.setStyleSheet("QPushButton { background: #2563eb; color: white; padding: 8px 16px; border-radius: 6px; } QPushButton:hover { background: #1d4ed8; }");
    vbox.addWidget(&lbl);
    vbox.addWidget(&calendar);
    vbox.addWidget(&box);
    connect(&box, &QDialogButtonBox::accepted, &dateDlg, &QDialog::accept);
    connect(&box, &QDialogButtonBox::rejected, &dateDlg, &QDialog::reject);
    if (dateDlg.exec() != QDialog::Accepted) return;

    QString selectedDate = calendar.selectedDate().toString("yyyy-MM-dd");

    // Build seat list from base capacity (use existing seats if available, else generate by capacity)
    std::vector<int> seatNumbers;
    for (const auto& s : seats) if (s.getBusId() == busId.toStdString()) seatNumbers.push_back(s.getSeatNo());
    if (seatNumbers.empty()) {
        int cap = buses[row].getCapacity();
        seatNumbers.reserve(cap);
        for (int i = 1; i <= cap; ++i) seatNumbers.push_back(i);
    }

    // Determine booked seats for the selected date from tickets
    std::set<int> bookedSeats;
    for (const auto& tk : tickets) {
        if (tk.getBusId() == busId.toStdString()) {
            QString bookedDate = QString::fromStdString(tk.getBookedAt()).left(10);
            if (bookedDate == selectedDate) {
                bookedSeats.insert(tk.getSeatNo());
            }
        }
    }
    
    // Create seat map dialog
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Seat Map " + busId + " - " + selectedDate);
    dialog->setStyleSheet("background: #0f172a; color: #e5e7eb;");
    dialog->setMinimumSize(500, 600);
    
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    
    // Summary
    int booked = static_cast<int>(bookedSeats.size());
    int total = static_cast<int>(seatNumbers.size());
    
    QLabel *lblSummary = new QLabel(QString("Total: %1 seats | Booked: %2 | Available: %3")
        .arg(total).arg(booked).arg(total - booked), dialog);
    lblSummary->setStyleSheet("font-size: 14px; font-weight: 600; color: #e5e7eb; padding: 10px; background: #1e293b; border-radius: 8px;");
    lblSummary->setAlignment(Qt::AlignCenter);
    
    // Seat table
    QTableWidget *table = new QTableWidget(dialog);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({"Seat No", "Status", "Availability"});
    table->setStyleSheet(StyleHelper::getTableStyle());
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    
    table->setRowCount(total);
    for (int i = 0; i < total; ++i) {
        int seatNo = seatNumbers[i];
        bool isBooked = bookedSeats.count(seatNo) > 0;
        table->setItem(i, 0, new QTableWidgetItem(QString::number(seatNo)));
        QString status = isBooked ? "🔴 Booked" : "🟢 Empty";
        QTableWidgetItem *statusItem = new QTableWidgetItem(status);
        statusItem->setForeground(QBrush(isBooked ? QColor(239, 68, 68) : QColor(34, 197, 94)));
        table->setItem(i, 1, statusItem);
        table->setItem(i, 2, new QTableWidgetItem(isBooked ? "Cannot book" : "Can book"));
    }

    QPushButton *btnClose = new QPushButton("Close", dialog);
    btnClose->setStyleSheet(StyleHelper::getSecondaryButtonStyle());
    btnClose->setCursor(Qt::PointingHandCursor);
    connect(btnClose, &QPushButton::clicked, dialog, &QDialog::accept);
    
    layout->addWidget(lblSummary);
    layout->addWidget(table);
    layout->addWidget(btnClose);
    
    dialog->exec();
}

// ========== TICKET FILTERS ==========
void AdminWindow::onFilterTicketsByBus() {
    if (buses.empty()) {
        QMessageBox::warning(this, "Error", "No buses available!");
        return;
    }
    
    bool ok;
    QStringList busIds;
    for (const auto& b : buses) busIds << QString::fromStdString(b.getId() + " - " + b.getName());
    QString busChoice = QInputDialog::getItem(this, "Filter by bus", "Select bus:", busIds, 0, false, &ok);
    if (!ok) return;
    QString busId = busChoice.split(" - ").first();
    
    std::vector<Ticket> filtered;
    for (const auto& tk : tickets) {
        if (QString::fromStdString(tk.getBusId()) == busId) {
            filtered.push_back(tk);
        }
    }
    
    if (filtered.empty()) {
        QMessageBox::information(this, "Result", "No tickets found for this bus.");
        populateTicketsTable();
        return;
    }
    
    QTableWidget *table = findChild<QTableWidget*>("ticketDetailTable");
    if (!table) return;
    
    // Show bus type in dialog
    QString busType = "N/A";
    for (const auto& bus : buses) {
        if (QString::fromStdString(bus.getId()) == busId) {
            busType = QString::fromStdString(bus.getType());
            break;
        }
    }
    
    table->setRowCount(filtered.size());
    for (size_t i = 0; i < filtered.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(filtered[i].getId())));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(filtered[i].getTripId())));
        table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(filtered[i].getBusId())));
        table->setItem(i, 3, new QTableWidgetItem(busType));
        table->setItem(i, 4, new QTableWidgetItem(QString::number(filtered[i].getSeatNo())));
        table->setItem(i, 5, new QTableWidgetItem(QString::fromStdString(filtered[i].getPassengerName())));
        table->setItem(i, 6, new QTableWidgetItem(QString::fromStdString(filtered[i].getPhoneNumber())));
        table->setItem(i, 7, new QTableWidgetItem(QString::number(filtered[i].getPrice())));
        table->setItem(i, 8, new QTableWidgetItem(QString::fromStdString(filtered[i].getBookedAt())));
    }
}

void AdminWindow::onBookTicketClicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Book Ticket (Admin)");
    dialog.setStyleSheet("background: #0f172a; color: #e5e7eb;");
    dialog.setMinimumWidth(550);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    QFormLayout *formLayout = new QFormLayout();
    
    QComboBox *cbRoute = new QComboBox(&dialog);
    cbRoute->addItem("-- Select Route --", ""); 
    for (const auto& r : routes) {
        cbRoute->addItem(QString::fromStdString(r.getName()), QString::fromStdString(r.getId()));
    }

    QComboBox *cbTrip = new QComboBox(&dialog);
    
    QCalendarWidget *calTrip = new QCalendarWidget(&dialog);
    calTrip->setGridVisible(true);
    
    calTrip->setMinimumDate(QDate::currentDate()); 

    calTrip->setSelectedDate(QDate::currentDate());
    calTrip->setStyleSheet(
        "QCalendarWidget { background: #0f172a; color: #e2e8f0; border: 1px solid #1f2937; border-radius: 12px; selection-background-color: #2563eb; selection-color: #f8fafc; }"
        "QCalendarWidget QWidget#qt_calendar_navigationbar { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1f2937, stop:1 #0f172a); min-height: 46px; border-bottom: 1px solid #1f2937; }"
        "QCalendarWidget QToolButton { color: #e5e7eb; background: transparent; border: none; padding: 6px 12px; font-weight: 600; min-width: 20px; }"
        "QCalendarWidget QToolButton:hover { color: #3b82f6; }"
        "QCalendarWidget QAbstractItemView { background: #0b1220; alternate-background-color: #111827; selection-background-color: #2563eb; selection-color: #f8fafc; outline: 0; font-weight: 600; }"
        "QCalendarWidget QAbstractItemView::item { padding: 6px; margin: 2px; border-radius: 8px; }"
        "QCalendarWidget QAbstractItemView::item:selected { background: #2563eb; color: #f8fafc; }"
        "QCalendarWidget QAbstractItemView::item:hover { background: rgba(59,130,246,0.18); }"
        "QCalendarWidget QSpinBox { background: #111827; color: #e5e7eb; border: 1px solid #1f2937; border-radius: 6px; padding: 2px 6px; }"
        "QCalendarWidget QWidget { alternate-background-color: #0f172a; }" 
    );

    auto updateTrips = [&]() {
        cbTrip->clear();
        QString selectedRouteId = cbRoute->currentData().toString();
        if (selectedRouteId.isEmpty()) return;

        for (const auto& trip : trips) {
            if (trip.getRouteId() == selectedRouteId.toStdString()) {
                cbTrip->addItem(QString::fromStdString(trip.getId() + " (" + trip.getDepart() + ")"), 
                                QString::fromStdString(trip.getId()));
            }
        }
    };
    connect(cbRoute, QOverload<int>::of(&QComboBox::currentIndexChanged), updateTrips);

    QComboBox *cbSeat = new QComboBox(&dialog);
    QLineEdit *txtName = new QLineEdit(&dialog); txtName->setPlaceholderText("Passenger Name");
    QLineEdit *txtPhone = new QLineEdit(&dialog); txtPhone->setPlaceholderText("Phone Number");
    QComboBox *cbPayment = new QComboBox(&dialog); cbPayment->addItems({"Cash", "Momo", "ZaloPay", "Bank Transfer"});
    QLabel *lblPrice = new QLabel("Price: 0 VND", &dialog); lblPrice->setStyleSheet("font-weight: bold; color: #FDB515;");

    auto updateSeatsAndPrice = [&]() {
        cbSeat->clear();
        lblPrice->setText("Price: 0 VND");
        if (cbTrip->count() == 0) return;

        QString tripId = cbTrip->currentData().toString();
        QString dateStr = calTrip->selectedDate().toString("yyyy-MM-dd");
        
        std::string busId;
        int capacity = 0;
        for (const auto& trip : trips) {
            if (trip.getId() == tripId.toStdString()) { busId = trip.getBusId(); break; }
        }
        for (const auto& bus : buses) {
            if (bus.getId() == busId) { capacity = bus.getCapacity(); break; }
        }

        if (capacity == 0) { cbSeat->addItem("No Bus Found", -1); return; }

        std::set<int> booked;
        for (const auto& tk : tickets) {
            if (tk.getBusId() == busId) {
                QString bookedDate = QString::fromStdString(tk.getBookedAt()).left(10);
                if (bookedDate == dateStr) booked.insert(tk.getSeatNo());
            }
        }

        bool anySeat = false;
        for (int seatNo = 1; seatNo <= capacity; ++seatNo) {
            if (booked.count(seatNo) == 0) {
                cbSeat->addItem(QString("Seat %1").arg(seatNo), seatNo);
                anySeat = true;
            }
        }
        if (!anySeat) cbSeat->addItem("Sold Out", -1);

        unsigned long price = 0;
        QString routeId = cbRoute->currentData().toString();
        for (const auto& route : routes) {
            if (route.getId() == routeId.toStdString()) {
                try { price = FareCalculator::calculate(std::stol(route.getDistance())); } 
                catch (...) { price = FareCalculator::MIN_FARE; }
                break;
            }
        }
        QString priceStr = QString::number(price);
        int len = priceStr.length();
        for (int i = len - 3; i > 0; i -= 3) priceStr.insert(i, ',');
        lblPrice->setText("Price: " + priceStr + " VND");
        lblPrice->setProperty("price", QVariant::fromValue(static_cast<qulonglong>(price)));
    };

    connect(cbTrip, QOverload<int>::of(&QComboBox::currentIndexChanged), updateSeatsAndPrice);
    connect(calTrip, &QCalendarWidget::selectionChanged, updateSeatsAndPrice);
    
    formLayout->addRow("Select Route:", cbRoute);
    formLayout->addRow("Select Trip:", cbTrip);
    formLayout->addRow("Date:", calTrip);
    formLayout->addRow("Seat:", cbSeat);
    formLayout->addRow("Passenger:", txtName);
    formLayout->addRow("Phone:", txtPhone);
    formLayout->addRow("Payment:", cbPayment);
    formLayout->addRow("", lblPrice);
    
    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(btnBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(btnBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        if (cbRoute->currentData().toString().isEmpty()) {
            QMessageBox::warning(this, "Error", "Please select a Route!"); return;
        }
        if (calTrip->selectedDate() < QDate::currentDate()) {
            QMessageBox::warning(this, "Error", "Cannot book ticket for a past date!"); return;
        }
        if (txtName->text().isEmpty() || txtPhone->text().isEmpty()) {
            QMessageBox::warning(this, "Error", "Please enter passenger name and phone number!"); return;
        }
        if (cbSeat->currentData().toInt() == -1) {
            QMessageBox::warning(this, "Error", "Selected trip is full or no seat selected!"); return;
        }
    
        std::string tripId = cbTrip->currentData().toString().toStdString();
        std::string fileId = "TK" + tripId.substr(1);
        std::string ticketPath = "Data/Ticket/" + fileId + ".txt";

        int maxTicketNum = 0;
        {
            std::ifstream in(ticketPath);
            std::string ln;
            while (std::getline(in, ln)) {
                if (ln.empty()) continue;
                try {
                    Ticket t = Ticket::fromCSV(ln);
                    std::string id = t.getId();
                    if (id.rfind("TK", 0) == 0) {
                        try {
                            int num = std::stoi(id.substr(2));
                            if (num > maxTicketNum) maxTicketNum = num;
                        } catch (...) {}
                    }
                } catch (...) {}
            }
        }
        int nextTicketNum = maxTicketNum + 1;
        std::stringstream tkSs;
        tkSs << "TK" << std::setw(3) << std::setfill('0') << nextTicketNum;
        std::string ticketId = tkSs.str();

        std::string busId;
        for (const auto& trip : trips) {
            if (trip.getId() == tripId) {
                busId = trip.getBusId();
                break;
            }
        }
        
        int seatNo = cbSeat->currentData().toInt();
        unsigned long ticketPrice = lblPrice->property("price").toULongLong();
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        char timebuf[20];
        std::strftime(timebuf, sizeof(timebuf), "%H:%M:%S", std::localtime(&time));
        std::string bookedAt = calTrip->selectedDate().toString("yyyy-MM-dd").toStdString() + " " + timebuf;
        
        Ticket newTicket(
            ticketId,
            tripId,
            busId,
            seatNo,
            txtName->text().toStdString(),
            txtPhone->text().toStdString(),
            ticketPrice,
            bookedAt,
            cbPayment->currentText().toStdString()
        );
        tickets.push_back(newTicket);

        {
            std::fstream out(ticketPath, std::ios::in | std::ios::out | std::ios::app);
            if (!out.is_open()) {
                QMessageBox::critical(this, "Error", "Cannot open ticket file for writing!");
                return;
            }
            out.seekg(0, std::ios::end);
            std::streampos sz = out.tellg();
            if (sz > 0) {
                out.seekg(-1, std::ios::end);
                char last; out.get(last);
                if (last != '\n') out << '\n';
            }
            out << newTicket.toCSV() << "\n";
            out.close();
        }

        QMessageBox::information(this, "Success", 
            QString("Ticket booked successfully!\nTicket ID: %1").arg(QString::fromStdString(ticketId)));
        loadAllData();
        populateTicketsTable();
    }
}

void AdminWindow::onCancelTicketClicked() {
    QTableWidget *table = findChild<QTableWidget*>("ticketDetailTable");
    if (!table || !table->selectionModel() || !table->selectionModel()->hasSelection()) {
        QMessageBox::warning(this, "Error", "Please select a ticket in the right table before canceling!");
        return;
    }
    auto items = table->selectedItems();
    if (items.empty()) {
        QMessageBox::warning(this, "Error", "Please select a ticket to cancel!");
        return;
    }
    int row = table->row(items.first());
    if (row < 0) return;
    QTableWidgetItem* idItem = table->item(row, 0);
    if (!idItem) return;

    std::string selectedId = idItem->text().toStdString();
    int selectedSeat = table->item(row, 2) ? table->item(row, 2)->text().toInt() : -1;
    std::string selectedPassenger = table->item(row, 3) ? table->item(row, 3)->text().toStdString() : "";

    auto itTicket = std::find_if(tickets.begin(), tickets.end(), [&](const Ticket& t){
        return t.getId() == selectedId && t.getSeatNo() == selectedSeat && t.getPassengerName() == selectedPassenger;
    });
    if (itTicket == tickets.end()) {
        // Fallback: match by id only
        itTicket = std::find_if(tickets.begin(), tickets.end(), [&](const Ticket& t){ return t.getId() == selectedId; });
    }
    if (itTicket == tickets.end()) {
        QMessageBox::warning(this, "Error", "Ticket not found in data!");
        return;
    }
    int ticketIndex = static_cast<int>(std::distance(tickets.begin(), itTicket));

    auto reply = QMessageBox::question(this, "Confirm", 
        QString("Cancel ticket %1?\nPassenger: %2\nSeat: %3")
            .arg(QString::fromStdString(itTicket->getId()))
            .arg(QString::fromStdString(itTicket->getPassengerName()))
            .arg(itTicket->getSeatNo()));
    
    if (reply == QMessageBox::Yes) {
        std::string busId = itTicket->getBusId();
        std::string ticketId = itTicket->getId();
        std::string tripId = itTicket->getTripId();
        
        // Remove ticket from vector
        tickets.erase(tickets.begin() + ticketIndex);
        
        // Find the ticket file (tickets are grouped by some criteria in files)
        // Read all ticket files and rewrite the one that contains this ticket
        std::string ticketDir = "Data/Ticket/";
        for (const auto& entry : fs::directory_iterator(ticketDir)) {
            if (entry.is_regular_file()) {
                std::string filepath = entry.path().string();
                
                // Read all tickets from this file
                std::vector<Ticket> fileTickets;
                std::ifstream file(filepath);
                std::string line;
                bool foundTicket = false;
                
                while (std::getline(file, line)) {
                    Ticket t = Ticket::fromCSV(line);
                    if (t.getId() == ticketId) {
                        foundTicket = true;
                        // Skip this ticket 
                    } else {
                        fileTickets.push_back(t);
                    }
                }
                file.close();
                
                // If found, rewrite the file without the deleted ticket
                if (foundTicket) {
                    if (fileTickets.empty()) {
                        // If no tickets left, delete the file
                        std::remove(filepath.c_str());
                    } else {
                        // Rewrite file with remaining tickets
                        Ultil<Ticket>::saveToFile(filepath, fileTickets);
                    }
                    break;
                }
            }
        }
        
        QMessageBox::information(this, "Success!", "Ticket cancellation successful!");
        loadAllData();
        populateTicketsTable();
    }
}