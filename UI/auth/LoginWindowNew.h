#ifndef LOGINWINDOWNEW_H
#define LOGINWINDOWNEW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "../Class/User.h"
#include "../Class/Auth.h"
#include <vector>

class LoginWindowNew : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindowNew(QWidget *parent = nullptr);
    ~LoginWindowNew();

signals:
    void loginSuccess(User user);

private slots:
    void onLoginClicked();
    void onCreateAccountClicked();

private:
    void setupUI();
    void setupStyles();
    void loadUsers();
    void saveUsers();
    void clearForm();

    AuthManager authManager;
    std::vector<User> users;

    QLineEdit *txtUsername;
    QLineEdit *txtPassword;
    QPushButton *btnLogin;
    QLabel *lblError;
};

#endif // LOGINWINDOWNEW_H

