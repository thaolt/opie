
/*
 * ioctl table - generated by regen.py - (C) Michael 'Mickey' Lauer <mickey@vanille.de>
 */

#ifndef IOCTLMAP_H
#define IOCTLMAP_H

#include <qstring.h>
#include <qintdict.h>

typedef QIntDict<QString> IntStringMap;

IntStringMap* constructIoctlMap();

#endif

