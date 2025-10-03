#include "../catch.hpp"
#include "builtin/builtin_manager.h"
#include "builtin/builtin_context.h"
#include "utils/variables.h"
#include "core/parser.h"

TEST_CASE("BuiltinManager - Command registration", "[builtin]") {
    BuiltinManager manager;

    SECTION("Check builtin commands exist") {
        REQUIRE(manager.isBuiltin("cd") == true);
        REQUIRE(manager.isBuiltin("echo") == true);
        REQUIRE(manager.isBuiltin("pwd") == true);
        REQUIRE(manager.isBuiltin("exit") == true);
        REQUIRE(manager.isBuiltin("help") == true);
        REQUIRE(manager.isBuiltin("version") == true);
        REQUIRE(manager.isBuiltin("export") == true);
        REQUIRE(manager.isBuiltin("unset") == true);
        REQUIRE(manager.isBuiltin("array") == true);
        REQUIRE(manager.isBuiltin("history") == true);
        REQUIRE(manager.isBuiltin("clear") == true);
    }

    SECTION("Check non-builtin command") {
        REQUIRE(manager.isBuiltin("nonexistent") == false);
        REQUIRE(manager.isBuiltin("ls") == false);
        REQUIRE(manager.isBuiltin("cat") == false);
    }

    SECTION("Get command names") {
        auto names = manager.getCommandNames();
        REQUIRE(names.size() >= 11);  // At least 11 builtin commands

        // Check that some expected commands are in the list
        bool hasEcho = false;
        bool hasCd = false;
        for (const auto& name : names) {
            if (name == "echo") hasEcho = true;
            if (name == "cd") hasCd = true;
        }
        REQUIRE(hasEcho == true);
        REQUIRE(hasCd == true);
    }
}

TEST_CASE("BuiltinManager - Command execution", "[builtin]") {
    BuiltinManager manager;
    VariableManager variables;
    CommandParser parser;
    std::vector<std::string> history;
    std::string currentDir = "/tmp";
    std::string homeDir = "/home/test";
    int exitCode = 0;
    bool exitRequested = false;
    std::string histFile = "test_history";

    auto expandFunc = [](const std::string& s) { return s; };

    BuiltinContext context(
        variables, parser, history, currentDir, homeDir,
        exitCode, exitRequested, histFile, expandFunc
    );

    SECTION("Execute pwd command") {
        std::vector<std::string> args = {"pwd"};
        auto result = manager.execute(args, context);
        REQUIRE(result.exitCode == 0);
    }

    SECTION("Execute clear command") {
        std::vector<std::string> args = {"clear"};
        auto result = manager.execute(args, context);
        REQUIRE(result.exitCode == 0);
    }

    SECTION("Execute exit command") {
        std::vector<std::string> args = {"exit", "0"};
        auto result = manager.execute(args, context);
        REQUIRE(result.shouldExit == true);
        REQUIRE(context.exitRequested == true);
    }
}
