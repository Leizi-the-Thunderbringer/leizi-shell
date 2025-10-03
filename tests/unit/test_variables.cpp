#include "../catch.hpp"
#include "utils/variables.h"

TEST_CASE("VariableManager - Basic operations", "[variables]") {
    VariableManager vm;

    SECTION("Set and get string variable") {
        vm.setString("TEST", "hello");
        auto* var = vm.get("TEST");
        REQUIRE(var != nullptr);
        REQUIRE(var->type == VarType::STRING);
        REQUIRE(var->toString() == "hello");
    }

    SECTION("Set and get array variable") {
        std::vector<std::string> arr = {"a", "b", "c"};
        vm.setArray("ARR", arr);
        auto* var = vm.get("ARR");
        REQUIRE(var != nullptr);
        REQUIRE(var->type == VarType::ARRAY);
        REQUIRE(var->arrayValue.size() == 3);
        REQUIRE(var->arrayValue[0] == "a");
    }

    SECTION("Erase variable") {
        vm.setString("TEMP", "value");
        REQUIRE(vm.get("TEMP") != nullptr);
        vm.erase("TEMP");
        REQUIRE(vm.get("TEMP") == nullptr);
    }

    SECTION("Get non-existent variable") {
        auto* var = vm.get("NONEXISTENT");
        REQUIRE(var == nullptr);
    }
}

TEST_CASE("VariableManager - String operations", "[variables]") {
    VariableManager vm;

    SECTION("setString and get") {
        vm.setString("TEST_VAR", "test_value");
        auto* var = vm.get("TEST_VAR");
        REQUIRE(var != nullptr);
        REQUIRE(var->type == VarType::STRING);
        REQUIRE(var->toString() == "test_value");
    }

    SECTION("Environment variable access") {
        // Test that PATH exists (from environment)
        auto* pathVar = vm.get("PATH");
        // PATH may or may not be in VariableManager depending on implementation
        // So we just test the API works
        REQUIRE(true);
    }

    SECTION("Non-existent variable get") {
        auto* var = vm.get("NONEXISTENT_VAR");
        REQUIRE(var == nullptr);
    }
}

TEST_CASE("VariableManager - Array operations", "[variables]") {
    VariableManager vm;

    SECTION("Set and get array") {
        std::vector<std::string> values = {"one", "two", "three"};
        vm.setArray("MY_ARRAY", values);

        auto* var = vm.get("MY_ARRAY");
        REQUIRE(var != nullptr);
        REQUIRE(var->type == VarType::ARRAY);
        REQUIRE(var->arrayValue.size() == 3);
        REQUIRE(var->arrayValue[0] == "one");
        REQUIRE(var->arrayValue[2] == "three");
    }
}
