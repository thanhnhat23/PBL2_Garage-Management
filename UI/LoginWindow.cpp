#include "LoginWindow.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("QWidget { background-color: #0b1220; color: #e5e7eb; font-family: 'Segoe UI', sans-serif; }");
    
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Left panel (Logo area)
    QWidget *leftPanel = new QWidget(this);
    leftPanel->setStyleSheet("background-color: #ff0000;"); // RED for testing
    leftPanel->setMinimumWidth(480);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setAlignment(Qt::AlignCenter);
    
    QLabel *testLabel = new QLabel("TESTING BUILD", leftPanel);
    testLabel->setStyleSheet("font-size: 48px; font-weight: bold; color: #ffffff;");
    testLabel->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(testLabel);
    
    // Right panel (Login form)
    QWidget *rightPanel = new QWidget(this);
    rightPanel->setStyleSheet("background-color: #0f172a; border-radius: 20px; border: 1px solid #1f2937;");
    rightPanel->setMinimumWidth(500);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(60, 80, 60, 80);
    rightLayout->setSpacing(20);
    
    // Title
    QLabel *lblTitle = new QLabel("LOGIN", rightPanel);
    lblTitle->setStyleSheet("font-size: 32px; font-weight: 800; color: #e5e7eb; margin-bottom: 20px;");
    lblTitle->setAlignment(Qt::AlignCenter);
    
    // Username field
    QLineEdit *txtUsername = new QLineEdit(rightPanel);
    txtUsername->setObjectName("txtUsername");
    txtUsername->setPlaceholderText("Username");
    txtUsername->setStyleSheet(
        "QLineEdit {"
        "   padding: 14px;"
        "   border: 1px solid #1f2937;"
        "   border-radius: 10px;"
        "   font-size: 16px;"
        "   background: #111827;"
        "   color: #e5e7eb;"
        "}"
        "QLineEdit:focus {"
        "   border: 1px solid #2563eb;"
        "   box-shadow: 0 0 0 2px rgba(37,99,235,0.25);"
        "}"
        "QLineEdit::placeholder { color: #6b7280; }"
    );
    txtUsername->setMinimumHeight(50);
    
    // Password field  
    QLineEdit *txtPassword = new QLineEdit(rightPanel);
    txtPassword->setObjectName("txtPassword");
    txtPassword->setPlaceholderText("Password");
    txtPassword->setEchoMode(QLineEdit::Password);
    txtPassword->setStyleSheet(
        "QLineEdit {"
        "   padding: 14px;"
        "   border: 1px solid #1f2937;"
        "   border-radius: 10px;"
        "   font-size: 16px;"
        "   background: #111827;"
        "   color: #e5e7eb;"
        "}"
        "QLineEdit:focus {"
        "   border: 1px solid #2563eb;"
        "   box-shadow: 0 0 0 2px rgba(37,99,235,0.25);"
        "}"
        "QLineEdit::placeholder { color: #6b7280; }"
    );
    txtPassword->setMinimumHeight(50);
    
    // Create account link
    QLabel *lblCreateLink = new QLabel("<a href='#' style='color: #60a5fa; text-decoration: none;'>Create account</a>", rightPanel);
    lblCreateLink->setAlignment(Qt::AlignRight);
    lblCreateLink->setOpenExternalLinks(false);
    lblCreateLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    
    // Login button
    QPushButton *btnLogin = new QPushButton("GET STARTED", rightPanel);
    btnLogin->setObjectName("btnLogin");
    btnLogin->setStyleSheet(
        "QPushButton {"
        "   background-color: #2563eb;"
        "   color: #f8fafc;"
        "   font-size: 17px;"
        "   font-weight: 700;"
        "   border: 1px solid #1d4ed8;"
        "   border-radius: 12px;"
        "   padding: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #1d4ed8;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #1e40af;"
        "}"
    );
    btnLogin->setMinimumHeight(55);
    btnLogin->setCursor(Qt::PointingHandCursor);
    
    // Hidden create button (for compatibility)
    QPushButton *btnCreate = new QPushButton(rightPanel);
    btnCreate->setObjectName("btnCreate");
    btnCreate->hide();
    
    // Add to right layout
    rightLayout->addWidget(lblTitle);
    rightLayout->addSpacing(20);
    rightLayout->addWidget(txtUsername);
    rightLayout->addWidget(txtPassword);
    rightLayout->addWidget(lblCreateLink);
    rightLayout->addSpacing(10);
    rightLayout->addWidget(btnLogin);
    rightLayout->addStretch();
    
    // Add panels to main layout
    mainLayout->addWidget(leftPanel, 1);
    mainLayout->addWidget(rightPanel, 1);
    
    setLayout(mainLayout);
    
    // Load users
    loadUsers();
    
    // Connect signals
    connect(btnLogin, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(btnCreate, &QPushButton::clicked, this, &LoginWindow::onCreateAccountClicked);
    connect(lblCreateLink, &QLabel::linkActivated, this, &LoginWindow::onCreateAccountClicked);
}

LoginWindow::~LoginWindow() {
}

void LoginWindow::loadUsers() {
    users.clear();
    std::ifstream file("Data/User.txt");
    std::string line;
    
    while (std::getline(file, line)) {
        if (!line.empty()) {
            users.push_back(User::fromCSV(line));
        }
    }
    
    // Create default admin if no users
    if (users.empty()) {
        users.push_back(User("U001", "admin", "admin123", Role::ADMIN));
        users.push_back(User("U002", "user1", "123456", Role::USER));
        saveUsers();
    }
}

void LoginWindow::saveUsers() {
    std::ofstream file("Data/User.txt");
    for (const auto& user : users) {
        file << user.toCSV() << "\n";
    }
}

void LoginWindow::onLoginClicked() {
    QLineEdit *txtUsername = findChild<QLineEdit*>("txtUsername");
    QLineEdit *txtPassword = findChild<QLineEdit*>("txtPassword");
    
    QString username = txtUsername->text();
    QString password = txtPassword->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter username and password!");
        return;
    }
    
    authManager.load(users);
    if (authManager.login(username.toStdString(), password.toStdString())) {
        const User* currentUserPtr = authManager.getCurrentUser();
        if (currentUserPtr) {
            emit loginSuccess(*currentUserPtr);
            clearForm();
        }
    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid username or password!");
    }
}

void LoginWindow::onCreateAccountClicked() {
    QLineEdit *txtUsername = findChild<QLineEdit*>("txtUsername");
    QLineEdit *txtPassword = findChild<QLineEdit*>("txtPassword");
    
    QString username = txtUsername->text();
    QString password = txtPassword->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter username and password!");
        return;
    }
    
    if (username.length() < 3) {
        QMessageBox::warning(this, "Error", "Username must be at least 3 characters!");
        return;
    }
    
    if (password.length() < 4) {
        QMessageBox::warning(this, "Error", "Password must be at least 4 characters!");
        return;
    }
    
    // Check if username exists
    for (const auto& user : users) {
        if (user.getUsername() == username.toStdString()) {
            QMessageBox::warning(this, "Error", "Username already exists!");
            return;
        }
    }
    
    // Generate new ID
    int maxId = 0;
    for (const auto& user : users) {
        std::string id = user.getId();
        if (id.size() >= 2 && (id[0] == 'U' || id[0] == 'u')) {
            try {
                int num = std::stoi(id.substr(1));
                maxId = std::max(maxId, num);
            } catch (...) {}
        }
    }
    
    std::stringstream ss;
    ss << "U" << std::setfill('0') << std::setw(3) << (maxId + 1);
    
    users.push_back(User(ss.str(), username.toStdString(), password.toStdString(), Role::USER));
    saveUsers();
    
    QMessageBox::information(this, "Success", "Account created successfully!\nID: " + QString::fromStdString(ss.str()));
    clearForm();
}

void LoginWindow::clearForm() {
    QLineEdit *txtUsername = findChild<QLineEdit*>("txtUsername");
    QLineEdit *txtPassword = findChild<QLineEdit*>("txtPassword");
    
    if (txtUsername) txtUsername->clear();
    if (txtPassword) txtPassword->clear();
}
