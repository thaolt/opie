TEMPLATE	= lib
CONFIG		+= qt plugin warn_on release
HEADERS	=	autorotate.h autorotateimpl.h
SOURCES	=	autorotate.cpp autorotateimpl.cpp
TARGET		= autorotateapplet
DESTDIR		= $(OPIEDIR)/plugins/applets
INCLUDEPATH += $(OPIEDIR)/include
DEPENDPATH      += $(OPIEDIR)/include ../launcher
LIBS            += -lqpe -lopie
VERSION		= 1.0.0

include ( $(OPIEDIR)/include.pro )
target.path = $$prefix/plugins/applets
