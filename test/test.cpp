#ifdef CAMPUS_BUNDLED_CATCH
#include "catch/catch_amalgamated.hpp"
#else
#include <catch2/catch_test_macros.hpp>
#endif

#include "CampusCompass.h"
#include <sstream>
#include <string>
#include <vector>

namespace {
std::string Run(CampusCompass& compass, const std::string& command) {
    std::ostringstream output;
    compass.ParseCommand(command, output);
    return output.str();
}
} // namespace

TEST_CASE("Rubric 1 - invalid commands leave no partial records", "[rubric]") {
    CampusCompass compass;
    REQUIRE(compass.ParseCSV("data/edges.csv", "data/classes.csv"));
    const std::vector<std::string> invalid{
        "insert \"A11y\" 45679999 1 1 COP3530",
        "insert \"Brandon\" 4567999 20 1 COP3530",
        "insert \"Brandon\" 45679999 20 1 cop3530",
        "insert \"Brandon\" 45679999 20 2 COP3530",
        "remov 45679999",
        "insert \"Brandon\" 45679999 20 1 ZZZ9999"
    };
    for (const auto& command : invalid) {
        INFO(command);
        REQUIRE(Run(compass, command) == "unsuccessful\n");
    }
    REQUIRE(Run(compass, "insert \"Brandon\" 45679999 20 1 COP3530") == "successful\n");
}

TEST_CASE("Rubric 2 - edge cases across student functions", "[rubric]") {
    CampusCompass compass;
    REQUIRE(compass.ParseCSV("data/edges.csv", "data/classes.csv"));
    REQUIRE(Run(compass, "remove 12345678") == "unsuccessful\n");
    REQUIRE(Run(compass, "insert \"Brian\" 35459999 21 1 MAC2311") == "successful\n");
    REQUIRE(Run(compass, "dropClass 35459999 MAC2311") == "successful\n");
    REQUIRE(Run(compass, "remove 35459999") == "unsuccessful\n");
    REQUIRE(Run(compass, "removeClass COP3530") == "unsuccessful\n");
}

TEST_CASE("Rubric 3 - replace drop removeClass and remove", "[rubric]") {
    CampusCompass compass;
    REQUIRE(compass.ParseCSV("data/edges.csv", "data/classes.csv"));
    REQUIRE(Run(compass, "insert \"Brandon\" 45679999 20 2 COP3530 MAC2311") == "successful\n");
    REQUIRE(Run(compass, "insert \"Brian\" 35459999 21 3 COP3530 CDA3101 MAC2311") == "successful\n");
    REQUIRE(Run(compass, "replaceClass 45679999 MAC2311 CDA3101") == "successful\n");
    REQUIRE(Run(compass, "dropClass 35459999 COP3530") == "successful\n");
    REQUIRE(Run(compass, "removeClass CDA3101") == "2\n");
    REQUIRE(Run(compass, "printShortestEdges 45679999") ==
            "Time For Shortest Edges: Brandon\nCOP3530: 20\n");
    REQUIRE(Run(compass, "printShortestEdges 35459999") ==
            "Time For Shortest Edges: Brian\nMAC2311: 11\n");
    REQUIRE(Run(compass, "remove 35459999") == "successful\n");
    REQUIRE(Run(compass, "remove 35459999") == "unsuccessful\n");
}

TEST_CASE("Rubric 4 - closures make a reachable class unreachable", "[rubric]") {
    CampusCompass compass;
    REQUIRE(compass.ParseCSV("data/edges.csv", "data/classes.csv"));
    REQUIRE(Run(compass, "insert \"Brian\" 35459999 21 1 MAC2311") == "successful\n");
    REQUIRE(Run(compass, "printShortestEdges 35459999") ==
            "Time For Shortest Edges: Brian\nMAC2311: 11\n");
    // the supplied edges.csv has exactly two roads incident to location 21.
    REQUIRE(Run(compass, "toggleEdgesClosure 2 12 21 21 22") == "successful\n");
    REQUIRE(Run(compass, "printShortestEdges 35459999") ==
            "Time For Shortest Edges: Brian\nMAC2311: -1\n");
    REQUIRE(Run(compass, "toggleEdgesClosure 2 21 12 22 21") == "successful\n");
    REQUIRE(Run(compass, "printShortestEdges 35459999") ==
            "Time For Shortest Edges: Brian\nMAC2311: 11\n");
}
