#include "../Class/User.h"
#include <sstream>
#include <iomanip>
#include <vector>

User::User() : role(Role::USER), phoneNumber("") {}

User::User(std::string id, std::string uname, std::string pwd, Role r, std::string phone)
: userId(id), username(uname), password(pwd), role(r), phoneNumber(phone) {}

void User::display() const {
    using namespace std;
    cout << left
         << setw(8)  << userId
         << setw(16) << username
         << setw(8)  << (role == Role::ADMIN ? "ADMIN" : "USER")
         << setw(15) << phoneNumber 
         << "\n";
}

std::string User::toCSV() const {
    return userId + "," + username + "," + password + "," + RoleToString(role) + "," + phoneNumber;
}

User User::fromCSV(const std::string& line) {
    std::stringstream ss(line);
    std::string id, uname, pwd, r, phone;
    
    if (!std::getline(ss, id, ',')) return User();
    if (!std::getline(ss, uname, ',')) return User();
    if (!std::getline(ss, pwd, ',')) return User();
    if (!std::getline(ss, r, ',')) return User();
    if (std::getline(ss, phone, ',')) {
        if (!phone.empty() && phone.back() == '\r') phone.pop_back();
    } else {
        phone = "";
    }

    return User(id, uname, pwd, RoleFromString(r), phone);
}