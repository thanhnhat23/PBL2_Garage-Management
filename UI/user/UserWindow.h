#ifndef USERWINDOW_H
#define USERWINDOW_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QCalendarWidget>
#include <QTabWidget>
#include <QTimer>
#include <QComboBox>
#include <QShowEvent>
#include <vector>
#include "../../Class/User.h"
#include "../../Class/Route.h"
#include "../../Class/Bus.h"
#include "../../Class/Trip.h"
#include "../../Class/Ticket.h"

class UserWindow : public QWidget {
    Q_OBJECT
public:
    explicit UserWindow(QWidget *parent = nullptr);
    ~UserWindow();
    void setUser(User user);

signals:
    void logout();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onBookTicketClicked();
    void onViewSeatMapClicked();
    void onCancelMyTicketClicked();
    void onLogoutClicked();
    void refreshData();

private:
    void setupUI();
    void setupTabs();
    void loadData();
    void populateTrips(QString routeId = ""); 
    void populateMyTickets();
    
    QIcon renderSvgIcon(const QString& resourcePath, const QSize& size, const QString& colorHex);

    User currentUser;
    QTabWidget *tabWidget;
    QPushButton *activeButton = nullptr;
    
    QTableWidget *tripTable;
    QTableWidget *myTicketsTable;
    QCalendarWidget *calendar;

    std::vector<Route> routes;
    std::vector<Bus> buses;
    std::vector<Trip> trips;
    std::vector<Ticket> tickets;
    
    QTimer *refreshTimer;
};

#endif 