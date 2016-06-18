cmake_minimum_required (VERSION 3.2.0)
project(fptool)

include_directories("include")

add_executable (fptool
                include/logging.h
                include/cmdline.h
                include/csd.h
                include/ssa.h
                include/tokenizer.h
                include/reader.h
                include/parser.h
                include/pass_addsub.h
                include/pass_clean.h
                include/pass_csdmul.h
                include/vhdlcodegen.h
                src/logging.cpp
                src/cmdline.cpp
                src/reader.cpp
                src/tokenizer.cpp
                src/parser.cpp
                src/ssa.cpp
                src/csd.cpp
                src/vhdlcodegen.cpp
                src/cppcodegen.cpp
                src/pass_addsub.cpp
                src/pass_csdmul.cpp
                src/pass_clean.cpp
                src/main.cpp)
