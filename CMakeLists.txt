cmake_minimum_required(VERSION 3.15)

project(libintelhex VERSION 1.0
                    DESCRIPTION "Intel hex C++ parsing library"
                    LANGUAGES CXX)

# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/catch)
add_library(Catch2::Catch IMPORTED INTERFACE)
target_include_directories(Catch2::Catch INTERFACE ${CATCH_INCLUDE_DIR})

add_library(intelhex src/intelhex.cpp)
target_compile_features(intelhex PUBLIC cxx_std_17)
target_include_directories(intelhex PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)


set(TESTS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
set(TESTS_SOURCE ${TESTS_SOURCE_DIR}/main.cpp
                 ${TESTS_SOURCE_DIR}/tests.cpp
                 ${TESTS_SOURCE_DIR}/../src/intelhex.cpp)

add_executable(tests ${TESTS_SOURCE})

target_link_libraries(tests Catch2::Catch intelhex)
