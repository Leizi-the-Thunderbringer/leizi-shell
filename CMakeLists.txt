cmake_minimum_required(VERSION 3.16)
project(leizi-shell VERSION 1.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -fsanitize=address -fsanitize=undefined")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()

# Platform-specific settings
if(APPLE)
    # macOS specific settings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

    # Add Homebrew paths
    if(EXISTS /opt/homebrew)
        include_directories(/opt/homebrew/include)
        link_directories(/opt/homebrew/lib)
    endif()

    if(EXISTS /usr/local)
        include_directories(/usr/local/include)
        link_directories(/usr/local/lib)
    endif()
endif()

# Find packages
find_package(PkgConfig QUIET)

# Try to find readline
find_path(READLINE_INCLUDE_DIR
        NAMES readline/readline.h
        PATHS /usr/include /usr/local/include /opt/homebrew/include
)

find_library(READLINE_LIBRARY
        NAMES readline
        PATHS /usr/lib /usr/local/lib /opt/homebrew/lib
)

if(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
    set(HAVE_READLINE TRUE)
    message(STATUS "Found readline: ${READLINE_LIBRARY}")
else()
    set(HAVE_READLINE FALSE)
    message(STATUS "Readline not found, using simple input method")
endif()

# Main executable
add_executable(leizi src/main.cpp)

# Set target properties
set_target_properties(leizi PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
)

# Link libraries
if(HAVE_READLINE)
    target_include_directories(leizi PRIVATE ${READLINE_INCLUDE_DIR})
    target_link_libraries(leizi ${READLINE_LIBRARY})

    # On some systems, we also need ncurses
    find_library(NCURSES_LIBRARY ncurses)
    if(NCURSES_LIBRARY)
        target_link_libraries(leizi ${NCURSES_LIBRARY})
    endif()
endif()

# Install targets
install(TARGETS leizi
        RUNTIME DESTINATION bin
)

# Install man page (if exists)
if(EXISTS ${CMAKE_SOURCE_DIR}/docs/leizi.1)
    install(FILES docs/leizi.1
            DESTINATION share/man/man1
    )
endif()

# Testing
enable_testing()

# Add tests if test directory exists
if(EXISTS ${CMAKE_SOURCE_DIR}/tests)
    add_subdirectory(tests)
endif()

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "leizi-shell")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A modern POSIX-compatible shell")
set(CPACK_PACKAGE_VENDOR "Leizi Shell Project")
set(CPACK_PACKAGE_CONTACT "your-email@example.com")

set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_SOURCE_IGNORE_FILES
        "/build/"
        "/cmake-build-*/"
        "/.git/"
        "/.github/"
        "/\..*$"
        ".*~$"
)

if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "DEB;RPM")

    # DEB package settings
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libstdc++6")
    set(CPACK_DEBIAN_PACKAGE_SECTION "shells")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

    # RPM package settings
    set(CPACK_RPM_PACKAGE_GROUP "System Environment/Shells")
    set(CPACK_RPM_PACKAGE_REQUIRES "glibc, libstdc++")
elseif(APPLE)
    set(CPACK_GENERATOR "productbuild")
endif()

include(CPack)

# Print configuration summary
message(STATUS "")
message(STATUS "Leizi Shell Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Readline support: ${HAVE_READLINE}")
if(HAVE_READLINE)
    message(STATUS "  Readline library: ${READLINE_LIBRARY}")
endif()
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")