/***************************************************************************
                          ipkg.cpp  -  description
                             -------------------
    begin                : Sat Aug 31 2002
    copyright            : (C) 2002 by Andy Qua
    email                : andy.qua@blueyonder.co.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

#include <stdio.h>
#include <unistd.h>

#ifdef QWS
#include <qpe/qpeapplication.h>
#else
#include <qapplication.h>
#endif
#include <qdir.h>
#include <qtextstream.h>

#include <opie/oprocess.h>

#include "utils.h"
#include "ipkg.h"
#include "global.h"

Ipkg :: Ipkg()
{
    proc = 0;
}

Ipkg :: ~Ipkg()
{
}

// Option is what we are going to do - install, upgrade, download, reinstall
// package is the package name to install - either a fully qualified path and ipk
//   file (if stored locally) or just the name of the package (for a network package)
// packageName is the package name - (for a network package this will be the same as
//    package parameter)
// dest is the destination alias (from ipk.conf)
// destDir is the dir that the destination alias points to (used to link to root)
// flags is the ipkg options flags
// dir is the directory to run ipkg in (defaults to "")
bool Ipkg :: runIpkg( )
{
    bool ret = false;
    QStringList commands;

    QDir::setCurrent( "/tmp" );

    if ( runtimeDir != "" )
    {
        commands << "cd ";
        commands << runtimeDir;
        commands << ";";
    }
    commands << "ipkg" << "-force-defaults";

    // only set the destination for an install operation
    if ( option == "install" )
        commands << "-dest" << destination;


    if ( option != "update" && option != "download" )
    {
        if ( flags & FORCE_DEPENDS )
            commands << "-force-depends";
        if ( flags & FORCE_REINSTALL  )
            commands << "-force-reinstall";
        if ( flags & FORCE_REMOVE )
            commands << "-force-removal-of-essential-packages";
        if ( flags & FORCE_OVERWRITE )
            commands << "-force-overwrite";
        if ( flags & VERBOSE_WGET )
            commands << "-verbose_wget";

        // Handle make links
        // Rules - If make links is switched on, create links to root
        // if destDir is NOT /
        if ( flags & MAKE_LINKS )
        {
            // If destDir == / turn off make links as package is being insalled
            // to root already.
            if ( destDir == "/" )
                flags ^= MAKE_LINKS;
        }
    }
    
#ifdef X86
    commands << "-f";
    commands << IPKG_CONF;
#endif


    if ( option == "reinstall" )
        commands << "install";
    else
        commands << option;
    if ( package != "" )
        commands << package;


    if ( package != "" )
        emit outputText( QString( "Dealing with package " ) + package );

    qApp->processEvents();

    // If we are removing, reinstalling or upgrading packages and make links option is selected
    // create the links
    if ( option == "remove" || option == "reinstall" || option == "upgrade" )
    {
        createLinks = false;
        if ( flags & MAKE_LINKS )
        {
            emit outputText( QString( "Removing symbolic links...\n" ) );
            linkPackage( Utils::getPackageNameFromIpkFilename( package ), destination, destDir );
            emit outputText( QString( " " ) );
        }
    }
    
    // Execute command
    dependantPackages = new QList<QString>;
    dependantPackages->setAutoDelete( true );

    ret = executeIpkgCommand( commands, option );

    if ( aborted )
        return false;

    if ( option == "install" || option == "reinstall" || option == "upgrade" )
    {
        // If we are not removing packages and make links option is selected
        // create the links
        createLinks = true;
        if ( flags & MAKE_LINKS )
        {
            emit outputText( " " );
            emit outputText( QString( "Creating symbolic links for " )+ package );

            linkPackage( Utils::getPackageNameFromIpkFilename( package ), destination, destDir );

            // link dependant packages that were installed with this release
            QString *pkg;
            for ( pkg = dependantPackages->first(); pkg != 0; pkg = dependantPackages->next() )
            {
                if ( *pkg == package )
                    continue;
                emit outputText( " " );
                emit outputText( QString( "Creating symbolic links for " )+ (*pkg) );
                linkPackage( Utils::getPackageNameFromIpkFilename( *pkg ), destination, destDir );
            }
        }
    }

    delete dependantPackages;

    // Finally, if we are removing a package, remove its entry from the <destdir>/usr/lib/ipkg/status file
    // to workaround an ipkg bug which stops reinstall to a different location
    if ( option == "remove" )
        removeStatusEntry();


    emit outputText( "Finished" );
    emit outputText( "" );
    return ret;

}

void Ipkg :: removeStatusEntry()
{
    QString statusFile = destDir;
    if ( statusFile.right( 1 ) != "/" )
        statusFile += "/";
    statusFile += "usr/lib/ipkg/status";
    QString outStatusFile = statusFile + ".tmp";

    emit outputText( "" );
    emit outputText( "Removing status entry..." );
    emit outputText( QString( "status file - " )+ statusFile );
    emit outputText( QString( "package - " )+ package );
    
    ifstream in( statusFile );
    ofstream out( outStatusFile );
    if ( !in.is_open() )
    {
        emit outputText( QString( "Couldn't open status file - " )+ statusFile );
        return;
    }

    if ( !out.is_open() )
    {
        emit outputText( QString( "Couldn't create tempory status file - " )+ outStatusFile );
        return;
    }

    char line[1001];
    char k[21];
    char v[1001];
    QString key;
    QString value;
    vector<QString> lines;
    do
    {
        in.getline( line, 1000 );
        if ( in.eof() )
            continue;

        k[0] = '\0';
        v[0] = '\0';

        sscanf( line, "%[^:]: %[^\n]", k, v );
        key = k;
        value = v;
        key = key.stripWhiteSpace();
        value = value.stripWhiteSpace();
        if ( key == "Package" && value == package )
        {
            // Ignore all lines up to next empty
            do
            {
                in.getline( line, 1000 );
                if ( in.eof() || QString( line ).stripWhiteSpace() == "" )
                    continue;
            } while ( !in.eof() && QString( line ).stripWhiteSpace() != "" );
        }

        lines.push_back( QString( line ) );
//        out << line << endl;
    } while ( !in.eof() );

    // Write lines out
    vector<QString>::iterator it;
    for ( it = lines.begin() ; it != lines.end() ; ++it )
    {
        cout << "Writing " << (const char *)(*it) << endl;
        out << (const char *)(*it) << endl;
    }
    
    in.close();
    out.close();

    // Remove old status file and put tmp stats file in its place
    remove( statusFile );
    rename( outStatusFile, statusFile );
}

int Ipkg :: executeIpkgCommand( QStringList &cmd, const QString option )
{
    // If one is already running - should never be but just to be safe
    if ( proc )
    {
        delete proc;
        proc = 0;
    }
    
    // OK we're gonna use OProcess to run this thing
    proc = new OProcess();
    aborted = false;


    // Connect up our slots
    connect(proc, SIGNAL(processExited(OProcess *)),
            this, SLOT( processFinished()));

    connect(proc, SIGNAL(receivedStdout(OProcess *, char *, int)),
            this, SLOT(commandStdout(OProcess *, char *, int)));

    connect(proc, SIGNAL(receivedStderr(OProcess *, char *, int)),
            this, SLOT(commandStderr(OProcess *, char *, int)));
    
    for ( QStringList::Iterator it = cmd.begin(); it != cmd.end(); ++it )
    {
         qDebug( "%s ", (*it).latin1() );
         *proc << (*it).latin1();
    }
    cout << endl;

    // Start the process going
    finished = false;
    if(!proc->start(OProcess::NotifyOnExit, OProcess::All))
    {
        emit outputText( QString( "Couldn't start ipkg process" ) );
        qDebug( "Couldn't start ipkg process!" );
    }

    // Now wait for it to finish
    while ( !finished )
        qApp->processEvents();
}

void Ipkg::commandStdout(OProcess*, char *buffer, int buflen)
{
    qDebug("received stdout %d bytes", buflen);

    QString lineStr = buffer;
    if ( lineStr[buflen-1] == '\n' )
        buflen --;
    lineStr = lineStr.left( buflen );
    emit outputText( lineStr );

    // check if we are installing dependant packages
    if ( option == "install" || option == "reinstall" )
    {
        // Need to keep track of any dependant packages that get installed
        // so that we can create links to them as necessary
        if ( lineStr.startsWith( "Installing " ) )
        {
            int start = lineStr.find( " " ) + 1;
            int end = lineStr.find( " ", start );
            QString *package = new QString( lineStr.mid( start, end-start ) );
            dependantPackages->append( package );
        }
    }
    
    qDebug(lineStr);
    buffer[0] = '\0';
}

void Ipkg::commandStderr(OProcess*, char *buffer, int buflen)
{
    qDebug("received stderrt %d bytes", buflen);

    QString lineStr = buffer;
    if ( lineStr[buflen-1] == '\n' )
        buflen --;
    lineStr=lineStr.left( buflen );
    emit outputText( lineStr );
    buffer[0] = '\0';
}

void Ipkg::processFinished()
{
    delete proc;
    proc = 0;
    finished = true;
}


void Ipkg :: abort()
{
    if ( proc )
    {
        proc->kill();
        aborted = true;
    }
}

/*
int Ipkg :: executeIpkgCommand( QString &cmd, const QString option )
{
    FILE *fp = NULL;
    char line[130];
    QString lineStr, lineStrOld;
    int ret = false;

    fp = popen( (const char *) cmd, "r");
    if ( fp == NULL )
    {
        cout << "Couldn't execute " << cmd << "! err = " << fp << endl;
        QString text;
        text.sprintf( "Couldn't execute %s! See stdout for error code", (const char *)cmd );
        emit outputText( text );
    }
    else
    {
        while ( fgets( line, sizeof line, fp) != NULL )
        {
            lineStr = line;
            lineStr=lineStr.left( lineStr.length()-1 );

            if ( lineStr != lineStrOld )
            {
                //See if we're finished
                if ( option == "install" || option == "reinstall" )
                {
                    // Need to keep track of any dependant packages that get installed
                    // so that we can create links to them as necessary
                    if ( lineStr.startsWith( "Installing " ) )
                    {
                        int start = lineStr.find( " " ) + 1;
                        int end = lineStr.find( " ", start );
                        QString *package = new QString( lineStr.mid( start, end-start ) );
                        dependantPackages->append( package );
                    }
                }

                if ( option == "update" )
                {
                    if (lineStr.contains("Updated list"))
                        ret = true;
                }
                else if ( option == "download" )
                {
                    if (lineStr.contains("Downloaded"))
                        ret = true;
                }
                else
                {
                    if (lineStr.contains("Done"))
                        ret = true;
                }

                emit outputText( lineStr );
            }
            lineStrOld = lineStr;
            qApp->processEvents();
        }
        pclose(fp);
    }

    return ret;
}
*/

void Ipkg :: linkPackage( const QString &packFileName, const QString &dest, const QString &destDir )
{
    if ( dest == "root" || dest == "/" )
        return;
        
    qApp->processEvents();
    QStringList *fileList = getList( packFileName, destDir );
    qApp->processEvents();
    processFileList( fileList, destDir );
    delete fileList;
}

QStringList* Ipkg :: getList( const QString &packageFilename, const QString &destDir )
{
    QString packageFileDir = destDir+"/usr/lib/ipkg/info/"+packageFilename+".list";
    QFile f( packageFileDir );

    cout << "Try to open " << packageFileDir << endl;
    if ( !f.open(IO_ReadOnly) )
    {
        // Couldn't open from dest, try from /
        cout << "Could not open:" << packageFileDir << endl;
        f.close();
        
        packageFileDir = "/usr/lib/ipkg/info/"+packageFilename+".list";
        f.setName( packageFileDir );
        qDebug( "Try to open %s", packageFileDir.latin1() );
        if ( ! f.open(IO_ReadOnly) )
        {
            qDebug( "Could not open: %s", packageFileDir.latin1() );
            emit outputText( QString( "Could not open :" ) + packageFileDir );
            return (QStringList*)0;
        }
    }
    QStringList *fileList = new QStringList();
    QTextStream t( &f );
    while ( !t.eof() )
        *fileList += t.readLine();

    f.close();
    return fileList;
}

void Ipkg :: processFileList( const QStringList *fileList, const QString &destDir )
{
    if ( !fileList || fileList->isEmpty() )
        return;

    QString baseDir = ROOT;

    if ( createLinks == true )
    {
        for ( uint i=0; i < fileList->count(); i++ )
        {
            processLinkDir( (*fileList)[i], baseDir, destDir );
            qApp->processEvents();
        }
    }
    else
    {
        for ( int i = fileList->count()-1; i >= 0 ; i-- )
        {
            processLinkDir( (*fileList)[i], baseDir, destDir );
            qApp->processEvents();
        }
    }
}

void Ipkg :: processLinkDir( const QString &file, const QString &destDir, const QString &baseDir )
{

    QString sourceFile = baseDir + file;
    
    QString linkFile = destDir;
    if ( file.startsWith( "/" ) && destDir.right( 1 ) == "/" )
    {
        linkFile += file.mid( 1 );
    }   
    else
    {
        linkFile += file;
    }
    QString text;
    if ( createLinks )
    {
        // If this file is a directory (ends with a /) and it doesn't exist,
        // we need to create it
        if ( file.right(1) == "/" )
        {
            QFileInfo f( linkFile );
            if ( !f.exists() )
            {
                emit outputText( QString( "Creating directory " ) + linkFile );
                QDir d;
                d.mkdir( linkFile, true );
            }
//            else
//                emit outputText( QString( "Directory " ) + linkFile + " already exists" );
            
        }
        else
        {
            int rc = symlink( sourceFile, linkFile );
            text = (rc == 0 ? "Linked " : "Failed to link ");
            text += sourceFile + " to " + linkFile;
            emit outputText( text );
        }
    }
    else
    {
        QFileInfo f( linkFile );
        if ( f.exists() )
        {
            if ( f.isFile() )
            {
                QFile f( linkFile );
                bool rc = f.remove();

                text = (rc ? "Removed " : "Failed to remove ");
                text += linkFile;
                emit outputText( text );
            }
            else if ( f.isDir() )
            {
                QDir d;
                bool rc = d.rmdir( linkFile, true );
                if ( rc )
                {
                    text = (rc ? "Removed " : "Failed to remove ");
                    text += linkFile;
                    emit outputText( text );
                }
            }
        }
    }
    
}
