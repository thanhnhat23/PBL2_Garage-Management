#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "LoginWindowNew.h"
#include "AdminWindow.h"
#include "UserWindow.h"
#include "../Class/User.h"
#include "../Class/Auth.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoginSuccess(User user);
    void onLogout();

private:
    QStackedWidget *stackedWidget;
    LoginWindowNew *loginWindowNew;
    AdminWindow *adminWindow;
    UserWindow *userWindow;
    
    AuthManager authManager;
    User currentUser;
};

#endif // MAINWINDOW_H
