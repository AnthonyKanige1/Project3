#include "CampusCompass.h"

#include <iostream>
#include <string>

int main() {
    CampusCompass compass;
    if (!compass.ParseCSV("data/edges.csv", "data/classes.csv")) {
        std::cerr << "Could not load data/edges.csv or data/classes.csv.\n";
        return 1;
    }

    // The assignment gives a valid command count on its own line.
    std::string line;
    if (!std::getline(std::cin, line)) {
        return 1;
    }
    const int commandCount = std::stoi(line);
    for (int i = 0; i < commandCount; ++i) {
        if (!std::getline(std::cin, line)) {
            return 1;
        }
        compass.ParseCommand(line);
    }
}
