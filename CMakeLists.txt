cmake_minimum_required(VERSION 3.10)

# Set the project name
project(PasswordManager)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)s

# Add the executable
add_executable(passwordmanager src/main.cpp)

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})