TEMPLATE	= app
CONFIG		= qt warn_on release
DESTDIR		= $(OPIEDIR)/bin
HEADERS	= addressbook.h \
                  contacteditor.h \
		  ablabel.h \
		  abtable.h \
		  addresssettings.h
SOURCES	= main.cpp \
		  addressbook.cpp \
		  contacteditor.cpp \
		  ablabel.cpp \
		  abtable.cpp \
		  addresssettings.cpp
INTERFACES	= addresssettingsbase.ui

TARGET		= addressbook
INCLUDEPATH += $(OPIEDIR)/include
DEPENDPATH	+= $(OPIEDIR)/include
LIBS            += -lqpe

TRANSLATIONS = ../i18n/de/addressbook.ts
TRANSLATIONS += ../i18n/fr/addressbook.ts
TRANSLATIONS += ../i18n/pt_BR/addressbook.ts
