#pragma once
#include <vector>
#include <string>
#include "User.h"

class AuthManager {
private:
    std::vector<User> users;
    const User* currentUser;
public:
    AuthManager();

    void load(const std::vector<User>& list);
    const User* login(const std::string& username, const std::string& password);
    void logout();

    bool isLoggedIn() const { return currentUser != nullptr; }
    bool isAdmin() const { return currentUser && currentUser->getRole() == Role::ADMIN; }
    const User* getCurrentUser() const { return currentUser; }
};
