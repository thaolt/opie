TEMPLATE    = lib
#CONFIG      += qt warn_on debug
CONFIG      += qt warn_on release
DESTDIR     = $(OPIEDIR)/lib
HEADERS     = osqlbackend.h      \
                osqldriver.h     \
                osqlerror.h      \
                osqlmanager.h    \
                osqlquery.h      \
                osqlresult.h     \
                osqltable.h      \
                osqlbackendmanager.h \
                osqlitedriver.h
SOURCES     = osqlbackend.cpp    \
                osqldriver.cpp   \
                osqlerror.cpp    \
                osqlmanager.cpp  \
                osqlquery.cpp    \
                osqlresult.cpp   \
                osqltable.cpp    \
                osqlbackendmanager.cpp \
                osqlitedriver.cpp
INTERFACES  =
TARGET      = opiedb2
VERSION     = 1.8.2
INCLUDEPATH += $(OPIEDIR)/include
DEPENDPATH  += $(OPIEDIR)/include

MOC_DIR     = moc
OBJECTS_DIR = obj

LIBS += -lsqlite -lqpe

!contains( platform, x11 ) {
  include ( $(OPIEDIR)/include.pro )
}

contains( platform, x11 ) {
  LIBS        += -L$(OPIEDIR)/lib -Wl,-rpath,$(OPIEDIR)/lib 
}

