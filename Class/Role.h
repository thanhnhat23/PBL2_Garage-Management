#pragma once
#include <string>
#include <algorithm>

enum class Role { ADMIN, USER };

inline std::string RoleToString(Role r) {
    return r == Role::ADMIN ? "ADMIN" : "USER";
}
inline Role RoleFromString(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c){ return std::toupper(c); });
    return (s == "ADMIN") ? Role::ADMIN : Role::USER;
}
