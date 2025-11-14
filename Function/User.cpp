#include "../Class/User.h"
#include <sstream>
#include <iomanip>

User::User() : role(Role::USER) {}

User::User(std::string id, std::string uname, std::string pwd, Role r)
: userId(id), username(uname), password(pwd), role(r) {}

void User::display() const {
    using namespace std;
    cout << left
         << setw(8)  << userId
         << setw(16) << username
         << setw(8)  << (role == Role::ADMIN ? "ADMIN" : "USER")
         << "\n";
}

std::string User::toCSV() const {
    return userId + "," + username + "," + password + "," + RoleToString(role);
}

User User::fromCSV(const std::string& line) {
    std::stringstream ss(line);
    std::string id, uname, pwd, r;
    if (!std::getline(ss, id, ',')) return User();
    if (!std::getline(ss, uname, ',')) return User();
    if (!std::getline(ss, pwd, ',')) return User();
    if (!std::getline(ss, r, ',')) return User();
    return User(id, uname, pwd, RoleFromString(r));
}
