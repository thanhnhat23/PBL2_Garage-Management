#include "../Class/Auth.h"

AuthManager::AuthManager() : currentUser(nullptr) {}

void AuthManager::load(const std::vector<User>& list) {
    users = list;
}

const User* AuthManager::login(const std::string& username, const std::string& password) {
    for (const auto& u : users) {
        if (u.getUsername() == username && u.getPassword() == password) {
            currentUser = &u;
            return currentUser;
        }
    }
    currentUser = nullptr;
    return nullptr;
}

void AuthManager::logout() {
    currentUser = nullptr;
}
