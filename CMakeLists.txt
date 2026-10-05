cmake_minimum_required(VERSION 3.20)
project(HybridPricingEngine VERSION 0.1.0 LANGUAGES CXX)

# C++ standard strict (-std=c++20, pas -std=gnu++20)
set(CMAKE_CXX_EXTENSIONS OFF)
# Génère compile_commands.json pour l'autocomplétion de VS Code
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()
add_subdirectory(cpp)