cmake_minimum_required(VERSION 3.2)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(BuildType)

if ($ENV{TRAVIS_BUILD_NUMBER})
    set(BUILD_NUM $ENV{TRAVIS_BUILD_NUMBER})
else()
    set(BUILD_NUM 99)
endif()

set(homework_VERSION_MAJOR 13)
set(homework_VERSION_MINOR 0)
set(homework_VERSION_BUILD ${BUILD_NUM})

project(join_server VERSION
        ${homework_VERSION_MAJOR}.${homework_VERSION_MINOR}.${homework_VERSION_BUILD})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(THREADS_PREFER_PTHREAD_FLAG ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wpedantic -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wpedantic -Wall -Wextra")

find_package(Threads REQUIRED)

configure_file(version.h.in version.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/include
        ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googlemock/include
        ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include)

set(HEADER_FILES
        include/commands.h
        include/interpreter.h
        include/logger.h
        include/processor.h
        include/server.h
        include/storage.h)

add_library(server STATIC
        src/commands.cpp
        src/interpreter.cpp
        src/logger.cpp
        src/processor.cpp
        src/server.cpp
        src/storage.cpp
        ${HEADER_FILES})

add_executable(join_server
    src/main.cpp
    include/server.h
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
    README.md)

add_executable(test_version src/test_server.cpp)

target_link_libraries(join_server server
        Threads::Threads)
target_link_libraries(test_version server gmock_main
        Threads::Threads)

install(TARGETS join_server RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_CONTACT "sysdos286@yahoo.com")
set(CPACK_PACKAGE_VERSION_MAJOR ${homework_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${homework_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${homework_VERSION_BUILD})

include(CPack)

enable_testing()

add_test(NAME test_version COMMAND test_version)

if(EXISTS ${CMAKE_SOURCE_DIR}/sandbox/CMakeLists.txt)
    add_subdirectory(sandbox EXCLUDE_FROM_ALL)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/googletest/CMakeLists.txt)
    # Replace install() to do-nothing macro.
    macro(install)
    endmacro()
    # Include subproject (or any other CMake code) with "disabled" install().
    message(STATUS "Adding Google Test framework.")
    add_subdirectory(googletest)
    # Restore original install() behavior.
    macro(install)
        _install(${ARGN})
    endmacro()
endif()

message(STATUS "CMAKE_CXX_FLAGS_RELEASE = ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG = ${CMAKE_CXX_FLAGS_DEBUG}")

message(STATUS "Homework version
    ${homework_VERSION_MAJOR}.${homework_VERSION_MINOR}.${homework_VERSION_BUILD}")

