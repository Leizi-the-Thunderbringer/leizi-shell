#define CATCH_CONFIG_MAIN
#include "../catch.hpp"
#include "core/parser.h"

TEST_CASE("CommandParser - Basic parsing", "[parser]") {
    CommandParser parser;

    SECTION("Empty command") {
        auto result = parser.parseCommand("");
        REQUIRE(result.empty());
    }

    SECTION("Simple command") {
        auto result = parser.parseCommand("echo hello");
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == "echo");
        REQUIRE(result[1] == "hello");
    }

    SECTION("Command with quotes") {
        auto result = parser.parseCommand("echo \"hello world\"");
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == "echo");
        REQUIRE(result[1] == "hello world");
    }

    SECTION("Command with single quotes") {
        auto result = parser.parseCommand("echo 'hello world'");
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == "echo");
        REQUIRE(result[1] == "hello world");
    }

    SECTION("Multiple arguments") {
        auto result = parser.parseCommand("ls -la /tmp");
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == "ls");
        REQUIRE(result[1] == "-la");
        REQUIRE(result[2] == "/tmp");
    }
}

TEST_CASE("CommandParser - Pipeline parsing", "[parser]") {
    CommandParser parser;

    SECTION("Simple pipeline") {
        auto result = parser.parsePipeline("ls | grep test");
        REQUIRE(result.size() == 2);
        REQUIRE(result[0].size() == 1);
        REQUIRE(result[0][0] == "ls");
        REQUIRE(result[1].size() == 2);
        REQUIRE(result[1][0] == "grep");
        REQUIRE(result[1][1] == "test");
    }

    SECTION("Multi-stage pipeline") {
        auto result = parser.parsePipeline("cat file | sort | uniq");
        REQUIRE(result.size() == 3);
        REQUIRE(result[0][0] == "cat");
        REQUIRE(result[1][0] == "sort");
        REQUIRE(result[2][0] == "uniq");
    }
}

TEST_CASE("CommandParser - Special characters", "[parser]") {
    CommandParser parser;

    SECTION("Redirection operators") {
        auto result = parser.parseCommand("echo test > file.txt");
        REQUIRE(result.size() == 4);
        REQUIRE(result[2] == ">");
    }

    SECTION("Background operator") {
        auto result = parser.parseCommand("sleep 10 &");
        REQUIRE(result.size() == 3);
        REQUIRE(result[2] == "&");
    }
}
