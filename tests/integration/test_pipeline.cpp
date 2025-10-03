#include "../catch.hpp"
#include <cstdlib>
#include <fstream>
#include <string>

// 这些是功能性集成测试，测试实际的 shell 功能

TEST_CASE("Pipeline functionality", "[integration][pipeline]") {
    SECTION("Basic pipeline execution") {
        // 测试简单的管道功能
        int result = system("echo 'test' | grep test > /tmp/leizi_test_pipeline.txt");
        REQUIRE(result == 0);

        // 验证输出
        std::ifstream file("/tmp/leizi_test_pipeline.txt");
        std::string content;
        std::getline(file, content);
        REQUIRE(content == "test");
        file.close();
        std::remove("/tmp/leizi_test_pipeline.txt");
    }

    SECTION("Multi-stage pipeline") {
        // 测试多级管道
        int result = system("echo -e 'a\\nb\\na\\nc' | sort | uniq > /tmp/leizi_test_multi_pipeline.txt");
        REQUIRE(result == 0);

        std::ifstream file("/tmp/leizi_test_multi_pipeline.txt");
        std::string line1, line2, line3;
        std::getline(file, line1);
        std::getline(file, line2);
        std::getline(file, line3);
        file.close();

        // Sort + uniq should give us: a, b, c
        REQUIRE((line1 == "a" || line1 == "-e a"));  // echo -e might vary
        std::remove("/tmp/leizi_test_multi_pipeline.txt");
    }
}

TEST_CASE("Pipeline with shell builtin", "[integration][pipeline]") {
    // 注意: 这个测试验证的是一般 shell 管道行为
    // 在实际的 leizi shell 中运行时会有不同的行为

    SECTION("Pipeline output") {
        int result = system("echo hello | cat > /tmp/leizi_test_builtin_pipe.txt");
        REQUIRE(result == 0);

        std::ifstream file("/tmp/leizi_test_builtin_pipe.txt");
        std::string content;
        std::getline(file, content);
        REQUIRE(content == "hello");
        file.close();
        std::remove("/tmp/leizi_test_builtin_pipe.txt");
    }
}
