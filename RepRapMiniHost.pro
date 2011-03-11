TEMPLATE = app
TARGET = RepRapMiniHost
QT += core \
    gui
HEADERS += RepRapHost.h \
    BoostComPort.hpp \
    RepRapMiniHost.h
SOURCES += RepRapHost.cpp \
    BoostComPort.cpp \
    main.cpp \
    RepRapMiniHost.cpp
FORMS += RepRapMiniHost.ui
RESOURCES += 
LIBS += -lboost_system -lboost_regex
