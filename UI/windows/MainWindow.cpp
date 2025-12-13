#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Set window properties
    setWindowTitle("Garage Management System");
    resize(1400, 900);
    
    // Initialize stacked widget
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
    
    // Create windows
    loginWindowNew = new LoginWindowNew(this);
    adminWindow = new AdminWindow(this);
    userWindow = new UserWindow(this);
    
    // Add to stack
    stackedWidget->addWidget(loginWindowNew);
    stackedWidget->addWidget(adminWindow);
    stackedWidget->addWidget(userWindow);
    
    // Connect signals
    connect(loginWindowNew, &LoginWindowNew::loginSuccess, this, &MainWindow::onLoginSuccess);
    connect(adminWindow, &AdminWindow::logout, this, &MainWindow::onLogout);
    connect(userWindow, &UserWindow::logout, this, &MainWindow::onLogout);
    
    // Show login by default
    stackedWidget->setCurrentWidget(loginWindowNew);
}

MainWindow::~MainWindow() {
}

void MainWindow::onLoginSuccess(User user) {
    currentUser = user;
    
    if (currentUser.getRole() == Role::ADMIN) {
        adminWindow->setUser(user);
        stackedWidget->setCurrentWidget(adminWindow);
    } else {
        userWindow->setUser(user);
        stackedWidget->setCurrentWidget(userWindow);
    }
}

void MainWindow::onLogout() {
    stackedWidget->setCurrentWidget(loginWindowNew);
}
