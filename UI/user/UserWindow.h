#ifndef USERWINDOW_H
#define USERWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QTabWidget>
#include <QCalendarWidget>
#include <QTableWidget>
#include "../Class/User.h"
#include "StyleHelper.h"
#include "../Class/Route.h"
#include "../Class/Bus.h"
#include "../Class/Trip.h"
#include "../Class/Ticket.h"
#include <vector>

class UserWindow : public QWidget {
    Q_OBJECT

public:
    explicit UserWindow(QWidget *parent = nullptr);
    ~UserWindow();
    void setUser(User user);
    
    static QIcon renderSvgIcon(const QString& resourcePath, const QSize& size = QSize(24, 24), const QString& colorHex = "#cbd5e1");

signals:
    void logout();

private slots:
    void onBookTicketClicked();
    void onViewSeatMapClicked();
    void onCancelMyTicketClicked();
    void onLogoutClicked();

private:
    User currentUser;
    QTabWidget *tabWidget;
    QCalendarWidget *calendar;
    QTableWidget *tripTable;
    QTableWidget *myTicketsTable;
    QString ticketSearch;
    QPushButton *activeButton = nullptr;

    // Data
    std::vector<Route> routes;
    std::vector<Bus> buses;
    std::vector<Trip> trips;
    std::vector<Ticket> tickets;

    void setupUI();
    void setupTabs();
    void loadData();
    void populateTrips();
    void populateMyTickets();
};

#endif // USERWINDOW_H
