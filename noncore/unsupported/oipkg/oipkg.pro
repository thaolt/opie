DESTDIR		= $(OPIEDIR)/bin
TEMPLATE	= app
#CONFIG		= qt warn_on release
CONFIG		= qt warn_on debug
HEADERS		= mainwindow.h \
		pksettings.h \
		pmipkg.h \
		packagelistitem.h \
		packagelist.h \
		package.h
SOURCES		= main.cpp \
		mainwindow.cpp \
		pksettings.cpp \
		pmipkg.cpp \
		packagelistitem.cpp \
		packagelist.cpp \
		package.cpp
INCLUDEPATH += $(OPIEDIR)/include
DEPENDPATH	+= $(OPIEDIR)/ioclude
LIBS            += -lqpe 
INTERFACES	= pkdesc.ui \
		runwindow.ui \
		pksettingsbase.ui
TARGET		= oipkg

