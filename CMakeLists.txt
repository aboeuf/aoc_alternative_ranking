cmake_minimum_required(VERSION 3.5)

project(aoc_alternative_ranking LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core Network)

include_directories("./src")

add_executable(aoc_alternative_ranking
  src/leaderboard.cpp
  src/manager.cpp
  src/main.cpp
)

target_link_libraries(aoc_alternative_ranking Qt5::Core Qt5::Network)
