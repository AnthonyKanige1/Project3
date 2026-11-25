#include "CampusCompass.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>

// This helper reads a single integer; rejects text such as "2abc", "jr43n".
namespace {
bool ReadInteger(const std::string& text, int& value) {
    std::istringstream field(text);
    char extra;
    return (field >> value) && !(field >> extra);
}
}

void Graph::AddRoad(int from, int to, int minutes,
                    const std::string& fromName, const std::string& toName) {
    locationNames[from] = fromName;
    locationNames[to] = toName;
    neighbors[from].push_back({to, minutes, true});
    neighbors[to].push_back({from, minutes, true});
}

std::string Graph::EdgeStatus(int from, int to) const {
    auto location = neighbors.find(from);
    if (location != neighbors.end()) {
        for (const Road& road : location->second) {
            if (road.to == to) {
                return road.open ? "open" : "closed";
            }
        }
    }
    return "DNE";
}

bool CampusCompass::ParseCSV(const std::string& edgesFilepath,
                             const std::string& classesFilepath) {
    std::ifstream edgesFile(edgesFilepath);
    std::ifstream classesFile(classesFilepath);
    std::string line;

    // The first row in each file contains column names.
    if (!std::getline(edgesFile, line) || !std::getline(classesFile, line)) {
        return false;
    }

    Graph loadedCampus;
    while (std::getline(edgesFile, line)) {
        std::istringstream row(line);
        std::string fromText, toText, fromName, toName, minutesText;
        if (!std::getline(row, fromText, ',') || !std::getline(row, toText, ',') ||
            !std::getline(row, fromName, ',') || !std::getline(row, toName, ',') ||
            !std::getline(row, minutesText)) {
            return false;
        }

        int from, to, minutes;
        if (!ReadInteger(fromText, from) || !ReadInteger(toText, to) ||
            !ReadInteger(minutesText, minutes)) {
            return false;
        }
        loadedCampus.AddRoad(from, to, minutes, fromName, toName);
    }

    std::map<std::string, int> loadedClasses;
    while (std::getline(classesFile, line)) {
        std::istringstream row(line);
        std::string classCode, locationText;
        int location;
        if (!std::getline(row, classCode, ',') || !std::getline(row, locationText, ',') ||
            !ReadInteger(locationText, location)) {
            return false;
        }
        // The remaining two columns are class times, | extra credit.
        loadedClasses[classCode] = location;
    }
    if (edgesFile.bad() || classesFile.bad()) {
        return false;
    }

    campus = std::move(loadedCampus);
    classLocations = std::move(loadedClasses);
    return true;
}


    bool CampusCompass::ParseCommand(const std::string& command) {
    std::istringstream input(command);
    std::string action;
    input >> action;

    if (action == "checkEdgeStatus") {
        std::string fromText, toText, extra;
        int from, to;
        if ((input >> fromText >> toText) && !(input >> extra) &&
            ReadInteger(fromText, from) && ReadInteger(toText, to)) {
            std::cout << campus.EdgeStatus(from, to) << '\n';
            return true;
        }
    }
    std::cout << "unsuccessful\n";
    return false;
   
    
}
