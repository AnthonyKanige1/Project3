#include "CampusCompass.h"

#include <iostream>
#include <string>

int main() {
    CampusCompass compass;
    if (!compass.ParseCSV("data/edges.csv", "data/classes.csv")) {
        std::cerr << "Could not load data/edges.csv or data/classes.csv.\n";
        return 1;
    }
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cerr << "Missing command count.\n";
        return 1;
    }
    int commandCount = 0;
    try {
        std::size_t used = 0;
        commandCount = std::stoi(line, &used);
        if (commandCount < 1 || commandCount > 1000 ||
            line.find_first_not_of(" \t\r", used) != std::string::npos) {
            std::cerr << "Invalid command count.\n";
            return 1;
        }
    } catch (const std::exception&) {
        std::cerr << "Invalid command count.\n";
        return 1;
    }
    for (int i = 0; i < commandCount; ++i) {
        if (!std::getline(std::cin, line)) {
            std::cerr << "Missing command line.\n";
            return 1;
        }
        compass.ParseCommand(line);
    }
    return 0;
}
