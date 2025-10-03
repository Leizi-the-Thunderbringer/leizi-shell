# 测试配置
cmake_minimum_required(VERSION 3.16)

# 包含 Catch2 头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 单元测试
add_executable(unit_tests
    unit/test_parser.cpp
    unit/test_variables.cpp
    unit/test_builtin.cpp
    ../src/utils/variables.cpp
    ../src/core/parser.cpp
    ../src/builtin/builtin_manager.cpp
    ../src/builtin/cd.cpp
    ../src/builtin/echo.cpp
    ../src/builtin/export.cpp
    ../src/builtin/array.cpp
    ../src/builtin/history.cpp
    ../src/builtin/simple.cpp
    ../src/builtin/info.cpp
    ../src/builtin/highlight.cpp
    ../src/syntax/highlighter.cpp
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(unit_tests PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# 集成测试
add_executable(integration_tests
    integration/test_main.cpp
    integration/test_pipeline.cpp
    integration/test_redirection.cpp
    integration/test_jobs.cpp
)

target_include_directories(integration_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(integration_tests PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# 添加测试
enable_testing()
add_test(NAME UnitTests COMMAND unit_tests)
add_test(NAME IntegrationTests COMMAND integration_tests)
