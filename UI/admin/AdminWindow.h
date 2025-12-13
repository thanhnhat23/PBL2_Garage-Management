#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QWidget>
#include <QTabWidget>
#include <QSplitter>
#include <QPushButton>
#include <QString>
#include <QTimer>
#include "../Class/User.h"
#include "../Class/Route.h"
#include "../Class/Bus.h"
#include "../Class/Trip.h"
#include "../Class/Ticket.h"
#include "../Class/Brand.h"
#include "../Class/Driver.h"
#include "../Class/Seat.h"
#include <vector>
#include <map>

class AdminWindow : public QWidget {
    Q_OBJECT

public:
    explicit AdminWindow(QWidget *parent = nullptr);
    ~AdminWindow();
    void setUser(User user);

signals:
    void logout();

private slots:
    void onLogoutClicked();
    void loadAllData();
    void onRefreshClicked();
    
    // Brand CRUD
    void onAddBrandClicked();
    void onEditBrandClicked();
    void onDeleteBrandClicked();
    
    // Route CRUD
    void onAddRouteClicked();
    void onEditRouteClicked();
    void onDeleteRouteClicked();
    
    // Bus CRUD
    void onAddBusClicked();
    void onEditBusClicked();
    void onDeleteBusClicked();
    
    // Driver CRUD
    void onAddDriverClicked();
    void onEditDriverClicked();
    void onDeleteDriverClicked();
    
    // Trip CRUD
    void onAddTripClicked();
    void onEditTripClicked();
    void onDeleteTripClicked();
    
    // Seat management
    void onViewSeatMapClicked();
    
    // Ticket filters and management
    void onFilterTicketsByBus();
    void onBookTicketClicked();
    void onCancelTicketClicked();
    
    // Stats filters
    void onFilterByDay();
    void onFilterByMonth();
    void onFilterByYear();
    void onShowAllStats();

private:
    User currentUser;
    QTabWidget *tabWidget;
    QPushButton *activeButton;  // Track active sidebar button
    
    // Helper for rendering SVG icons with color replacement
    static QIcon renderSvgIcon(const QString& resourcePath, const QSize& size = QSize(24, 24), const QString& colorHex = "#cbd5e1");
    
    // Data
    std::vector<Brand> brands;
    std::vector<Bus> buses;
    std::vector<Driver> drivers;
    std::vector<Route> routes;
    std::vector<Trip> trips;
    std::vector<Seat> seats;
    std::vector<Ticket> tickets;

    // Optimized cache for ticket table: precomputed display strings and search keys
    struct TicketRowCache {
        QString id;
        QString routeName;
        QString busName;
        QString seat;
        QString passenger;
        QString phone;
        QString price;     // formatted with thousands separator
        QString bookedAt;
        QString payment;
        QString searchKey; // concatenation of lowercased searchable fields
    };
    std::vector<TicketRowCache> ticketCache;

    // Search keywords per tab
    QString brandSearch;
    QString routeSearch;
    QString busSearch;
    QString driverSearch;
    QString tripSearch;
    QString ticketSearch;
    
    void setupUI();
    void setupBrandsTab();
    void setupRoutesTab();
    void setupBusesTab();
    void setupDriversTab();
    void setupTripsTab();
    void setupTicketsTab();
    void setupSeatsTab();
    void setupStatsTab();
    
    // Table population methods
    void populateBrandsTable();
    void populateRoutesTable();
    void populateBusesTable();
    void populateDriversTable();
    void populateTripsTable();
    void populateTicketsTable();
    void populateTicketFileSummary();
    void populateTicketsForFile(const std::string& fileId);
    void loadTicketsData();
    void rebuildTicketCache();
    void populateSeatsTable();
    void populateStatsTable();
    void populateStatsTableWithFilter(const std::vector<Ticket>& filteredTickets);

    // Tickets master-detail state
    std::string selectedTicketFile;
    QTimer *ticketSearchTimer = nullptr; // debounce ticket search
    std::map<std::string, std::vector<Ticket>> ticketsByFile; // fileId -> tickets in that file
    std::vector<std::string> ticketFileOrder; // preserve display order of ticket files

    // Lazy loading state
    bool ticketsLoaded = false;
    int ticketsTabIndex = -1;
};

#endif // ADMINWINDOW_H
