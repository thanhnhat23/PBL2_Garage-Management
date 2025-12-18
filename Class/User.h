#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "Role.h"

class User {
private:
    std::string userId;
    std::string username;
    std::string password;
    std::string phoneNumber;
    Role role;
public:
    User();
    User(std::string id, std::string uname, std::string pwd, Role r, std::string phone);

    const std::string& getId() const { return userId; }
    const std::string& getUsername() const { return username; }
    const std::string& getPassword() const { return password; }
    std::string getPhoneNumber() const { return phoneNumber; }
    Role getRole() const { return role; }
    void setPhoneNumber(const std::string& phone) { phoneNumber = phone; }
    void display() const;
    std::string toCSV() const;
    static User fromCSV(const std::string& line);
};
