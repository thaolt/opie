/***************************************************************************
   AdvancedFm.cpp
                             -------------------
                             ** Created: Sat Mar 9 23:33:09 2002
    copyright            : (C) 2002 by ljp
    email                : ljp@llornkcor.com
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    ***************************************************************************/
#define DEVELOPERS_VERSION
#define QTOPIA_INTERNAL_FSLP // to get access to fileproperties

#include "advancedfm.h"
#include "inputDialog.h"
#include "filePermissions.h"
#include "output.h"

#include <opie/ofileselector.h>
#include <opie/ofiledialog.h>


#include <qpe/lnkproperties.h>
#include <qpe/filemanager.h>
#include <qpe/qcopenvelope_qws.h>
#include <qpe/qpemenubar.h>
#include <qpe/qpetoolbar.h>
#include <qpe/qpeapplication.h>
#include <qpe/resource.h>
#include <qpe/qcopenvelope_qws.h>
#include <qpe/config.h>
#include <qpe/mimetype.h>
#include <qpe/applnk.h>
#include <qpe/ir.h>

//#include <opie/ofileselector.h>
#include <qmultilineedit.h>

#include <qtextstream.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qfile.h>
#include <qstring.h>
#include <qcombobox.h>
#include <qpopupmenu.h>
#include <qlistview.h>
#include <qmainwindow.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include <qregexp.h>

#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <mntent.h>
#include <string.h>
#include <errno.h>
#include <sys/vfs.h>
#include <mntent.h>

AdvancedFm::AdvancedFm( )
  : QMainWindow( )
{
  setCaption( tr( "AdvancedFm" ) );

  QGridLayout *layout = new QGridLayout( this );
  layout->setSpacing( 2);
  layout->setMargin( 2);

  connect( qApp,SIGNAL( aboutToQuit()),SLOT( cleanUp()) );

  QPEMenuBar *menuBar = new QPEMenuBar(this);
  fileMenu = new QPopupMenu( this );
  viewMenu  = new QPopupMenu( this );

  layout->addMultiCellWidget( menuBar, 0, 0, 0, 1 );

  menuBar->insertItem( tr( "File" ), fileMenu);
  menuBar->insertItem( tr( "View" ), viewMenu);

  qpeDirButton= new QToolButton(this,"QPEButton");
  qpeDirButton->setPixmap( Resource::loadPixmap("launcher/opielogo16x16"));//,"",this,"QPEButton");
  qpeDirButton ->setFixedSize( QSize( 20, 20 ) );
  connect( qpeDirButton ,SIGNAL(released()),this,SLOT( QPEButtonPushed()) );
  layout->addMultiCellWidget( qpeDirButton , 0, 0, 2, 2);

  cfButton = new QToolButton( this,"CFButton");
  cfButton->setPixmap(Resource::loadPixmap("cardmon/pcmcia"));
  cfButton ->setFixedSize( QSize( 20, 20 ) );
  connect( cfButton ,SIGNAL(released()),this,SLOT( CFButtonPushed()) );
  layout->addMultiCellWidget( cfButton , 0, 0, 3, 3);

  sdButton = new QToolButton( this,"SDButton");
  sdButton->setPixmap(Resource::loadPixmap("sdmon/sdcard"));
  sdButton->setFixedSize( QSize( 20, 20 ) );
  connect( sdButton ,SIGNAL(released()),this,SLOT( SDButtonPushed()) );
  layout->addMultiCellWidget( sdButton , 0, 0, 4, 4);

  cdUpButton = new QToolButton( this,"cdUpButton");
  cdUpButton->setPixmap(Resource::loadPixmap("up"));
  cdUpButton ->setFixedSize( QSize( 20, 20 ) );
  connect( cdUpButton ,SIGNAL(released()),this,SLOT( upDir()) );
  layout->addMultiCellWidget( cdUpButton , 0, 0, 5, 5);

  docButton = new QToolButton( this,"docsButton");
  docButton->setPixmap(Resource::loadPixmap("DocsIcon"));
  docButton->setFixedSize( QSize( 20, 20 ) );
  connect( docButton,SIGNAL(released()),this,SLOT( docButtonPushed()) );
  layout->addMultiCellWidget( docButton, 0, 0, 6, 6);

  homeButton = new QToolButton( this,"homeButton");
  homeButton->setPixmap(Resource::loadPixmap("home"));
  homeButton->setFixedSize( QSize( 20, 20 ) );
  connect(homeButton,SIGNAL(released()),this,SLOT(homeButtonPushed()) );
  layout->addMultiCellWidget( homeButton, 0, 0, 7, 7);

  fileMenu->insertItem( tr( "Show Hidden Files" ), this,  SLOT( showHidden() ));
  fileMenu->setItemChecked( fileMenu->idAt(0),TRUE);
  fileMenu->insertSeparator();
  fileMenu->insertItem( tr( "Make Directory" ), this, SLOT( mkDir() ));
  fileMenu->insertItem( tr( "Rename" ), this, SLOT( rn() ));
  fileMenu->insertItem( tr( "Run Command" ), this, SLOT( runCommandStd() ));
  fileMenu->insertItem( tr( "Run Command with Output" ), this, SLOT( runCommand() ));
  fileMenu->insertSeparator();
  fileMenu->insertItem( tr( "Delete" ), this, SLOT( del() ));
  fileMenu->setCheckable(TRUE);

  viewMenu->insertItem( tr( "Switch to Local" ), this, SLOT( switchToLocalTab() ));
  viewMenu->insertItem( tr( "Switch to Remote" ), this, SLOT( switchToRemoteTab() ));
  viewMenu->insertSeparator();
  viewMenu->insertItem( tr( "About" ), this, SLOT( doAbout() ));
  viewMenu->setCheckable(TRUE);


  currentPathCombo = new QComboBox( FALSE, this, "currentPathCombo" );
  currentPathCombo->setEditable(TRUE);
  layout->addMultiCellWidget( currentPathCombo, 1, 1, 0, 7);
  currentPathCombo->lineEdit()->setText( currentDir.canonicalPath());

  connect( currentPathCombo, SIGNAL( activated( const QString & ) ),
           this, SLOT(  currentPathComboActivated( const QString & ) ) );

  connect( currentPathCombo->lineEdit(),SIGNAL(returnPressed()),
           this,SLOT(currentPathComboChanged()));

  currentPathCombo->lineEdit()->setText( currentDir.canonicalPath());

  layout->addMultiCellWidget( currentPathCombo, 1, 1, 0, 7);


  TabWidget = new QTabWidget( this, "TabWidget" );
  layout->addMultiCellWidget( TabWidget, 2, 2, 0, 7);

  tab = new QWidget( TabWidget, "tab" );
  tabLayout = new QGridLayout( tab );
  tabLayout->setSpacing( 2);
  tabLayout->setMargin( 2);

  Local_View = new QListView( tab, "Local_View" );
  Local_View->addColumn( tr("File"),130);
  Local_View->addColumn( tr("Size"),-1);
  Local_View->setColumnAlignment(1,QListView::AlignRight);
  Local_View->addColumn( tr("Date"),-1);
  Local_View->setColumnAlignment(2,QListView::AlignRight);
  Local_View->setAllColumnsShowFocus(TRUE);
  Local_View->setMultiSelection( TRUE );
  Local_View->setSelectionMode(QListView::Extended);

  QPEApplication::setStylusOperation( Local_View->viewport(),QPEApplication::RightOnHold);

  tabLayout->addWidget( Local_View, 0, 0 );

  connect( Local_View, SIGNAL( clicked( QListViewItem*)),
           this,SLOT( localListClicked(QListViewItem *)) );
  connect( Local_View, SIGNAL( mouseButtonPressed( int, QListViewItem *, const QPoint&, int)),
           this,SLOT( localListPressed(int, QListViewItem *, const QPoint&, int)) );

  connect( Local_View, SIGNAL( selectionChanged() ), SLOT( cancelMenuTimer() ) );

  TabWidget->insertTab( tab, tr("1"));

  tab_2 = new QWidget( TabWidget, "tab_2" );
  tabLayout_2 = new QGridLayout( tab_2 );
  tabLayout_2->setSpacing( 2);
  tabLayout_2->setMargin( 2);

  Remote_View = new QListView( tab_2, "Remote_View" );
  Remote_View->addColumn( tr("File"),130);
  Remote_View->addColumn( tr("Size"),-1);
  Remote_View->setColumnAlignment(1,QListView::AlignRight);
  Remote_View->addColumn( tr("Date"),-1);
  Remote_View->setColumnAlignment(2,QListView::AlignRight);
  Remote_View->setAllColumnsShowFocus(TRUE);
  Remote_View->setMultiSelection( TRUE );
  Remote_View->setSelectionMode(QListView::Extended);


  QPEApplication::setStylusOperation( Remote_View->viewport(),QPEApplication::RightOnHold);

  connect( Remote_View, SIGNAL( clicked( QListViewItem*)),
           this,SLOT( remoteListClicked(QListViewItem *)) );
  connect( Remote_View, SIGNAL( mouseButtonPressed( int, QListViewItem *, const QPoint&, int)),
           this,SLOT( remoteListPressed(int, QListViewItem *, const QPoint&, int)) );

  tabLayout_2->addWidget( Remote_View, 0, 0 );


  TabWidget->insertTab( tab_2, tr( "2"));

  connect(TabWidget,SIGNAL(currentChanged(QWidget *)),
          this,SLOT(tabChanged(QWidget*)));

  /*     tab_3 = new QWidget( TabWidget, "tab_3" );
         tabLayout_3 = new QGridLayout( tab_3 );
         tabLayout_3->setSpacing( 2);
         tabLayout_3->setMargin( 2);


         //     OFileDialog fileDialog;
         // fileDialog;
         //    fileSelector = new FileSelector( "*",tab_3, "fileselector" , FALSE, FALSE); //buggy
         //     fileDialog = new OFileDialog("bangalow", tab_3, 4, 2, "Bungalow");
         //      OFileSelector fileDialog = new OFileSelector( tab_3, 4, 2,"/");

         QListView *fileTree;
         fileTree = new QListView( tab_3, "tree" );

      
         tabLayout_3->addMultiCellWidget( fileTree, 0, 0, 0, 3 );

         TabWidget->insertTab( tab_3, tr( "Remote" ) );
  */
         
  ///////////////
     
  currentDir.setFilter( QDir::Files | QDir::Dirs | QDir::Hidden | QDir::All);
  currentDir.setPath( QDir::currentDirPath());

  currentRemoteDir.setFilter( QDir::Files | QDir::Dirs | QDir::Hidden | QDir::All);
  currentRemoteDir.setPath( QDir::currentDirPath());

  b = TRUE;

  filterStr="*";
  b=FALSE;

  connect( &menuTimer, SIGNAL( timeout() ), SLOT( showFileMenu() ) );
    
  populateLocalView();
  populateRemoteView();
  currentPathCombo->setFocus();
}

AdvancedFm::~AdvancedFm()
{
}

void AdvancedFm::cleanUp()
{
  QString sfile=QDir::homeDirPath();
  if(sfile.right(1) != "/")
    sfile+="/._temp";
  else
    sfile+="._temp";
  QFile file( sfile);
  if(file.exists())
    file.remove();
}

void AdvancedFm::tabChanged(QWidget *w)
{
  if (TabWidget->currentPageIndex() == 0) {
    currentPathCombo->lineEdit()->setText( currentDir.canonicalPath());
    viewMenu->setItemChecked(viewMenu->idAt(0),TRUE);
    viewMenu->setItemChecked(viewMenu->idAt(1),FALSE);
  QString fs= getFileSystemType((const QString &) currentDir.canonicalPath());
  setCaption("AdvancedFm :: "+fs+" :: "+checkDiskSpace((const QString &) currentDir.canonicalPath())+" kB free" );
    
  }
  if (TabWidget->currentPageIndex() == 1) {
    currentPathCombo->lineEdit()->setText( currentRemoteDir.canonicalPath());
    viewMenu->setItemChecked(viewMenu->idAt(1),TRUE);
    viewMenu->setItemChecked(viewMenu->idAt(0),FALSE);
  QString fs= getFileSystemType((const QString &) currentRemoteDir.canonicalPath());
  setCaption("AdvancedFm :: "+fs+" :: "+checkDiskSpace((const QString &) currentRemoteDir.canonicalPath())+" kB free" );
  }
}


void AdvancedFm::populateLocalView()
{
  QPixmap pm;
  Local_View->clear();
  currentDir.setSorting(/* QDir::Size*/ /*| QDir::Reversed | */QDir::DirsFirst);
  currentDir.setMatchAllDirs(TRUE);
  currentDir.setNameFilter(filterStr);
  QString fileL, fileS, fileDate;
  QString fs= getFileSystemType((const QString &) currentDir.canonicalPath());
  setCaption("AdvancedFm :: "+fs+" :: "+checkDiskSpace((const QString &) currentDir.canonicalPath())+" kB free" );
  bool isDir=FALSE;
  const QFileInfoList *list = currentDir.entryInfoList( /*QDir::All*/ /*, QDir::SortByMask*/);
  QFileInfoListIterator it(*list);
  QFileInfo *fi;
  while ( (fi=it.current()) ) {
    if (fi->isSymLink() ) {
      QString symLink=fi->readLink();
      QFileInfo sym( symLink);
      fileS.sprintf( "%10li", sym.size() );
      fileL.sprintf( "%s ->  %s",  fi->fileName().data(),sym.absFilePath().data() );
      fileDate = sym.lastModified().toString();
    } else {
      fileS.sprintf( "%10li", fi->size() );
      fileL.sprintf( "%s",fi->fileName().data() );
      fileDate= fi->lastModified().toString();
      if( QDir(QDir::cleanDirPath( currentDir.canonicalPath()+"/"+fileL)).exists() ) {
        fileL+="/";
        isDir=TRUE;
      }
    }
    QFileInfo fileInfo(  currentDir.canonicalPath()+"/"+fileL);
    if(fileL !="./" && fi->exists()) {
      item= new QListViewItem( Local_View, fileL, fileS , fileDate);

      if(isDir || fileL.find("/",0,TRUE) != -1) {

        if( !QDir( fi->filePath() ).isReadable()) //is directory
          pm = Resource::loadPixmap( "lockedfolder" );
        else
          pm= Resource::loadPixmap( "folder" );
      } else if ( fs == "vfat" && fileInfo.filePath().contains("/bin") ) {
        pm = Resource::loadPixmap( "exec");
      } else if( (fileInfo.permission( QFileInfo::ExeUser)
                  | fileInfo.permission( QFileInfo::ExeGroup)
                  | fileInfo.permission( QFileInfo::ExeOther)) && fs != "vfat" ) {
        pm = Resource::loadPixmap( "exec");
      } else if( !fi->isReadable() ) {
        pm = Resource::loadPixmap( "locked" );
      } else { //everything else goes by mimetype
        MimeType mt(fi->filePath());
        pm=mt.pixmap(); //sets the correct pixmap for mimetype
        if(pm.isNull())
          pm =  Resource::loadPixmap( "UnknownDocument-14" );
      }
      item->setPixmap( 0,pm);
      if(  fi->isSymLink() && fileL.find("->",0,TRUE) != -1) {
        // overlay link image
        pm= Resource::loadPixmap( "folder" );
        QPixmap lnk = Resource::loadPixmap( "opie/symlink" );
        QPainter painter( &pm );
        painter.drawPixmap( pm.width()-lnk.width(), pm.height()-lnk.height(), lnk );
        pm.setMask( pm.createHeuristicMask( FALSE ) );
        item->setPixmap( 0, pm);
      }
    }
    isDir=FALSE;
    ++it;
  }

  if(currentDir.canonicalPath().find("dev",0,TRUE) != -1) {
    struct stat buf;
    dev_t devT;
    DIR *dir;
    struct dirent *mydirent;
    if((dir = opendir( currentDir.canonicalPath().latin1())) != NULL)
      while ((mydirent = readdir(dir)) != NULL) {
        lstat( mydirent->d_name, &buf);
        qDebug(mydirent->d_name);
        fileL.sprintf("%s", mydirent->d_name);
        devT = buf.st_dev;
        fileS.sprintf("%d, %d", (int) ( devT >>8) &0xFF, (int)devT &0xFF);
        fileDate.sprintf("%s", ctime( &buf.st_mtime));
        if( fileL.find(".") == -1 ){
          item= new QListViewItem( Local_View, fileL, fileS, fileDate);
          pm =  Resource::loadPixmap( "UnknownDocument-14" );
          item->setPixmap( 0,pm);
        }
      }

    closedir(dir);
  }

  Local_View->setSorting( 3,FALSE);
  fillCombo( (const QString &) currentDir.canonicalPath());
}


void AdvancedFm::populateRemoteView()
{
  QPixmap pm;
  Remote_View->clear();
  currentRemoteDir.setSorting(/* QDir::Size*/ /*| QDir::Reversed | */QDir::DirsFirst);
  currentRemoteDir.setMatchAllDirs(TRUE);
  currentRemoteDir.setNameFilter(filterStr);
  QString fileL, fileS, fileDate;

  QString fs= getFileSystemType((const QString &) currentRemoteDir.canonicalPath());
  setCaption("AdvancedFm :: "+fs+" :: "+checkDiskSpace((const QString &) currentRemoteDir.canonicalPath())+" kB free" );
  bool isDir=FALSE;
  const QFileInfoList *list = currentRemoteDir.entryInfoList( /*QDir::All*/ /*, QDir::SortByMask*/);
  QFileInfoListIterator it(*list);
  QFileInfo *fi;
  while ( (fi=it.current()) ) {
    if (fi->isSymLink() ){
      QString symLink=fi->readLink();
      //         qDebug("Symlink detected "+symLink);
      QFileInfo sym( symLink);
      fileS.sprintf( "%10li", sym.size() );
      fileL.sprintf( "%s ->  %s",  fi->fileName().data(),sym.absFilePath().data() );
      fileDate = sym.lastModified().toString();
    } else {
      //        qDebug("Not a dir: "+currentDir.canonicalPath()+fileL);
      fileS.sprintf( "%10li", fi->size() );
      fileL.sprintf( "%s",fi->fileName().data() );
      fileDate= fi->lastModified().toString();
      if( QDir(QDir::cleanDirPath( currentRemoteDir.canonicalPath()+"/"+fileL)).exists() ) {
        fileL+="/";
        isDir=TRUE;
        //     qDebug( fileL);
      }
    }
    QFileInfo fileInfo(  currentRemoteDir.canonicalPath()+"/"+fileL);
    if(fileL !="./" && fi->exists()) {
      item= new QListViewItem( Remote_View, fileL, fileS, fileDate);
      QPixmap pm;

      if(isDir || fileL.find("/",0,TRUE) != -1) {
        if( !QDir( fi->filePath() ).isReadable())
          pm = Resource::loadPixmap( "lockedfolder" );
        else
          pm= Resource::loadPixmap( "folder" );
      } else if ( fs == "vfat" && fileInfo.filePath().contains("/bin") ) {
        pm = Resource::loadPixmap( "exec");
      } else if( (fileInfo.permission( QFileInfo::ExeUser)
                  | fileInfo.permission( QFileInfo::ExeGroup)
                  | fileInfo.permission( QFileInfo::ExeOther)) && fs != "vfat" ) {
        pm = Resource::loadPixmap( "exec");
      } else if( !fi->isReadable() ) {
        pm = Resource::loadPixmap( "locked" );
      } else {
        MimeType mt(fi->filePath());
        pm=mt.pixmap(); //sets the correct pixmap for mimetype
        if(pm.isNull())
          pm =  Resource::loadPixmap( "UnknownDocument-14" );
      }
      if(  fi->isSymLink() && fileL.find("->",0,TRUE) != -1) {
        // overlay link image
        pm= Resource::loadPixmap( "folder" );
        QPixmap lnk = Resource::loadPixmap( "opie/symlink" );
        QPainter painter( &pm );
        painter.drawPixmap( pm.width()-lnk.width(), pm.height()-lnk.height(), lnk );
        pm.setMask( pm.createHeuristicMask( FALSE ) );
      }
      item->setPixmap( 0, pm);
    }
    isDir=FALSE;
    ++it;
  }

  if(currentRemoteDir.canonicalPath().find("dev",0,TRUE) != -1) {
    struct stat buf;
    DIR *dir;
    struct dirent *mydirent;
    if((dir = opendir( currentRemoteDir.canonicalPath().latin1())) != NULL)
      while ((mydirent = readdir(dir)) != NULL) {
        lstat( mydirent->d_name, &buf);
        qDebug(mydirent->d_name);
        fileL.sprintf("%s", mydirent->d_name);
        fileS.sprintf("%d,%d", (int) (buf.st_dev>>8)&0xFF, (int) buf.st_dev &0xFF);
        fileDate.sprintf("%s", ctime( &buf.st_mtime));
        if( fileL.find(".") == -1 ){
          item= new QListViewItem( Remote_View, fileL, fileS, fileDate);
          pm =  Resource::loadPixmap( "UnknownDocument-14" );
          item->setPixmap( 0,pm);
        }
      }

    closedir(dir);
  }

  Remote_View->setSorting( 3,FALSE);
  fillCombo( (const QString &) currentRemoteDir.canonicalPath() );
}

void AdvancedFm::localListClicked(QListViewItem *selectedItem)
{
  if(selectedItem) {
    QString strItem=selectedItem->text(0);
    QString strSize=selectedItem->text(1);
    strSize=strSize.stripWhiteSpace();
    if(strItem.find("@",0,TRUE) !=-1 || strItem.find("->",0,TRUE) !=-1 ) { //if symlink
      // is symlink
      QString strItem2 = strItem.right( (strItem.length() - strItem.find("->",0,TRUE)) - 4);
      if(QDir(strItem2).exists() ) {
        currentDir.cd(strItem2, TRUE);
        populateLocalView();
      }
    } else { // not a symlink
      if(strItem.find(". .",0,TRUE) && strItem.find("/",0,TRUE)!=-1 ) {
        if(QDir(QDir::cleanDirPath(currentDir.canonicalPath()+"/"+strItem)).exists() ) {
          strItem=QDir::cleanDirPath(currentDir.canonicalPath()+"/"+strItem);
          currentDir.cd(strItem,FALSE);
          populateLocalView();
        } else {
          currentDir.cdUp();
          populateLocalView();
        }
        if(QDir(strItem).exists()){
          currentDir.cd(strItem, TRUE);
          populateLocalView();
        }
      } else {
        strItem=QDir::cleanDirPath(currentDir.canonicalPath()+"/"+strItem);
        if( QFile::exists(strItem ) ) {
          //                     qDebug("clicked item "+strItem);
          //                     DocLnk doc( strItem, FALSE );
          //                     doc.execute();
          //    Local_View->clearSelection();
        }
      } //end not symlink
      chdir(strItem.latin1());
    }
  }
}

void AdvancedFm::remoteListClicked(QListViewItem *selectedItem)
{
  if(selectedItem) {
    QString strItem=selectedItem->text(0);
    QString strSize=selectedItem->text(1);
    strSize=strSize.stripWhiteSpace();
    if(strItem.find("@",0,TRUE) !=-1 || strItem.find("->",0,TRUE) !=-1 ) { //if symlink
      // is symlink
      QString strItem2 = strItem.right( (strItem.length() - strItem.find("->",0,TRUE)) - 4);
      if(QDir(strItem2).exists() ) {
        currentRemoteDir.cd(strItem2, TRUE);
        populateRemoteView();
      }
    } else { // not a symlink
      if(strItem.find(". .",0,TRUE) && strItem.find("/",0,TRUE)!=-1 ) {
        if(QDir(QDir::cleanDirPath( currentRemoteDir.canonicalPath()+"/"+strItem)).exists() ) {
          strItem=QDir::cleanDirPath( currentRemoteDir.canonicalPath()+"/"+strItem);
          currentRemoteDir.cd(strItem,FALSE);
          populateRemoteView();
        } else {
          currentRemoteDir.cdUp();
          populateRemoteView();
        }
        if(QDir(strItem).exists()){
          currentRemoteDir.cd(strItem, TRUE);
          populateRemoteView();
        }
      } else {
        strItem=QDir::cleanDirPath( currentRemoteDir.canonicalPath()+"/"+strItem);
        if( QFile::exists(strItem ) ) {
          //                     qDebug("clicked item "+strItem);
          //                     DocLnk doc( strItem, FALSE );
          //                     doc.execute();
          //    Remote_View->clearSelection();
        }
      } //end not symlink
      chdir(strItem.latin1());
    }
  }
}

void AdvancedFm::doLocalCd()
{
  localListClicked( Local_View->currentItem());
}

void AdvancedFm::doRemoteCd()
{
  localListClicked( Remote_View->currentItem());
}

void AdvancedFm::showHidden()
{
  if (b) {
    currentDir.setFilter( QDir::Files | QDir::Dirs | QDir::Hidden | QDir::All);
    fileMenu->setItemChecked( fileMenu->idAt(0),TRUE);
    b=FALSE;

  }  else {
    currentDir.setFilter( QDir::Files | QDir::Dirs/* | QDir::Hidden*/ | QDir::All);
    fileMenu->setItemChecked( fileMenu->idAt(0),FALSE);
    b=TRUE;
  }
  populateLocalView();

}

void AdvancedFm::showRemoteHidden()
{
  if (b) {
    currentRemoteDir.setFilter( QDir::Files | QDir::Dirs | QDir::Hidden | QDir::All);
    b=TRUE;

  }  else {
    currentRemoteDir.setFilter( QDir::Files | QDir::Dirs/* | QDir::Hidden*/ | QDir::All);
    b=FALSE;
  }
  populateRemoteView();
}

void AdvancedFm::localListPressed( int mouse, QListViewItem *item, const QPoint &point, int i)
{
  qDebug("list pressed");
  switch (mouse) {
  case 1:
    break;
  case 2:
    menuTimer.start( 750, TRUE );
    qDebug("Start menu timer\n");
    break;
  };
}

void AdvancedFm::remoteListPressed( int mouse, QListViewItem *item, const QPoint &point, int i)
{

  switch (mouse) {
  case 1:
    break;
  case 2:
    menuTimer.start( 750, TRUE );
    qDebug("Start menu timer");
    break;
  };
}

void AdvancedFm::runThis() {
  QString fs;
  if (TabWidget->currentPageIndex() == 0) {
    QString curFile = Local_View->currentItem()->text(0);
    if(curFile != "../") {

      fs= getFileSystemType((const QString &) currentDir.canonicalPath());
      QFileInfo fileInfo( currentDir.canonicalPath()+"/"+curFile);
      qDebug( fileInfo.owner());
      if( (fileInfo.permission( QFileInfo::ExeUser)
           | fileInfo.permission( QFileInfo::ExeGroup)
           | fileInfo.permission( QFileInfo::ExeOther)) // & fs.find("vfat",0,TRUE) == -1) {
          | fs == "vfat" && fileInfo.filePath().contains("/bin") ) {
        QCopEnvelope e("QPE/System", "execute(QString)" );
        e << curFile;
      } else {
        curFile =  currentDir.canonicalPath()+"/"+curFile;
        DocLnk nf(curFile);
        QString execStr = nf.exec();
        qDebug( execStr);
        if( execStr.isEmpty() ) {
        } else {
          nf.execute();
        }
      }
    }
  } else {
    QString curFile = Remote_View->currentItem()->text(0);
    if(curFile != "../") {

      fs= getFileSystemType((const QString &) currentRemoteDir.canonicalPath());
      qDebug("Filesystemtype is "+fs);
      QFileInfo fileInfo( currentRemoteDir.canonicalPath()+"/"+curFile);
      if( (fileInfo.permission( QFileInfo::ExeUser)
           | fileInfo.permission( QFileInfo::ExeGroup)
           | fileInfo.permission( QFileInfo::ExeOther)) // & fs.find("vfat",0,TRUE) == -1) {
          | fs == "vfat" && fileInfo.filePath().contains("/bin") ) {
        QCopEnvelope e("QPE/System", "execute(QString)" );
        e << curFile;
      } else {
        curFile =  currentRemoteDir.canonicalPath()+"/"+curFile;
        DocLnk nf(curFile);
        QString execStr = nf.exec();
        qDebug(execStr);
        if( execStr.isEmpty() ) {
        } else {
          nf.execute();
        }
      }
    }
  }
}

void AdvancedFm::runText() {
  if (TabWidget->currentPageIndex() == 0) {
    QString curFile = Local_View->currentItem()->text(0);
    if(curFile != "../") {
      curFile =  currentDir.canonicalPath()+"/"+curFile;
      QCopEnvelope e("QPE/Application/textedit", "setDocument(QString)" );
      e << curFile;
    }
  } else {
    QString curFile = Remote_View->currentItem()->text(0);
    if(curFile != "../") {
      curFile =  currentRemoteDir.canonicalPath()+"/"+curFile;
      DocLnk nf(curFile);
      QCopEnvelope e("QPE/Application/textedit", "setDocument(QString)" );
      e << curFile;
    }
  }
}

void AdvancedFm::localMakDir()
{
  InputDialog *fileDlg;
  fileDlg = new InputDialog(this,tr("Make Directory"),TRUE, 0);
  fileDlg->exec();
  if( fileDlg->result() == 1 ) {
    QString  filename = fileDlg->LineEdit1->text();
    currentDir.mkdir( currentDir.canonicalPath()+"/"+filename);
  }
  populateLocalView();
}

void AdvancedFm::remoteMakDir()
{
  InputDialog *fileDlg;
  fileDlg = new InputDialog(this,tr("Make Directory"),TRUE, 0);
  fileDlg->exec();
  if( fileDlg->result() == 1 ) {
    QString  filename = fileDlg->LineEdit1->text();
    currentRemoteDir.mkdir(  currentRemoteDir.canonicalPath()+"/"+filename);
  }
  populateRemoteView();
}

void AdvancedFm::localDelete()
{
  QStringList curFileList = getPath();
  if(curFileList.count() > 0) {
    QString myFile;
    for ( QStringList::Iterator it = curFileList.begin(); it != curFileList.end(); ++it ) {
      myFile = (*it);
      if( myFile.find(" -> ",0,TRUE) != -1)
        myFile = myFile.left( myFile.find(" -> ",0,TRUE));

      QString f = currentDir.canonicalPath();
      if(f.right(1).find("/",0,TRUE) == -1)
        f+="/";
      f+=myFile;
      if(QDir(f).exists() && !QFileInfo(f).isSymLink() ) {
        switch ( QMessageBox::warning(this,tr("Delete"),tr("Do you really want to delete\n")+f+
                                      "\nand all it's contents ?"
                                      ,tr("Yes"),tr("No"),0,0,1) ) {
        case 0: {
          f=f.left(f.length()-1);
          QString cmd="rm -rf "+f;
          system( cmd.latin1());
          populateLocalView();
        }
          break;
        case 1:
          // exit
          break;
        };

      } else {
        switch ( QMessageBox::warning(this,tr("Delete"),tr("Do you really want to delete\n")+f
                                      +" ?",tr("Yes"),tr("No"),0,0,1) ) {
        case 0: {
          QString cmd="rm "+f;
          QFile file(f);
          file.remove();
          //                   system( cmd.latin1());
          populateLocalView();
        }
          break;
        case 1:
          // exit
          break;
        };
      }
    }
  }
}

void AdvancedFm::remoteDelete()
{
  QStringList curFileList = getPath();
  if( curFileList.count() > 0) {
    QString myFile;

    for ( QStringList::Iterator it = curFileList.begin(); it != curFileList.end(); ++it ) {
      myFile = (*it);
      if(myFile.find(" -> ",0,TRUE) != -1)
        myFile = myFile.left(myFile.find(" -> ",0,TRUE));
      QString f = currentRemoteDir.canonicalPath();
      if(f.right(1).find("/",0,TRUE) == -1)
        f+="/";
      f+=myFile;
      if(QDir(f).exists() && !QFileInfo(f).isSymLink()  ) {
        switch ( QMessageBox::warning(this,tr("Delete"),tr("Do you really want to delete\n")+f+
                                      "\nand all it's contents ?",
                                      tr("Yes"),tr("No"),0,0,1) ) {
        case 0: {
          f=f.left(f.length()-1);
          QString cmd="rm -rf "+f;
          system( cmd.latin1());
          populateRemoteView();
        }
          break;
        case 1:
          // exit
          break;
        };

      } else {
        switch ( QMessageBox::warning(this,tr("Delete"),tr("Do you really want to delete\n")+f
                                      +" ?",tr("Yes"),tr("No"),0,0,1) ) {
        case 0: {
          QString cmd="rm "+f;
          QFile file(f);
          file.remove();
          //                   system( cmd.latin1());
          populateRemoteView();
        }
          break;
        case 1:
          // exit
          break;
        };
      }
    }
  }
}

void AdvancedFm::localRename()
{
  QString curFile = Local_View->currentItem()->text(0);
  qDebug("currentItem "+curFile);
  if( curFile !="../") {
    InputDialog *fileDlg;
    fileDlg = new InputDialog(this,tr("Rename"),TRUE, 0);
    fileDlg->setInputText((const QString &)curFile);
    fileDlg->exec();
    if( fileDlg->result() == 1 ) {
      QString oldname = currentDir.canonicalPath() + "/" + curFile;
      QString newName = currentDir.canonicalPath() + "/" + fileDlg->LineEdit1->text();//+".playlist";
      if( rename(oldname.latin1(), newName.latin1())== -1)
        QMessageBox::message(tr("Note"),tr("Could not rename"));
    }
    populateLocalView();
  }
}

void AdvancedFm::remoteRename()
{
  QString curFile = Remote_View->currentItem()->text(0);
  if( curFile !="../") {
    InputDialog *fileDlg;
    fileDlg = new InputDialog(this,tr("Rename"),TRUE, 0);
    fileDlg->setInputText((const QString &)curFile);
    fileDlg->exec();
    if( fileDlg->result() == 1 ) {
      QString oldname = currentRemoteDir.canonicalPath() + "/" + curFile;
      QString newName = currentRemoteDir.canonicalPath() + "/" + fileDlg->LineEdit1->text();//+".playlist";
      if( rename(oldname.latin1(), newName.latin1())== -1)
        QMessageBox::message(tr("Note"),tr("Could not rename"));
    }
    populateRemoteView();
  }
}

void AdvancedFm::switchToLocalTab()
{
  TabWidget->setCurrentPage(0);
  Local_View->setFocus();
}

void AdvancedFm::switchToRemoteTab()
{
  TabWidget->setCurrentPage(1);
  Remote_View->setFocus();
}

void AdvancedFm::readConfig()
{
  Config cfg("AdvancedFm");
}

void AdvancedFm::writeConfig()
{
  Config cfg("AdvancedFm");
}

void  AdvancedFm::currentPathComboChanged()
{
  if (TabWidget->currentPageIndex() == 0) {
    if(QDir( currentPathCombo->lineEdit()->text()).exists()) {
      currentDir.setPath( currentPathCombo->lineEdit()->text() );
      populateLocalView();
    } else {
      QMessageBox::message(tr("Note"),tr("That directory does not exist"));
    }
  }
  if (TabWidget->currentPageIndex() == 0) {
    if(QDir( currentPathCombo->lineEdit()->text()).exists()) {
      currentRemoteDir.setPath( currentPathCombo->lineEdit()->text() );
      populateRemoteView();
    } else {
      QMessageBox::message(tr("Note"),tr("That directory does not exist"));
    }
  }
}

void  AdvancedFm::fillCombo(const QString &currentPath) {

  if (TabWidget->currentPageIndex() == 0) {
    currentPathCombo->lineEdit()->setText( currentPath);
    if( localDirPathStringList.grep(currentPath,TRUE).isEmpty() ) {
      currentPathCombo->clear();
      localDirPathStringList.prepend( currentPath );
      currentPathCombo->insertStringList( localDirPathStringList,-1);
    }
  } else {
    currentPathCombo->lineEdit()->setText( currentPath);
    if( remoteDirPathStringList.grep( currentPath,TRUE).isEmpty() ) {
      currentPathCombo->clear();
      remoteDirPathStringList.prepend( currentPath );
      currentPathCombo->insertStringList( remoteDirPathStringList,-1);
    }
  }
}

void AdvancedFm::currentPathComboActivated(const QString & currentPath) {
  if (TabWidget->currentPageIndex() == 0) {
    chdir( currentPath.latin1() );
    currentDir.cd( currentPath, TRUE);
    populateLocalView();
    update();
  } else {
    chdir( currentPath.latin1() );
    currentRemoteDir.cd( currentPath, TRUE);
    populateRemoteView();
    update();
  }
}

void AdvancedFm::filePerms() {

  QStringList curFileList = getPath();
  QString filePath;

  if (TabWidget->currentPageIndex() == 0) {
    filePath = currentDir.canonicalPath()+"/";
  } else {
    filePath= currentRemoteDir.canonicalPath()+"/";
  }

  for ( QStringList::Iterator it = curFileList.begin(); it != curFileList.end(); ++it ) {
    filePermissions *filePerm;
    filePerm = new filePermissions(this, "Permissions",true,0,(const QString &)(filePath+*it));
    filePerm->showMaximized();
    filePerm->exec();
    if( filePerm)
      delete  filePerm;
  }
  if (TabWidget->currentPageIndex() == 0) {
    populateLocalView();
  } else {
    populateRemoteView();
  }
}

void AdvancedFm::doProperties() {
  QStringList curFileList = getPath();
  QString filePath;
  if (TabWidget->currentPageIndex() == 0) {
    filePath = currentDir.canonicalPath()+"/";
  } else {
    filePath= currentRemoteDir.canonicalPath()+"/";
  }
  //    qDebug("%d",curFileList.count());
  for ( QStringList::Iterator it = curFileList.begin(); it != curFileList.end(); ++it ) {
    qDebug((filePath+*it));
    DocLnk lnk( (filePath+*it));
    LnkProperties prop( &lnk );
    prop.showMaximized();
    prop.exec();
  }
}

QStringList AdvancedFm::getPath() {
  QStringList strList;
  if (TabWidget->currentPageIndex() == 0) {
    QList<QListViewItem> * getSelectedItems( QListView * Local_View );
    QListViewItemIterator it( Local_View );
    for ( ; it.current(); ++it ) {
      if ( it.current()->isSelected() ) {
        strList <<  it.current()->text(0);
        qDebug(it.current()->text(0));
      }
    }
    return strList;
  } else {
    QList<QListViewItem> * getSelectedItems( QListView * Remote_View );
    QListViewItemIterator it( Remote_View );
    for ( ; it.current(); ++it ) {
      if ( it.current()->isSelected() ) {
        strList << it.current()->text(0);
        qDebug(it.current()->text(0));
      }
    }
    return strList;
  }
  return "";
}

void AdvancedFm::homeButtonPushed() {
  QString current = QDir::homeDirPath();
  chdir( current.latin1() );
  if (TabWidget->currentPageIndex() == 0) {
    currentDir.cd(  current, TRUE);
    populateLocalView();
  } else {
    currentRemoteDir.cd(  current, TRUE);
    populateRemoteView();
  }
  update();
}

void AdvancedFm::docButtonPushed() {
  QString current = QPEApplication::documentDir();
  chdir( current.latin1() );
  if (TabWidget->currentPageIndex() == 0) {
    currentDir.cd( current, TRUE);
    populateLocalView();
  } else {
    currentRemoteDir.cd( current, TRUE);
    populateRemoteView();
  }
  update();
}

void AdvancedFm::SDButtonPushed() {
  QString current = "/mnt/card";
  chdir( current.latin1() );
  if (TabWidget->currentPageIndex() == 0) {
    currentDir.cd( current, TRUE);
    populateLocalView();
  } else {
    currentRemoteDir.cd( current, TRUE);
    populateRemoteView();
  }
  update();

}

void AdvancedFm::CFButtonPushed() {
  QString current = "/mnt/cf";
  chdir( current.latin1() );
  if (TabWidget->currentPageIndex() == 0) {
    currentDir.cd( current, TRUE);
    populateLocalView();
  } else {
    currentRemoteDir.cd( current, TRUE);
    populateRemoteView();
  }
  update();

}


void AdvancedFm::upDir()
{
  if (TabWidget->currentPageIndex() == 0) {
    QString current = currentDir.canonicalPath();
    QDir dir(current);
    dir.cdUp();
    current = dir.canonicalPath();
    chdir( current.latin1() );
    currentDir.cd(  current, TRUE);
    populateLocalView();
    update();
  } else {
    QString current = currentRemoteDir.canonicalPath();
    QDir dir(current);
    dir.cdUp();
    current = dir.canonicalPath();
    chdir( current.latin1() );
    currentRemoteDir.cd(  current, TRUE);
    populateRemoteView();
    update();
  }
}

void AdvancedFm::copy()
{
  qApp->processEvents();
  QStringList curFileList = getPath();
  if( curFileList.count() > 0) {
    QString curFile;
    if (TabWidget->currentPageIndex() == 0) {
      for ( QStringList::Iterator it = curFileList.begin(); it != curFileList.end(); ++it ) {

        QString destFile = currentRemoteDir.canonicalPath()+"/"+(*it);
        qDebug("Destination file is "+destFile);

        curFile = currentDir.canonicalPath()+"/"+(*it);
        qDebug("CurrentFile file is " + curFile);

        QFile f(destFile);
        if( f.exists()) {
          switch ( QMessageBox::warning(this,tr("Delete"),
                                        destFile+tr(" already exists\nDo you really want to delete it?"),
                                        tr("Yes"),tr("No"),0,0,1) ) {
          case 0: 
            f.remove();
            break;
          case 1:
            return;
            break;
          };
        }
        if(!copyFile(destFile, curFile) ) {
          QMessageBox::message("AdvancedFm","Could not copy\n"+curFile +"to\n"+destFile);
          qWarning("nothin doing");
        }
      }
      populateRemoteView();
      TabWidget->setCurrentPage(1);

    } else {
      for ( QStringList::Iterator it = curFileList.begin(); it != curFileList.end(); ++it ) {

        QString destFile = currentDir.canonicalPath()+"/"+(*it);
        qDebug("Destination file is "+destFile);
        curFile = currentRemoteDir.canonicalPath()+"/"+(*it);
        qDebug("CurrentFile file is " + curFile);

        QFile f(destFile);
        if( f.exists()) {
          switch ( QMessageBox::warning(this,tr("Delete"),
                                        destFile+tr(" already exists\nDo you really want to delete it?"),
                                        tr("Yes"),tr("No"),0,0,1) ) {
          case 0: 
            f.remove();
            break;
          case 1:
            return;
            break;
          };
        }
        if(!copyFile(destFile, curFile) ) {
          QMessageBox::message("AdvancedFm",tr("Could not copy\n")+curFile +tr("to\n")+destFile);

          qWarning("nothin doing");
        }
      }
      populateLocalView();
      TabWidget->setCurrentPage(0);
    }

  }
}

void AdvancedFm::copyAs() {
  qApp->processEvents();

  QStringList curFileList = getPath();
  QString curFile;
  InputDialog *fileDlg;
  if (TabWidget->currentPageIndex() == 0) {
    qDebug("tab 1");
    for ( QStringList::Iterator it = curFileList.begin(); it != curFileList.end(); ++it ) {
      QString destFile;
      curFile = currentDir.canonicalPath()+"/"+(*it);
      fileDlg = new InputDialog( this,tr("Copy "+curFile+" As"), TRUE, 0);

      fileDlg->setInputText((const QString &) destFile );
      fileDlg->exec();

      if( fileDlg->result() == 1 ) {
        QString  filename = fileDlg->LineEdit1->text();
        destFile = currentRemoteDir.canonicalPath()+"/"+filename;

        QFile f(destFile);
        if( f.exists()) {
          switch (QMessageBox::warning(this,tr("Delete"),
                                       destFile+tr(" already exists\nDo you really want to delete it?"),
                                       tr("Yes"),tr("No"),0,0,1) ) {
          case 0: 
            f.remove();
            break;
          case 1:
            return;
            break;
          };
        }
        if(!copyFile(destFile, curFile) ) {
          QMessageBox::message("AdvancedFm",tr("Could not copy\n")+curFile +tr("to\n")+destFile);
          qWarning("nothin doing");
        }
      }
      delete fileDlg;

    }
    populateRemoteView();
    TabWidget->setCurrentPage(1);

  } else {
    for ( QStringList::Iterator it = curFileList.begin(); it != curFileList.end(); ++it ) {

      curFile = currentRemoteDir.canonicalPath()+"/"+(*it);
      fileDlg = new InputDialog( this,tr("Copy "+curFile+" As"), TRUE, 0);

      QString destFile;
      fileDlg->setInputText((const QString &) destFile);
      fileDlg->exec();

      if( fileDlg->result() == 1 ) {
        QString  filename = fileDlg->LineEdit1->text();
        destFile = currentDir.canonicalPath()+"/"+filename;

        QFile f( destFile);
        if( f.exists()) {
          switch ( QMessageBox::warning(this,tr("Delete"),
                                        destFile+tr(" already exists\nDo you really want to delete it?"),
                                        tr("Yes"),tr("No"),0,0,1) ) {
          case 0: 
            f.remove();
            break;
          case 1:
            return;
            break;
          };
        }
        if(!copyFile(destFile, curFile) ) {
          QMessageBox::message("AdvancedFm",tr("Could not copy\n")+curFile +tr("to\n")+destFile);
          qWarning("nothin doing");
        }
                        
      }
      delete fileDlg;

    }
    populateLocalView();
    TabWidget->setCurrentPage(0);
  }
}

void AdvancedFm::copySameDir() {
  qApp->processEvents();
  QStringList curFileList = getPath();
  QString curFile;
  InputDialog *fileDlg;

  if (TabWidget->currentPageIndex() == 0) {

    for ( QStringList::Iterator it = curFileList.begin(); it != curFileList.end(); ++it ) {

      QString destFile;
      curFile = currentDir.canonicalPath()+"/"+(*it);

      fileDlg = new InputDialog(this,tr("Copy ")+curFile+tr(" As"),TRUE, 0);
      fileDlg->setInputText((const QString &) destFile );
      fileDlg->exec();

      if( fileDlg->result() == 1 ) {

        QString filename = fileDlg->LineEdit1->text();
        destFile = currentDir.canonicalPath()+"/"+filename;

        QFile f(destFile);
        if( f.exists()) {
          switch (QMessageBox::warning(this,tr("Delete"),
                                       destFile+tr(" already exists\nDo you really want to delete it?"),
                                       tr("Yes"),tr("No"),0,0,1) ) {
          case 0:
            qDebug("");
            f.remove();
            break;
          case 1:
            return;
            break;
          };
        }
        if(!copyFile(destFile, curFile) ) {
          QMessageBox::message("AdvancedFm",tr("Could not copy\n")+curFile +tr("to\n")+destFile);
          qWarning("nothin doing");
        }
                    
        qDebug("copy "+curFile+" as "+destFile);
      }
      delete fileDlg;
    }
    populateLocalView();

  } else {
    for ( QStringList::Iterator it = curFileList.begin(); it != curFileList.end(); ++it ) {

      curFile = currentRemoteDir.canonicalPath()+"/"+(*it);

      fileDlg = new InputDialog(this,tr("Copy ")+curFile+tr(" As"),TRUE, 0);
      QString destFile;
      fileDlg->setInputText((const QString &) destFile);
      fileDlg->exec();
      if( fileDlg->result() == 1 ) {
        QString  filename = fileDlg->LineEdit1->text();

        destFile = currentRemoteDir.canonicalPath()+"/"+filename;

        QFile f(destFile);
        if( f.exists()) {
          switch ( QMessageBox::warning(this,tr("Delete"),
                                        destFile+tr(" already exists\nDo you really want to delete it?"),
                                        tr("Yes"),tr("No"),0,0,1) ) {
          case 0: 
            f.remove();
            break;
          case 1:
            return;
            break;
          };
        }
        if(!copyFile(destFile, curFile) ) {
          QMessageBox::message("AdvancedFm",tr("Could not copy\n")+curFile +tr("to\n")+destFile);
          qWarning("nothin doing");
        }
        qDebug("copy "+curFile+" as "+destFile);
      }
      delete fileDlg;
    }
    populateRemoteView();
  }
}
 
void AdvancedFm::move() {
  qApp->processEvents();

  QStringList curFileList = getPath();
  if( curFileList.count() > 0) {
    QString curFile;
    QString destFile;

    if (TabWidget->currentPageIndex() == 0) {

      for ( QStringList::Iterator it = curFileList.begin(); it != curFileList.end(); ++it ) {

        QString destFile =  currentRemoteDir.canonicalPath();

        if(destFile.right(1).find("/",0,TRUE) == -1)
          destFile+="/";
        destFile +=(*it);
        curFile = currentDir.canonicalPath();

        qDebug("Destination file is "+destFile);

        if(curFile.right(1).find("/",0,TRUE) == -1)
          curFile +="/";

        curFile+=(*it);
        qDebug("CurrentFile file is " + curFile);

        QFile f( curFile);
        if( f.exists()) {
          if(!copyFile( destFile, curFile) ) {
            QMessageBox::message(tr("Note"),tr("Could not move\n")+curFile);
            return;
          } else
            QFile::remove(curFile);
        }
      }

      TabWidget->setCurrentPage(1);

    } else { //view 2

      for ( QStringList::Iterator it = curFileList.begin(); it != curFileList.end(); ++it ) {

        QString destFile = currentDir.canonicalPath();

        if(destFile.right(1).find("/",0,TRUE) == -1)
          destFile+="/";

        destFile +=(*it);

        qDebug("Destination file is "+destFile);

        curFile = currentRemoteDir.canonicalPath();

        if(curFile.right(1).find("/",0,TRUE) == -1)
          curFile +="/";
        curFile+=(*it);
        qDebug("CurrentFile file is " + curFile);

        QFile f( curFile);
        if( f.exists()) {
          if(!copyFile( destFile, curFile) ) {
            QMessageBox::message(tr("Note"),tr("Could not move\n") + curFile);
            return;
          } else
            QFile::remove( curFile);
        }
        TabWidget->setCurrentPage(0);
      }
    }
    populateRemoteView();
    populateLocalView();
  }
}

bool AdvancedFm::copyFile( const QString & dest, const QString & src ) {
  char bf[ 50000 ];
  int  bytesRead;
  bool success = TRUE;
  struct stat status;

  QFile s( src );
  QFile d( dest );

  if( s.open( IO_ReadOnly | IO_Raw ) &&  d.open( IO_WriteOnly | IO_Raw ) )  {
    while( (bytesRead = s.readBlock( bf, sizeof( bf ) )) == sizeof( bf ) ) {
      if( d.writeBlock( bf, sizeof( bf ) ) != sizeof( bf ) ){
        success = FALSE;
        break;
      }
    }
    if( success && (bytesRead > 0) ){
      d.writeBlock( bf, bytesRead );
    }
  } else {
    success = FALSE;
  }

  // Set file permissions
  if( stat( (const char *) src, &status ) == 0 ){
    chmod( (const char *) dest, status.st_mode );
  }

  return success;
}

void AdvancedFm::runCommand() {
  QString curFile;
  if (TabWidget->currentPageIndex() == 0) {
    if( Local_View->currentItem())
      curFile = currentDir.canonicalPath() +"/"+ Local_View->currentItem()->text(0);
  } else {
    if(Remote_View->currentItem())
      curFile = currentRemoteDir.canonicalPath() + "/"+Remote_View->currentItem()->text(0);
  }

  InputDialog *fileDlg;
  fileDlg = new InputDialog(this,tr("Run Command"),TRUE, 0);
  fileDlg->setInputText(curFile);
  fileDlg->exec();
  QString command;
  if( fileDlg->result() == 1 ) {
    command = fileDlg->LineEdit1->text();

    Output *outDlg;
    outDlg = new Output(this, tr("AdvancedFm Output"),FALSE);
    outDlg->showMaximized();
    outDlg->show();
    qApp->processEvents();
    FILE *fp;
    char line[130];
    sleep(1);
    command +=" 2>&1";
    fp = popen(  (const char *) command, "r");
    if ( !fp ) {
      qDebug("Could not execute '" + command + "'! err=%d", fp);
      QMessageBox::warning( this, "AdvancedFm", tr("command failed!"), tr("&OK") );
      pclose(fp);
      return;
    } else {
      while ( fgets( line, sizeof line, fp)) {
        QString lineStr = line;
        lineStr=lineStr.left(lineStr.length()-1);
        outDlg->OutputEdit->append(lineStr);
        outDlg->OutputEdit->setCursorPosition(outDlg->OutputEdit->numLines() + 1,0,FALSE);
      }
    }
    pclose(fp);

  }
}

void AdvancedFm::runCommandStd() {
  QString curFile;
  if (TabWidget->currentPageIndex() == 0) {
    if( Local_View->currentItem())
      curFile = currentDir.canonicalPath() +"/"+  Local_View->currentItem()->text(0);
  } else {
    if(Remote_View->currentItem())
      curFile = currentRemoteDir.canonicalPath() +"/"+  Remote_View->currentItem()->text(0);
  }

  InputDialog *fileDlg;
  fileDlg = new InputDialog(this,tr("Run Command"),TRUE, 0);
  fileDlg->setInputText(curFile);
  fileDlg->exec();
  QString command;
  if( fileDlg->result() == 1 ) {
    qApp->processEvents();
    command = fileDlg->LineEdit1->text() + " &";
    system(command.latin1());
  }
}

void AdvancedFm::fileStatus() {
  QString curFile;
  if (TabWidget->currentPageIndex() == 0) {
    curFile = Local_View->currentItem()->text(0);
  } else {
    curFile = Remote_View->currentItem()->text(0);
  }
  QString command = " stat -l "+ curFile +" 2>&1";
  Output *outDlg;
  outDlg = new Output(this, tr("AdvancedFm Output"),FALSE);
  outDlg->showMaximized();
  outDlg->show();
  qApp->processEvents();
  FILE *fp;
  char line[130];
  sleep(1);
  fp = popen(  (const char *) command, "r");
  if ( !fp ) {
    qDebug("Could not execute '" + command + "'! err=%d", fp);
    QMessageBox::warning( this, "AdvancedFm", tr("command failed!"), tr("&OK") );
    pclose(fp);
    return;
  } else {
    while ( fgets( line, sizeof line, fp)) {
      outDlg->OutputEdit->append(line);
      outDlg->OutputEdit->setCursorPosition(outDlg->OutputEdit->numLines() + 1,0,FALSE);

    }

  }
  pclose(fp);
}

void AdvancedFm::mkDir() {
  if (TabWidget->currentPageIndex() == 0)
    localMakDir();
  else
    remoteMakDir();

}

void AdvancedFm::rn() {
  if (TabWidget->currentPageIndex() == 0)
    localRename();
  else
    remoteRename();

}

void AdvancedFm::del() {
  if (TabWidget->currentPageIndex() == 0)
    localDelete();
  else
    remoteDelete();
}

void AdvancedFm::doAbout() {
  QMessageBox::message("AdvancedFm",tr("Advanced FileManager\n"
                                       "is copyright 2002 by\n"
                                       "L.J.Potter<llornkcor@handhelds.org>\n"
                                       "and is licensed by the GPL"));
}

void AdvancedFm::keyReleaseEvent( QKeyEvent *e)
{
  if( TabWidget->hasFocus()) 
    switch ( e->key() ) {
    case Key_Delete:
      del();
      break;
    case Key_H:
      showHidden();
      break;
    case Key_E:
      runThis();
      break;
    case Key_C:
      copy();
      break;
    case Key_A:
      copyAs();
      break;
    case Key_M:
      move();
      break;
    case Key_R:
      rn();
      break;
    case Key_I:
      fileStatus();
      break;
    case Key_U:
      upDir();
      break;
    case Key_P:
      filePerms();
      break;
    case Key_N:
      mkDir();
      break;
    case Key_1:
      switchToLocalTab();
      break;
    case Key_2:
      switchToRemoteTab();
      break;
    case Key_3:
      CFButtonPushed();
      break;
    case Key_4:
      SDButtonPushed();
      break;
    case Key_5:
      homeButtonPushed();
      break;
    case Key_6:
      docButtonPushed();
      break;
    case Key_7:
      break;
    case Key_8:
      break;
    case Key_9:
      break;
    case Key_0:
      break;
    }
}

void AdvancedFm::mkSym() {
  QString cmd;
  QStringList curFileList = getPath();
  if( curFileList.count() > 0) {

    if (TabWidget->currentPageIndex() == 0) {
      for ( QStringList::Iterator it = curFileList.begin(); it != curFileList.end(); ++it ) {

        QString destName = currentRemoteDir.canonicalPath()+"/"+(*it);
        if(destName.right(1) == "/") destName = destName.left( destName.length() -1);
        QString curFile =  currentDir.canonicalPath()+"/"+(*it);
        if( curFile.right(1) == "/") curFile = curFile.left( curFile.length() -1);
        cmd = "ln -s "+curFile+" "+destName;
        qDebug(cmd);
        system(cmd.latin1() );
      }
      populateRemoteView();
      TabWidget->setCurrentPage(1);
    } else {
      for ( QStringList::Iterator it = curFileList.begin(); it != curFileList.end(); ++it ) {

        QString destName = currentDir.canonicalPath()+"/"+(*it);
        if(destName.right(1) == "/") destName = destName.left( destName.length() -1);
        QString curFile =  currentRemoteDir.canonicalPath()+"/"+(*it);
        if( curFile.right(1) == "/") curFile = curFile.left( curFile.length() -1);

        cmd = "ln -s "+curFile+" "+destName;
        qDebug(cmd);
        system(cmd.latin1() );
      }
      populateLocalView();
      TabWidget->setCurrentPage(0);
    }
  }
}

void AdvancedFm::QPEButtonPushed() {
  QString current = QPEApplication::qpeDir();
  chdir( current.latin1() );
  if (TabWidget->currentPageIndex() == 0) {
    currentDir.cd( current, TRUE);
    populateLocalView();
  } else {
    currentRemoteDir.cd( current, TRUE);
    populateRemoteView();
  }
  update();
}

void AdvancedFm::parsetab(const QString &fileName) {

  fileSystemTypeList.clear();
  fsList.clear();
  struct mntent *me;
  FILE *mntfp = setmntent( fileName.latin1(), "r" );
  if ( mntfp ) {
    while ( (me = getmntent( mntfp )) != 0 ) {
      QString deviceName = me->mnt_fsname;
      QString filesystemType = me->mnt_type;
      QString mountDir = me->mnt_dir;
      if(deviceName != "none") {
        if( fsList.contains(filesystemType) == 0
            & filesystemType.find("proc",0,TRUE) == -1
            & filesystemType.find("cramfs",0,TRUE) == -1
            & filesystemType.find("auto",0,TRUE) == -1)
          fsList << filesystemType;
        fileSystemTypeList << mountDir+"::"+filesystemType;
      }
    }
  }
  endmntent( mntfp );
}

QString  AdvancedFm::getFileSystemType(const QString &currentText) {
  parsetab("/etc/mtab"); //why did TT forget filesystem type?
  QString current = currentText;//.right( currentText.length()-1);
  QString baseFs;
  for ( QStringList::Iterator it = fileSystemTypeList.begin(); it != fileSystemTypeList.end(); ++it ) {
    QString temp = (*it);
    QString path = temp.left(temp.find("::",0,TRUE) );
    path = path.right( path.length()-1);
    if(path.isEmpty()) baseFs = temp.right( temp.length() - temp.find("::",0,TRUE) - 2);
    if( current.find( path,0,TRUE) != -1 && !path.isEmpty()) {
      return temp.right( temp.length() - temp.find("::",0,TRUE) - 2);
    }
  }
  return baseFs;
}

QString  AdvancedFm::getDiskSpace( const QString &) {

}

void AdvancedFm::doBeam() {
  Ir ir;
  if(!ir.supported()){
  } else {

    QStringList curFileList = getPath();
    if( curFileList.count() > 0) {

      if (TabWidget->currentPageIndex() == 0) {
        for ( QStringList::Iterator it = curFileList.begin(); it != curFileList.end(); ++it ) {

          QString curFile =  currentDir.canonicalPath()+"/"+(*it);
          if( curFile.right(1) == "/") curFile = curFile.left( curFile.length() -1);
          Ir *file = new Ir(this, "IR");
          connect(file, SIGNAL(done(Ir*)), this, SLOT( fileBeamFinished( Ir * )));
          file->send( curFile, curFile );
        }

      }  else {
        for ( QStringList::Iterator it = curFileList.begin(); it != curFileList.end(); ++it ) {

          QString curFile =  currentRemoteDir.canonicalPath()+"/"+(*it);
          if( curFile.right(1) == "/") curFile = curFile.left( curFile.length() -1);
          Ir *file = new Ir(this, "IR");
          connect(file, SIGNAL(done(Ir*)), this, SLOT( fileBeamFinished( Ir * )));
          file->send( curFile, curFile );

        }
      }
    }
  }

}

void AdvancedFm::fileBeamFinished( Ir *ir) {
  QMessageBox::message( tr("Advancedfm Beam out"), tr("Ir sent.") ,tr("Ok") );

}

void AdvancedFm::showFileMenu() {

  QString curApp;
  bool isLocalView = false;
  if (TabWidget->currentPageIndex() == 0) {
    isLocalView = TRUE;
    curApp = Local_View->currentItem()->text(0);
  }   else {
    curApp = Remote_View->currentItem()->text(0);
  }

  MimeType mt( curApp );
  const AppLnk* app = mt.application();
  QFile fi(curApp);

  QPopupMenu *m = new QPopupMenu(0);

  m->insertItem(  tr( "Show Hidden Files" ), this,  SLOT( showHidden() ));
  m->insertSeparator();
  if ( QFileInfo(fi).isDir() ) {
    m->insertItem( tr( "Change Directory" ), this, SLOT( doLocalCd() ));
  } else {

    if ( app )
      m->insertItem( app->pixmap(), tr( "Open in " + app->name() ), this, SLOT( runThis() ) );
    else if( QFileInfo(fi).isExecutable() )
      m->insertItem(  tr( "Execute" ), this, SLOT( runThis() ) );

    m->insertItem( Resource::loadPixmap( "txt" ), tr( "Open as text" ),this, SLOT( runText() ) );
  }
  m->insertSeparator();


  if(isLocalView)
    m->insertItem( tr( "Make Directory" ), this, SLOT( localMakDir() ));
  else
    m->insertItem( tr( "Make Directory" ), this, SLOT( remoteMakDir() ));

  m->insertItem( tr( "Make Symlink" ), this, SLOT( mkSym() ));
  m->insertSeparator();

  if(isLocalView)
    m->insertItem( tr( "Rename" ), this, SLOT( localRename() ));
  else
    m->insertItem( tr( "Rename" ), this, SLOT( remoteRename() ));

  m->insertItem( tr( "Copy" ), this, SLOT( copy() ));
  m->insertItem( tr( "Copy As" ), this, SLOT( copyAs() ));
  m->insertItem( tr( "Copy Same Dir" ), this, SLOT( copySameDir() ));
  m->insertItem( tr( "Move" ), this, SLOT( move() ));
  m->insertSeparator();

  if(isLocalView)
    m->insertItem( tr( "Rescan" ), this, SLOT( populateLocalView() ));
  else
    m->insertItem( tr( "Rescan" ), this, SLOT( populateRemoteView() ));

  m->insertItem( tr( "Run Command" ), this, SLOT( runCommand() ));
  m->insertItem( tr( "File Info" ), this, SLOT( fileStatus() ));
  m->insertSeparator();

  if(isLocalView)
    m->insertItem( tr( "Delete" ), this, SLOT( localDelete() ));
  else
    m->insertItem( tr( "Delete" ), this, SLOT( remoteDelete() ));

  m->insertSeparator();
  m->insertItem( tr( "Set Permissions" ), this, SLOT( filePerms() ));

  if( QFile(QPEApplication::qpeDir()+"lib/libopie.so").exists() )  //bad hack for Sharp zaurus failings
    m->insertItem( tr( "Properties" ), this, SLOT( doProperties() ));
  m->setCheckable(TRUE);
  if (!b)
    m->setItemChecked(m->idAt(0),TRUE);
  else
    m->setItemChecked(m->idAt(0),FALSE);
  if(Ir::supported())
    m->insertItem( tr( "Beam File" ), this, SLOT( doBeam() ));
  m->setFocus();
  m->exec( QCursor::pos() );
  sleep(1);
  if(m) delete m;
}


void AdvancedFm::cancelMenuTimer() {

  qDebug("selectionChanged: cancel menu timer");
  if( menuTimer.isActive() )
    menuTimer.stop();
}

QString AdvancedFm::checkDiskSpace(const QString &path) {

  struct statfs fss;
  if ( !statfs( path.latin1(), &fss ) ) {
    int blkSize = fss.f_bsize;
//    int totalBlks = fs.f_blocks;
    int availBlks = fss.f_bavail;

    long mult = blkSize / 1024;
    long div = 1024 / blkSize;
    if ( !mult ) mult = 1;
    if ( !div ) div = 1;


    return QString::number(availBlks * mult / div);
  }
  return "";
}

