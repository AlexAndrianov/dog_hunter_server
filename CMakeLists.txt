cmake_minimum_required(VERSION 3.14)

project(dog_hunter_server LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

find_package(SQLite3 REQUIRED)
find_package(GTest REQUIRED)

# Add the library subdirectory
add_subdirectory(dog_hunter_core)

add_executable(dog_hunter_server
  main.cpp
  TCPServer.h
  Database.h
)

target_link_libraries(dog_hunter_server Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network core SQLite::SQLite3)

include(GNUInstallDirs)
install(TARGETS dog_hunter_server
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_executable(RunTests TCPServer.h Database.h Tests.cpp)

target_link_libraries(RunTests Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network core SQLite::SQLite3 gtest::gtest)

# Add tests
include(GoogleTest)
gtest_discover_tests(RunTests)
