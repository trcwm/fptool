TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += include
INCLUDEPATH += externals/fplib/src

HEADERS += include/cmdline.h \
           include/utils.h \
           include/cppcodegen.h \
           include/csd.h \
           include/logging.h \
           include/parser.h \
           include/pass_addsub.h \
           include/pass_truncate.h \
           include/pass_clean.h \
           include/pass_csdmul.h \
           include/astgraphviz.h \
           include/reader.h \
           include/ssa.h \
           include/ssapass.h \
           include/tokenizer.h \
           include/vhdlcodegen.h \
           include/astnode.h \
           include/astvisitor.h \
           include/ssacreator.h \
           include/ssaprint.h \
           include/ssaevaluator.h \
           externals/fplib/src/fplib.h

SOURCES += src/cmdline.cpp \
           src/utils.cpp \
           src/cppcodegen.cpp \
           src/csd.cpp \
           src/logging.cpp \
           src/main.cpp \
           src/parser.cpp \
           src/pass_addsub.cpp \
           src/pass_truncate.cpp \
           src/pass_clean.cpp \
           src/pass_csdmul.cpp \
           src/astgraphviz.cpp \
           src/reader.cpp \
           src/ssa.cpp \
           src/tokenizer.cpp \
           src/vhdlcodegen.cpp \
           src/astvisitor.cpp \
           src/ssacreator.cpp \
           src/ssaprint.cpp \
           src/ssaevaluator.cpp \
           externals/fplib/src/fplib.cpp
