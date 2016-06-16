cmake_minimum_required (VERSION 3.2.0)
project(fptool)

include_directories("include")

add_executable (fptool
                src/reader.cpp
                src/tokenizer.cpp
                src/parser.cpp
                src/ssa.cpp
                src/csd.cpp
                src/vhdlcodegen.cpp
                src/pass_addsub.cpp
                src/pass_csdmul.cpp
                src/pass_clean.cpp
                src/main.cpp)
