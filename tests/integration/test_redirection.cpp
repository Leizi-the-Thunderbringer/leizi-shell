#include "../catch.hpp"
#include <cstdlib>
#include <fstream>
#include <string>

TEST_CASE("Redirection - Output", "[integration][redirection]") {
    SECTION("Basic output redirection") {
        int result = system("echo 'test output' > /tmp/leizi_test_redir.txt");
        REQUIRE(result == 0);

        std::ifstream file("/tmp/leizi_test_redir.txt");
        std::string content;
        std::getline(file, content);
        REQUIRE(content == "test output");
        file.close();
        std::remove("/tmp/leizi_test_redir.txt");
    }

    SECTION("Output append redirection") {
        system("echo 'first line' > /tmp/leizi_test_append.txt");
        system("echo 'second line' >> /tmp/leizi_test_append.txt");

        std::ifstream file("/tmp/leizi_test_append.txt");
        std::string line1, line2;
        std::getline(file, line1);
        std::getline(file, line2);
        file.close();

        REQUIRE(line1 == "first line");
        REQUIRE(line2 == "second line");
        std::remove("/tmp/leizi_test_append.txt");
    }
}

TEST_CASE("Redirection - Input", "[integration][redirection]") {
    SECTION("Input redirection") {
        // 创建输入文件
        std::ofstream outfile("/tmp/leizi_test_input.txt");
        outfile << "input content\n";
        outfile.close();

        // 使用输入重定向
        int result = system("cat < /tmp/leizi_test_input.txt > /tmp/leizi_test_output.txt");
        REQUIRE(result == 0);

        // 验证输出
        std::ifstream infile("/tmp/leizi_test_output.txt");
        std::string content;
        std::getline(infile, content);
        REQUIRE(content == "input content");
        infile.close();

        std::remove("/tmp/leizi_test_input.txt");
        std::remove("/tmp/leizi_test_output.txt");
    }
}

TEST_CASE("Redirection - Error", "[integration][redirection]") {
    SECTION("Error redirection") {
        // 这个命令会产生错误
        int result = system("ls /nonexistent_directory_12345 2> /tmp/leizi_test_error.txt");
        // ls 会返回非零退出码
        REQUIRE(result != 0);

        // 错误输出应该被重定向到文件
        std::ifstream file("/tmp/leizi_test_error.txt");
        REQUIRE(file.good());
        std::string content;
        std::getline(file, content);
        // 应该包含错误信息
        REQUIRE(content.length() > 0);
        file.close();
        std::remove("/tmp/leizi_test_error.txt");
    }
}
