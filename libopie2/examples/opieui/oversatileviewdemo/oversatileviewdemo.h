/*
                             This file is part of the Opie Project

                             Copyright (C) 2003 Michael 'Mickey' Lauer <mickey@tm.informatik.uni-frankfurt.de>
              =.
            .=l.
           .>+-=
 _;:,     .>    :=|.         This program is free software; you can
.> <`_,   >  .   <=          redistribute it and/or  modify it under
:`=1 )Y*s>-.--   :           the terms of the GNU Library General Public
.="- .-=="i,     .._         License as published by the Free Software
 - .   .-<_>     .<>         Foundation; either version 2 of the License,
     ._= =}       :          or (at your option) any later version.
    .%`+i>       _;_.
    .i_,=:_.      -<s.       This program is distributed in the hope that
     +  .  -:.       =       it will be useful,  but WITHOUT ANY WARRANTY;
    : ..    .:,     . . .    without even the implied warranty of
    =_        +     =;=|`    MERCHANTABILITY or FITNESS FOR A
  _.=:.       :    :=>`:     PARTICULAR PURPOSE. See the GNU
..}^=.=       =       ;      Library General Public License for more
++=   -.     .`     .:       details.
 :     =  ...= . :.=-
 -.   .:....=;==+<;          You should have received a copy of the GNU
  -_. . .   )=.  =           Library General Public License along with
    --        :-=`           this library; see the file COPYING.LIB.
                             If not, write to the Free Software Foundation,
                             Inc., 59 Temple Place - Suite 330,
                             Boston, MA 02111-1307, USA.

*/

#ifndef OVERSATILEVIEWDEMO_H
#define OVERSATILEVIEWDEMO_H

#include <qvbox.h>

class OVersatileView;
class OVersatileViewItem;

class OVersatileViewDemo: public QVBox
{
  Q_OBJECT
  
  public:
    OVersatileViewDemo( QWidget* parent=0, const char* name=0, WFlags f=0 );
    virtual ~OVersatileViewDemo();

  public slots:
    void selectionChanged();
    void selectionChanged( OVersatileViewItem * );
    void currentChanged( OVersatileViewItem * );
    void clicked( OVersatileViewItem * );
    void pressed( OVersatileViewItem * );

    void doubleClicked( OVersatileViewItem *item );
    void returnPressed( OVersatileViewItem *item );
  
    void onItem( OVersatileViewItem *item );
    void onViewport();

    void expanded( OVersatileViewItem *item );
    void collapsed( OVersatileViewItem *item );
    
    void moved();

    void contextMenuRequested( OVersatileViewItem *item, const QPoint&, int col );
    
  private:
    OVersatileView* vv;

};

#endif
