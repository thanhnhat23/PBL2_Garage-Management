#pragma once
#include <string>
#include <vector>

class ConsoleMenu {
public:
    static int pick(const std::string& title, const std::vector<std::string>& options);
};
