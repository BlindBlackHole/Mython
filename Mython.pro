TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        comparators.cpp \
        lexer.cpp \
        main.cpp \
        object.cpp \
        object_holder.cpp \
        object_holder_test.cpp \
        object_test.cpp \
        parse.cpp \
        parse_test.cpp \
        statement.cpp \
        statement_test.cpp

HEADERS += \
    comparators.h \
    lexer.h \
    lexer_test.h \
    object.h \
    object_holder.h \
    parse.h \
    statement.h \
    test_runner.h
