TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += include

HEADERS += include/cmdline.h \
           include/cppcodegen.h \
           include/csd.h \
           include/logging.h \
           include/parser.h \
           include/pass_addsub.h \
           include/pass_clean.h \
           include/pass_csdmul.h \
           include/pass_graphviz.h \
           include/reader.h \
           include/ssa.h \
           include/tokenizer.h \
           include/vhdlcodegen.h

SOURCES += src/cmdline.cpp \
           src/cppcodegen.cpp \
           src/csd.cpp \
           src/logging.cpp \
           src/main.cpp \
           src/parser.cpp \
           src/pass_addsub.cpp \
           src/pass_clean.cpp \
           src/pass_csdmul.cpp \
           src/pass_graphviz.cpp \
           src/reader.cpp \
           src/ssa.cpp \
           src/tokenizer.cpp \
           src/vhdlcodegen.cpp



