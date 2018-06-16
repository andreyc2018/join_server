add_executable(async_read_until async_read_until.cpp)

target_compile_options(async_read_until PRIVATE -Wpedantic -Wall -Wextra -I${CMAKE_SOURCE_DIR})

