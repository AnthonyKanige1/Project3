#pragma once

#include <map>
#include <string>
#include <vector>

// I delcared a graph of campus locations. Each location has a list of roads that leave it.
class Graph {
public:
    void AddRoad(int from, int to, int minutes,
                 const std::string& fromName, const std::string& toName);
    std::string EdgeStatus(int from, int to) const;

private:
    struct Road {
        int to;
        int minutes;
        bool open;
    };

    std::map<int, std::vector<Road>> neighbors;
    std::map<int, std::string> locationNames;
};

// main() talks to this object. The Graph handles each question about roads.
class CampusCompass {
public:
    CampusCompass() = default;
    bool ParseCSV(const std::string& edgesFilepath, const std::string& classesFilepath);
    bool ParseCommand(const std::string& command);

private:
    Graph campus;
    std::map<std::string, int> classLocations;
};
