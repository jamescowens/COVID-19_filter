TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
DEFINES += BOOST_THREAD_USE_LIB
INCLUDEPATH += /home/jco/local/boost/include
DEPENDPATH += /home/jco/local/boost/lib
LIBS += -L/home/jco/local/boost/lib -lboost_system -lboost_filesystem -lboost_program_options -lboost_date_time -lboost_chrono
QMAKE_CXXFLAGS += -std=c++11
SOURCES += \
        main.cpp

DISTFILES += \
    README.md
