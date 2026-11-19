cmake_minimum_required(VERSION 3.10)
project(LibraryManager LANGUAGES CXX)

# Enforce C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include header directory
include_directories(include)

# Common domain source files
set(CORE_SOURCES
    src/Book.cpp
    src/Member.cpp
    src/Loan.cpp
    src/Library.cpp
    src/Storage.cpp
    src/DateUtils.cpp
)

# Main Application Executable
add_executable(library_manager
    src/main.cpp
    ${CORE_SOURCES}
)

# Unit Test Executable
add_executable(test_library
    tests/test_library.cpp
    ${CORE_SOURCES}
)

# Optional test runner target
enable_testing()
add_test(NAME LibraryManagerTestSuite COMMAND test_library)
