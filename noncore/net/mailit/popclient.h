/**********************************************************************
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Palmtop Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef PopClient_H
#define PopClient_H

#include <stdio.h>
#include <qsocket.h>
#include <qstring.h>
#include <qobject.h>
#include <qtextstream.h>
#include <qlist.h>
#include "maillist.h"

class PopClient: public QObject
{
  Q_OBJECT
  
public:
  PopClient();
  ~PopClient();
  void newConnection(const QString &target, int port);
  void setAccount(const QString &popUser, const QString &popPasswd);
  void setSynchronize(int lastCount);
  void removeSynchronize();
  void headersOnly(bool headers, int limit);
  void setSelectedMails(MailList *list);
  
signals:
  void newMessage(const QString &, int, uint, bool);
  void errorOccurred(int status, const QString & Msg );
  void updateStatus(const QString &);
  void mailTransfered(int);
  void mailboxSize(int);
  void currentMailSize(int);
  void downloadedSize(int);
    
public slots:
  void errorHandling(int);
  void errorHandlingWithMsg(int, const QString & );

protected slots:
  void connectionEstablished();
  void incomingData();
  
private:
  QSocket *socket;
  QTextStream *stream;
  enum transferStatus
  {
    Init, Pass, Stat, Mcnt, Read, List, Size, Retr, Acks,
    Quit, Done, Ignore
  };
  int status, lastSync;
  int messageCount, newMessages, mailSize, headerLimit;
  bool receiving, synchronize, preview, selected;
  QString popUserName, popPassword, message;
  MailList *mailList;
};

#endif
