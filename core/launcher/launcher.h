/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.**
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
#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <qpe/config.h>
#include <qpe/applnk.h>

#include <opie/ofileselector.h>

#include <qmainwindow.h>
#include <qtabbar.h>
#include <qstringlist.h>
#include <qvbox.h>
#include <qlist.h>
#include <qpe/palmtopuidgen.h>
#include "launcherview.h"

class AppLnk;
class AppLnkSet;
class DocLnkSet;
class QWidgetStack;
class StorageInfo;
class SyncDialog;


class DocumentTab : public OFileSelector
{
  Q_OBJECT
public:
 DocumentTab( QWidget *parent, int mode, int selector, const QString &dirName, const QString &fileName);//, 4,0, "/","");
~DocumentTab();
private:

};

class CategoryTabBar : public QTabBar
{
    Q_OBJECT
public:
    CategoryTabBar( QWidget *parent=0, const char *name=0 );
    ~CategoryTabBar();

signals:

protected slots:
    virtual void layoutTabs();

protected:
    void paint ( QPainter *p, QTab *t, bool f ) const;
    void paintLabel( QPainter* p, const QRect& br, QTab* t, bool has_focus ) const;
   
};

class CategoryTabWidget : public QVBox {
    // can't use a QTabWidget, since it won't let us set the frame style.
    Q_OBJECT
public:
    CategoryTabWidget( QWidget* parent );
    void initializeCategories(AppLnkSet* rootFolder, AppLnkSet* docFolder,
      const QList<FileSystem> &);
//    void updateDocs(AppLnkSet* docFolder, const QList<FileSystem> &fs);
    void updateLink(const QString& linkfile);
    void setBusy(bool on);
    QString getAllDocLinkInfo() const;
    LauncherView *view( const QString &id );
    void setBusyIndicatorType ( const QString &type );
    DocumentTab *fileSel;
signals:
    void selected(const QString&);
    void clicked(const AppLnk*);
    void rightPressed(AppLnk*);
    void ofileSel(const DocLnk &);
public slots:
    void nextTab();
    void prevTab();
    void showTab(const QString&);
   void clickie(const DocLnk&);
   void clickie(const QString &);
protected slots:
    void tabProperties();

protected:
    void setTabAppearance( const QString &id, Config &cfg );
    void paletteChange( const QPalette &p );

private:
    CategoryTabBar* categoryBar;
    QWidgetStack* stack;
    LauncherView* docview;
    QStringList ids;
    int tabs;
    LauncherView* newView( const QString&, const QPixmap& pm, const QString& label );
    void addItem( const QString& );
};

class Launcher : public QMainWindow
{
    Q_OBJECT
    friend class LauncherPrivate;
public:
    Launcher( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~Launcher();

    static QString appsFolderName();

    virtual void showMaximized();
    static bool mkdir(const QString &path);

public slots:
    void viewSelected(const QString&);
    void showTab(const QString&);
    void select( const AppLnk * );
    void externalSelected( const AppLnk *);
    void properties( AppLnk * );
    void nextView();

signals:
    void executing( const AppLnk * );
    void busy();
    void notBusy(const QString&);

private slots:
    void doMaximize();
    void systemMessage( const QCString &, const QByteArray &);
    void launcherMessage( const QCString &, const QByteArray &);
    void storageChanged();
    void cancelSync();

private:
    void updateApps();
    void loadDocs();
    void updateDocs();
    void updateTabs();
    void updateMimeTypes();
    void updateMimeTypes(AppLnkSet*);
    void preloadApps();
    AppLnkSet *rootFolder;
    DocLnkSet *docsFolder;
    CategoryTabWidget *tabs;
    StorageInfo *storage;
    SyncDialog *syncDialog;

    void updateLink(const QString& link);
    bool in_lnk_props;
    bool got_lnk_change;
    QString lnk_change;

    QString m_timeStamp;
    Qtopia::UidGen uidgen;
};

#endif // LAUNCHERVIEW_H

