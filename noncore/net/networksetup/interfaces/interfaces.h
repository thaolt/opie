#ifndef INTERFACES_H
#define INTERFACES_H

#include <qstring.h>
#include <qstringlist.h>

#define INTERFACES_LOOPBACK "loopback"

#define INTERFACES_FAMILY_INET "inet"
#define INTERFACES_FAMILY_IPX "ipx"
#define INTERFACES_FAMILY_INET6 "inet6"

#define INTERFACES_METHOD_DHCP "dhcp"
#define INTERFACES_METHOD_STATIC "static"
#define INTERFACES_METHOD_PPP "ppp"

/**
 * This class provides a clean frontend for parsing the network interfaces file.
 * It provides helper functions to minipulate the options within the file.
 * See the interfaces man page for the syntax rules.
 */
class Interfaces {

public:
  Interfaces(QString useInterfacesFile = "/etc/network/interfaces");
  QStringList getInterfaceList();
  
  bool isAuto(const QString &interface);
  bool setAuto(const QString &interface, bool setAuto);
  
  inline bool removeInterface();
  bool addInterface(const QString &interface, const QString &family, const QString &method);
  bool copyInterface(const QString &oldInterface, const QString &newInterface);
  bool setInterface(QString interface);
  inline bool isInterfaceSet();
  QString getInterfaceName(bool &error);
  bool setInterfaceName(const QString &newName);
  QString getInterfaceFamily(bool &error);
  bool setInterfaceFamily(const QString &newName);
  QString getInterfaceMethod(bool &error);
  bool setInterfaceMethod(const QString &newName);
  inline QString getInterfaceOption(const QString &option, bool &error); 
  inline bool setInterfaceOption(const QString &option, const QString &value);
  inline bool removeInterfaceOption(const QString &option, const QString &value);
  inline bool removeAllInterfaceOptions();
  
  bool setMapping(const QString &interface);
  inline bool removeMapping();
  inline void addMapping(const QString &options);
  inline bool setMap(const QString &map, const QString &value);
  inline bool removeMap(const QString &map, const QString &value);
  inline QString getMap(const QString &map, bool &error);
  inline bool setScript(const QString &argument);
  inline QString getScript(bool &error);
		  
  bool write();
  
private:
  bool setStanza(const QString &stanza, const QString &option, QStringList::Iterator &iterator);
  bool setOption(const QStringList::Iterator &start, const QString &option, const QString &value);
  bool removeOption(const QStringList::Iterator &start, const QString &option, const QString &value);
  QString getOption(const QStringList::Iterator &start, const QString &option, bool &error);
  bool removeStanza(QStringList::Iterator &stanza);
  bool removeAllOptions(const QStringList::Iterator &start);
  
  QString interfacesFile;
  QStringList interfaces;
  QStringList::Iterator currentIface;
  QStringList::Iterator currentMapping;

  QStringList acceptedFamily;
};

#endif

// interfaces

