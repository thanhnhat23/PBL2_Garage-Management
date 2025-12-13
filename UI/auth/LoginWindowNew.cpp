#include "LoginWindowNew.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QMessageBox>
#include <QInputDialog>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

LoginWindowNew::LoginWindowNew(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("QWidget { background: #0f1419; color: #e2e8f0; font-family: 'Segoe UI', sans-serif; }");
    setupUI();
    setupStyles();
    loadUsers();
}

LoginWindowNew::~LoginWindowNew() {
}

void LoginWindowNew::setupUI() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ===== LEFT SIDE: Logo (2/3) =====
    QWidget *leftWidget = new QWidget();
    leftWidget->setStyleSheet("QWidget { background: #0a0e14; }");
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setAlignment(Qt::AlignCenter);
    leftLayout->setContentsMargins(60, 60, 60, 60);

    QLabel *lblLogo = new QLabel();
    QPixmap logoPixmap(":icons/icons/logo.png");
    lblLogo->setPixmap(logoPixmap.scaled(580, 580, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    lblLogo->setAlignment(Qt::AlignCenter);

    leftLayout->addStretch();
    leftLayout->addWidget(lblLogo);
    leftLayout->addStretch();

    // ===== RIGHT SIDE: Login Form (1/3) =====
    QWidget *rightWidget = new QWidget();
    rightWidget->setStyleSheet("QWidget { background: #0f1419; }");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setAlignment(Qt::AlignCenter);
    rightLayout->setContentsMargins(80, 80, 80, 80);
    rightLayout->setSpacing(20);

    QLabel *lblWelcome = new QLabel("Welcome Back");
    lblWelcome->setStyleSheet("color: #f1f5f9; font-size: 32px; font-weight: 800;");
    lblWelcome->setAlignment(Qt::AlignCenter);

    txtUsername = new QLineEdit();
    txtUsername->setObjectName("txtUsername");
    txtUsername->setPlaceholderText("Username");
    txtUsername->setMinimumHeight(50);

    txtPassword = new QLineEdit();
    txtPassword->setObjectName("txtPassword");
    txtPassword->setPlaceholderText("Password");
    txtPassword->setEchoMode(QLineEdit::Password);
    txtPassword->setMinimumHeight(50);

    btnLogin = new QPushButton("Login");
    btnLogin->setObjectName("btnLogin");
    btnLogin->setMinimumHeight(50);
    btnLogin->setCursor(Qt::PointingHandCursor);

    lblError = new QLabel();
    lblError->setStyleSheet("color: #ef4444; font-size: 13px;");
    lblError->setAlignment(Qt::AlignCenter);
    lblError->hide();

    rightLayout->addStretch();
    rightLayout->addWidget(lblWelcome);
    rightLayout->addSpacing(30);
    rightLayout->addWidget(txtUsername);
    rightLayout->addWidget(txtPassword);
    rightLayout->addSpacing(10);
    rightLayout->addWidget(btnLogin);
    rightLayout->addWidget(lblError);

    QLabel *lblCreateAccount = new QLabel("<p style='color: #94a3b8;'> Don't have an account? <a href='#' style='color: #1d4ed8;'>Create account</a></p>");
    lblCreateAccount->setAlignment(Qt::AlignCenter);
    lblCreateAccount->setOpenExternalLinks(false);
    lblCreateAccount->setStyleSheet("font-size: 12px;");
    rightLayout->addSpacing(10);
    rightLayout->addWidget(lblCreateAccount);
    rightLayout->addStretch();

    mainLayout->addWidget(leftWidget, 2);
    mainLayout->addWidget(rightWidget, 1);
    setLayout(mainLayout);

    connect(btnLogin, &QPushButton::clicked, this, &LoginWindowNew::onLoginClicked);
    connect(lblCreateAccount, &QLabel::linkActivated, this, &LoginWindowNew::onCreateAccountClicked);
}

void LoginWindowNew::setupStyles() {
    // Input fields
    QString inputStyle =
        "QLineEdit {"
        "  background: #1a1f2e;"
        "  border: 1px solid #334155;"
        "  border-radius: 8px;"
        "  padding: 12px 16px;"
        "  color: #e2e8f0;"
        "  font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #3b82f6;"
        "  background: #1a1f2e;"
        "}"
        "QLineEdit::placeholder {"
        "  color: #64748b;"
        "}";

    txtUsername->setStyleSheet(inputStyle);
    txtPassword->setStyleSheet(inputStyle);

    // Login button
    QString buttonStyle =
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #1d4ed8);"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 8px;"
        "  font-size: 15px;"
        "  font-weight: 700;"
        "  box-shadow: 0 10px 25px rgba(59, 130, 246, 0.35);"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #1e40af);"
        "}"
        "QPushButton:pressed {"
        "  box-shadow: 0 5px 15px rgba(59, 130, 246, 0.25);"
        "}";

    btnLogin->setStyleSheet(buttonStyle);
}

void LoginWindowNew::onLoginClicked() {
    QString username = txtUsername->text();
    QString password = txtPassword->text();

    if (username.isEmpty() || password.isEmpty()) {
        lblError->setText("Please enter username and password!");
        lblError->show();
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
        lblError->setText("Invalid username or password!");
        lblError->show();
    }
}

void LoginWindowNew::onCreateAccountClicked() {
    // Dialog to create new account
    bool ok;
    QString username = QInputDialog::getText(this, "Create Account", "Username:", QLineEdit::Normal, "", &ok);

    if (!ok || username.isEmpty()) {
        return;
    }

    // Check if username already exists
    for (const auto& user : users) {
        if (user.getUsername() == username.toStdString()) {
            QMessageBox::warning(this, "Error", "Username already exists!");
            return;
        }
    }

    QString password = QInputDialog::getText(this, "Create Account", "Password:", QLineEdit::Password, "", &ok);

    if (!ok || password.isEmpty()) {
        return;
    }

    // Generate ID with proper format (U001, U002, U003, etc.)
    int nextId = users.size() + 1;
    QString userId = QString("U%1").arg(nextId, 3, 10, QChar('0'));  // Format as U001, U002, etc.

    // Create new user with USER role
    User newUser(userId.toStdString(),
                 username.toStdString(),
                 password.toStdString(),
                 Role::USER);

    users.push_back(newUser);
    saveUsers();

    QMessageBox::information(this, "Success", "Account created successfully! Please login.");
    clearForm();
}

void LoginWindowNew::clearForm() {
    txtUsername->clear();
    txtPassword->clear();
    lblError->hide();
}

void LoginWindowNew::loadUsers() {
    users.clear();
    std::ifstream file("Data/User.txt");
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty()) {
            users.push_back(User::fromCSV(line));
        }
    }

    if (users.empty()) {
        users.push_back(User("U001", "admin", "admin123", Role::ADMIN));
        users.push_back(User("U002", "user1", "123456", Role::USER));
        saveUsers();
    }
}

void LoginWindowNew::saveUsers() {
    std::ofstream file("Data/User.txt");
    for (const auto& user : users) {
        file << user.toCSV() << "\n";
    }
}
