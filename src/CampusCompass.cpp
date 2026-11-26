#include "CampusCompass.h"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <functional>
#include <iostream>
#include <queue>
#include <set>
#include <sstream>

namespace {
bool IsDigit(char c) { return c >= '0' && c <= '9'; }

bool IsLetter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool ReadInteger(const std::string& text, int& value) {
    if (text.empty() || !std::all_of(text.begin(), text.end(), IsDigit)) {
        return false;
    }
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size();
}

bool ValidID(const std::string& id) {
    return id.size() == 8 && std::all_of(id.begin(), id.end(), IsDigit);
}

bool ValidName(const std::string& name) {
    return std::any_of(name.begin(), name.end(), IsLetter) &&
           std::all_of(name.begin(), name.end(), [](char c) {
               return IsLetter(c) || c == ' ';
           });
}

bool ValidCode(const std::string& code) {
    return code.size() == 7 &&
           std::all_of(code.begin(), code.begin() + 3,
                       [](char c) { return c >= 'A' && c <= 'Z'; }) &&
           std::all_of(code.begin() + 3, code.end(), IsDigit);
}

bool AtEnd(std::istringstream& input) {
    input >> std::ws;
    return input.eof();
}

bool Status(bool success, std::ostream& output) {
    output << (success ? "successful\n" : "unsuccessful\n");
    return success;
}

std::string Trim(const std::string& text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    return text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
}

bool ReadCSVRow(const std::string& line, std::vector<std::string>& fields) {
    fields.clear();
    std::size_t position = 0;
    while (true) {
        std::string field;
        if (position < line.size() && line[position] == '"') {
            ++position;
            bool closed = false;
            while (position < line.size()) {
                const char c = line[position++];
                if (c != '"') {
                    field += c;
                } else if (position < line.size() && line[position] == '"') {
                    field += '"';
                    ++position;
                } else {
                    closed = true;
                    break;
                }
            }
            if (!closed) {
                return false;
            }
            while (position < line.size() &&
                   (line[position] == ' ' || line[position] == '\t' || line[position] == '\r')) {
                ++position;
            }
            if (position < line.size() && line[position] != ',') {
                return false;
            }
        } else {
            while (position < line.size() && line[position] != ',') {
                if (line[position] == '"') {
                    return false;
                }
                field += line[position++];
            }
        }
        fields.push_back(Trim(field));
        if (position == line.size()) {
            return true;
        }
        ++position;
    }
}

bool ReadTime(const std::string& text, int& minutes) {
    int hour = 0;
    int minute = 0;
    if (text.size() != 5 || text[2] != ':' ||
        !ReadInteger(text.substr(0, 2), hour) ||
        !ReadInteger(text.substr(3, 2), minute) || hour > 23 || minute > 59) {
        return false;
    }
    minutes = hour * 60 + minute;
    return true;
}
} // namespace

std::pair<int, int> Graph::RoadKey(int from, int to) {
    return {std::min(from, to), std::max(from, to)};
}

int Graph::OtherEnd(const Road& road, int location) {
    return road.from == location ? road.to : road.from;
}

bool Graph::AddRoad(int from, int to, int minutes,
                    const std::string& fromName, const std::string& toName) {
    const auto key = RoadKey(from, to);
    if (from < 0 || to < 0 || minutes < 0 || roadIndex.count(key) != 0) {
        return false;
    }
    const std::size_t index = roads.size();
    roads.push_back({from, to, minutes, true});
    roadIndex.emplace(key, index);
    neighbors[from].push_back(index);
    neighbors[to].push_back(index);
    locationNames[from] = fromName;
    locationNames[to] = toName;
    return true;
}

bool Graph::HasLocation(int location) const {
    return neighbors.count(location) != 0;
}

std::string Graph::EdgeStatus(int from, int to) const {
    const auto found = roadIndex.find(RoadKey(from, to));
    if (found == roadIndex.end()) {
        return "DNE";
    }
    return roads[found->second].open ? "open" : "closed";
}

bool CampusCompass::ParseCSV(const std::string& edgesFilepath,
                             const std::string& classesFilepath) {
    std::ifstream edgesFile(edgesFilepath);
    std::ifstream classesFile(classesFilepath);
    std::string line;
    if (!std::getline(edgesFile, line) || !std::getline(classesFile, line)) {
        return false;
    }
    Graph loadedCampus;
    std::vector<std::string> fields;
    while (std::getline(edgesFile, line)) {
        if (Trim(line).empty()) {
            continue;
        }
        int from = 0, to = 0, minutes = 0;
        if (!ReadCSVRow(line, fields) || fields.size() != 5 ||
            !ReadInteger(fields[0], from) || !ReadInteger(fields[1], to) ||
            !ReadInteger(fields[4], minutes) ||
            !loadedCampus.AddRoad(from, to, minutes, fields[2], fields[3])) {
            return false;
        }
    }
    std::map<std::string, Course> loadedCourses;
    while (std::getline(classesFile, line)) {
        if (Trim(line).empty()) {
            continue;
        }
        Course course{};
        if (!ReadCSVRow(line, fields) || fields.size() != 4 || !ValidCode(fields[0]) ||
            !ReadInteger(fields[1], course.location) ||
            !loadedCampus.HasLocation(course.location) ||
            !ReadTime(fields[2], course.start) || !ReadTime(fields[3], course.end) ||
            course.end < course.start || !loadedCourses.emplace(fields[0], course).second) {
            return false;
        }
    }
    if (edgesFile.bad() || classesFile.bad()) {
        return false;
    }
    // a failed load preserves the previous graph, catalog, and students.
    campus = std::move(loadedCampus);
    courses = std::move(loadedCourses);
    students.clear();
    return true;
}

bool CampusCompass::HasCourse(const std::string& code) const {
    return ValidCode(code) && courses.count(code) != 0;
}

bool CampusCompass::Insert(const std::string& name, const std::string& id, int residence,
                           const std::vector<std::string>& codes) {
    if (!ValidName(name) || !ValidID(id) || students.count(id) != 0 ||
        !campus.HasLocation(residence) || codes.empty() || codes.size() > 6) {
        return false;
    }
    std::set<std::string> unique;
    for (const auto& code : codes) {
        if (!HasCourse(code) || !unique.insert(code).second) {
            return false;
        }
    }
    students.emplace(id, Student{name, residence, codes});
    return true;
}

bool CampusCompass::Remove(const std::string& id) {
    return students.erase(id) != 0;
}

bool CampusCompass::DropClass(const std::string& id, const std::string& code) {
    const auto student = students.find(id);
    if (student == students.end() || !HasCourse(code)) {
        return false;
    }
    auto& codes = student->second.classes;
    const auto enrolled = std::find(codes.begin(), codes.end(), code);
    if (enrolled == codes.end()) {
        return false;
    }
    codes.erase(enrolled);
    if (codes.empty()) {
        students.erase(student);
    }
    return true;
}

bool CampusCompass::ReplaceClass(const std::string& id, const std::string& oldCode,
                                 const std::string& newCode) {
    const auto student = students.find(id);
    if (student == students.end() || !HasCourse(oldCode) || !HasCourse(newCode)) {
        return false;
    }
    auto& codes = student->second.classes;
    const auto enrolled = std::find(codes.begin(), codes.end(), oldCode);
    if (enrolled == codes.end() || std::find(codes.begin(), codes.end(), newCode) != codes.end()) {
        return false;
    }
    *enrolled = newCode;
    return true;
}

std::size_t CampusCompass::RemoveClass(const std::string& code) {
    std::size_t affected = 0;
    for (auto student = students.begin(); student != students.end();) {
        auto& codes = student->second.classes;
        const auto enrolled = std::find(codes.begin(), codes.end(), code);
        if (enrolled != codes.end()) {
            codes.erase(enrolled);
            ++affected;
        }
        if (codes.empty()) {
            student = students.erase(student);
        } else {
            ++student;
        }
    }
    return affected;
}

bool CampusCompass::ParseCommand(const std::string& command) {
    return ParseCommand(command, std::cout);
}

bool CampusCompass::ParseCommand(const std::string& command, std::ostream& output) {
    std::istringstream input(command);
    std::string action;
    input >> action;

    if (action == "insert") {
        std::string name, id, residenceText, countText;
        int residence = 0, count = 0;
        input >> std::ws;
        if (input.get() != '"' || !std::getline(input, name, '"') ||
            (input.peek() != ' ' && input.peek() != '\t') ||
            !(input >> id >> residenceText >> countText) ||
            !ReadInteger(residenceText, residence) || !ReadInteger(countText, count) ||
            count < 1 || count > 6) {
            return Status(false, output);
        }
        std::vector<std::string> codes;
        for (int i = 0; i < count; ++i) {
            std::string code;
            if (!(input >> code)) {
                return Status(false, output);
            }
            codes.push_back(code);
        }
        return Status(AtEnd(input) && Insert(name, id, residence, codes), output);
    }

    if (action == "remove" || action == "dropClass" || action == "replaceClass") {
        std::string id, oldCode, newCode;
        if (!(input >> id) || !ValidID(id)) {
            return Status(false, output);
        }
        if (action == "remove") {
            return Status(AtEnd(input) && Remove(id), output);
        }
        if (!(input >> oldCode)) {
            return Status(false, output);
        }
        if (action == "dropClass") {
            return Status(AtEnd(input) && DropClass(id, oldCode), output);
        }
        return Status((input >> newCode) && AtEnd(input) && ReplaceClass(id, oldCode, newCode), output);
    }

    if (action == "removeClass") {
        std::string code;
        if (!(input >> code) || !AtEnd(input) || !HasCourse(code)) {
            return Status(false, output);
        }
        const std::size_t affected = RemoveClass(code);
        if (affected == 0) {
            return Status(false, output);
        }
        output << affected << '\n';
        return true;
    }

    if (action == "checkEdgeStatus") {
        std::string fromText, toText;
        int from = 0, to = 0;
        if (!(input >> fromText >> toText) || !AtEnd(input) ||
            !ReadInteger(fromText, from) || !ReadInteger(toText, to)) {
            return Status(false, output);
        }
        output << campus.EdgeStatus(from, to) << '\n';
        return true;
    }
    return Status(false, output);
}
