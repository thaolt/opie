SOURCES	+= chess.cpp main.cpp 
HEADERS	+= chess.h 
DESTDIR = ../bin
TARGET = chess
DEPENDPATH	+= $(OPIEDIR)/include
INTERFACES = mainwindow.ui 
IMAGES	= images/new.png images/repeat.png images/txt.png images/back.png 
TEMPLATE	=app
CONFIG	+= qt warn_on release
INCLUDEPATH += $(OPIEDIR)/include
LIBS	+= -lqpe
DBFILE	= chess.db
LANGUAGE	= C++
CPP_ALWAYS_CREATE_SOURCE	= TRUE

TRANSLATIONS = ../i18n/pt_BR/chess.ts
