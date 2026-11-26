#pragma once

#include <cstddef>
#include <iosfwd>
#include <map>
#include <string>
#include <utility>
#include <vector>

// each undirected road has one stored weight and closure flag.
class Graph {
public:
    using Time = long long;
    struct Paths {
        std::map<int, Time> distance;
        std::map<int, int> previous;
    };

    bool AddRoad(int from, int to, int minutes,
                 const std::string& fromName, const std::string& toName);
    bool HasLocation(int location) const;
    std::string EdgeStatus(int from, int to) const;
    bool ToggleEdgesClosure(const std::vector<std::pair<int, int>>& endpoints);
    bool IsConnected(int from, int to) const;
    Paths ShortestPaths(int source) const;
    Time StudentZoneCost(int residence, const std::vector<int>& destinations) const;

private:
    struct Road {
        int from;
        int to;
        int minutes;
        bool open;
    };
    static std::pair<int, int> RoadKey(int from, int to);
    static int OtherEnd(const Road& road, int location);
    std::vector<Road> roads;
    std::map<std::pair<int, int>, std::size_t> roadIndex;
    std::map<int, std::vector<std::size_t>> neighbors;
    std::map<int, std::string> locationNames;
};

// The command interface owns validation, students, and the course catalog.
class CampusCompass {
public:
    bool ParseCSV(const std::string& edgesFilepath, const std::string& classesFilepath);
    bool ParseCommand(const std::string& command);
    bool ParseCommand(const std::string& command, std::ostream& output);

private:
    struct Course {
        int location;
        int start;
        int end;
    };
    struct Student {
        std::string name;
        int residence;
        std::vector<std::string> classes;
    };
    bool HasCourse(const std::string& code) const;
    bool Insert(const std::string& name, const std::string& id, int residence,
                const std::vector<std::string>& codes);
    bool Remove(const std::string& id);
    bool DropClass(const std::string& id, const std::string& code);
    bool ReplaceClass(const std::string& id, const std::string& oldCode,
                      const std::string& newCode);
    std::size_t RemoveClass(const std::string& code);
    void PrintShortestEdges(const Student& student, std::ostream& output) const;
    void PrintStudentZone(const Student& student, std::ostream& output) const;
    bool VerifySchedule(const Student& student, std::ostream& output) const;

    Graph campus;
    std::map<std::string, Course> courses;
    std::map<std::string, Student> students;
};
