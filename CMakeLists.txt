cmake_minimum_required (VERSION 2.8.0)
project(fptool)

include_directories("include")

add_executable (fptool src/reader.cpp src/tokenizer.cpp src/parser.cpp src/ssa.cpp src/main.cpp)