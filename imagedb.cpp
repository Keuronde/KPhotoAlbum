/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "imagedb.h"
#include "showbusycursor.h"
#include "options.h"
#include <qfileinfo.h>
#include <qfile.h>
#include <qdir.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kimageio.h>
#include "util.h"
#include "groupCounter.h"
#include <kmdcodec.h>
#include <qprogressdialog.h>
#include <qapplication.h>
#include <qeventloop.h>

ImageDB* ImageDB::_instance = 0;

ImageDB::ImageDB( const QDomElement& top, const QDomElement& blockList, bool* newImages )
{
    QString directory = Options::instance()->imageDirectory();
    if ( directory.isEmpty() )
        return;
    if ( directory.endsWith( QString::fromLatin1("/") ) )
        directory = directory.mid( 0, directory.length()-1 );


    // Load the information from the XML file.
    QDict<void> loadedFiles( 6301 /* a large prime */ );

    // Collect all the ImageInfo's which do not have md5 sum, so we can calculate them in one go,
    // and show the user a progress bar while doing so.
    // This is really only needed for upgrading from KimDaBa version 1.0 so at a later point this
    // code might simply be deleted.
    QValueList<ImageInfo*> missingSums;

    for ( QDomNode node = top.firstChild(); !node.isNull(); node = node.nextSibling() )  {
        QDomElement elm;
        if ( node.isElement() )
            elm = node.toElement();
        else
            continue;

        QString fileName = elm.attribute( QString::fromLatin1("file") );
        if ( fileName.isNull() )
            qWarning( "Element did not contain a file attribute" );
        else if ( loadedFiles.find( fileName ) != 0 )
            qWarning( "XML file contained image %s, more than ones - only first one will be loaded", fileName.latin1());
        else {
            loadedFiles.insert( directory + QString::fromLatin1("/") + fileName,
                                (void*)0x1 /* void pointer to nothing I never need the value,
                                              just its existsance, must be != 0x0 though.*/ );
            ImageInfo* info = load( fileName, elm );
            if ( info->MD5Sum().isNull() && info->imageOnDisk() )
                missingSums.append( info );
            else
                _md5Map.insert( info->MD5Sum(), fileName );
        }
    }

    // calculate md5sums - this should only happen the first time the user starts kimdaba after md5sum has been introducted.
    if ( missingSums.count() != 0 )  {
        calculateMissingMD5sums( missingSums );
        *newImages = true;
    }

    // Read the block list
    for ( QDomNode node = blockList.firstChild(); !node.isNull(); node = node.nextSibling() )  {
        QDomElement elm;
        if ( node.isElement() )
            elm = node.toElement();
        else
            continue;

        QString fileName = elm.attribute( QString::fromLatin1( "file" ) );
        if ( !fileName.isEmpty() )
            _blockList << fileName;
    }

    uint count = _images.count();
    loadExtraFiles( loadedFiles, directory );
    *newImages |= ( count != _images.count() );

    connect( Options::instance(), SIGNAL( deletedOption( const QString&, const QString& ) ),
             this, SLOT( deleteOption( const QString&, const QString& ) ) );
    connect( Options::instance(), SIGNAL( renamedOption( const QString&, const QString&, const QString& ) ),
             this, SLOT( renameOption( const QString&, const QString&, const QString& ) ) );
    connect( Options::instance(), SIGNAL( locked( bool, bool ) ), this, SLOT( lockDB( bool, bool ) ) );
}

int ImageDB::totalCount() const
{
    return _images.count();
}

void ImageDB::search( const ImageSearchInfo& info, int from, int to )
{
    ShowBusyCursor dummy;
    int c = count( info, true, from, to );
    emit searchCompleted();
    emit matchCountChange( from, to, c );
}

int ImageDB::count( const ImageSearchInfo& info )
{
    return count( info, false, -1, -1 );
}

int ImageDB::count( const ImageSearchInfo& info, bool makeVisible, int from, int to )
{
    int count = 0;
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        bool match = !(*it)->isLocked() && const_cast<ImageSearchInfo&>(info).match( *it ); // PENDING(blackie) remove cast

        if ( match )
            ++count;
        match &= ( from != -1 && to != -1 && from <= count && count <= to ) ||
                 ( from == -1 && to == -1 );
        if ( makeVisible )
            (*it)->setVisible( match );
    }
    return count;
}

ImageDB* ImageDB::instance()
{
    if ( _instance == 0 )
        qFatal("ImageDB::instance must not be called before ImageDB::setup");
    return _instance;
}

bool ImageDB::setup( const QDomElement& top, const QDomElement& blockList )
{
    bool newImages;
    _instance = new ImageDB( top, blockList, &newImages );
    return newImages;
}

ImageInfo* ImageDB::load( const QString& fileName, QDomElement elm )
{
    ImageInfo* info = new ImageInfo( fileName, elm );
    info->setVisible( false );
    _images.append(info);
    return info;
}

void ImageDB::loadExtraFiles( const QDict<void>& loadedFiles, QString directory )
{
    if ( directory.endsWith( QString::fromLatin1("/") ) )
        directory = directory.mid( 0, directory.length()-1 );

    QString imageDir = Options::instance()->imageDirectory();
    if ( imageDir.endsWith( QString::fromLatin1("/") ) )
        imageDir = imageDir.mid( 0, imageDir.length()-1 );

    QDir dir( directory );
    QStringList dirList = dir.entryList( QDir::All );
    for( QStringList::Iterator it = dirList.begin(); it != dirList.end(); ++it ) {
        QString file = directory + QString::fromLatin1("/") + *it;
        QFileInfo fi( file );
        if ( (*it) == QString::fromLatin1(".") || (*it) == QString::fromLatin1("..") ||
             (*it) == QString::fromLatin1("ThumbNails") ||
             (*it) == QString::fromLatin1("CategoryImages") ||
             !fi.isReadable() )
                continue;

        if ( fi.isFile() && (loadedFiles.find( file ) == 0) &&
             KImageIO::canRead(KImageIO::type(fi.extension())) ) {
            QString baseName = file.mid( imageDir.length()+1 );

            if ( ! _blockList.contains( baseName ) ) {
                loadExtraFile( baseName );
            }
        }
        else if ( fi.isDir() )  {
            loadExtraFiles( loadedFiles, file );
        }
    }
}

void ImageDB::loadExtraFile( const QString& relativeName )
{
    QString sum = MD5Sum( Options::instance()->imageDirectory() + QString::fromLatin1("/") + relativeName );
    if ( _md5Map.contains( sum ) ) {
        QString fileName = _md5Map[sum];
        for( ImageInfoListIterator it( _images ); *it; ++it ) {
            if ( (*it)->fileName(true) == fileName ) {
                // Update the label in case it contained the previos file name
                QFileInfo fi( (*it)->fileName() );
                if ( (*it)->label() == fi.baseName() ) {
                    QFileInfo fi2( relativeName );
                    (*it)->setLabel( fi2.baseName() );
                }

                (*it)->setFileName( relativeName );
                return;
            }
        }
    }

    ImageInfo* info = new ImageInfo( relativeName  );
    _images.append(info);
}

void ImageDB::save( QDomElement top )
{
    ImageInfoList list = _images;

    // Copy files from clipboard to end of overview, so we don't loose them
    for( ImageInfoListIterator it(_clipboard); *it; ++it ) {
        list.append( *it );
    }

    QDomDocument doc = top.ownerDocument();
    QDomElement images = doc.createElement( QString::fromLatin1( "images" ) );
    top.appendChild( images );

    for( ImageInfoListIterator it( list ); *it; ++it ) {
        images.appendChild( (*it)->save( doc ) );
    }

    QDomElement blockList = doc.createElement( QString::fromLatin1( "blocklist" ) );
    bool any=false;
    for( QStringList::Iterator it = _blockList.begin(); it != _blockList.end(); ++it ) {
        any=true;
        QDomElement elm = doc.createElement( QString::fromLatin1( "block" ) );
        elm.setAttribute( QString::fromLatin1( "file" ), *it );
        blockList.appendChild( elm );
    }

    if (any)
        top.appendChild( blockList );
}

bool ImageDB::isClipboardEmpty()
{
    return _clipboard.count() == 0;
}

QMap<QString,int> ImageDB::classify( const ImageSearchInfo& info, const QString &group )
{
    QMap<QString, int> map;
    GroupCounter counter( group );

    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        bool match = !(*it)->isLocked() && const_cast<ImageSearchInfo&>(info).match( *it ); // PENDING(blackie) remove cast
        if ( match ) {
            QStringList list = (*it)->optionValue(group);
            counter.count( list );
            for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
                map[*it]++;
            }
            if ( list.count() == 0 )
                map[i18n( "**NONE**" )]++;
        }
    }

    QMap<QString,int> groups = counter.result();
    for( QMapIterator<QString,int> it= groups.begin(); it != groups.end(); ++it ) {
        map[it.key()] = it.data();
    }

    return map;
}






int ImageDB::countItemsOfOptionGroup( const QString& group )
{
    int count = 0;
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        if ( (*it)->optionValue( group ).count() != 0 )
            ++count;
    }
    return count;
}

void ImageDB::renameOptionGroup( const QString& oldName, const QString newName )
{
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        (*it)->renameOptionGroup( oldName, newName );
    }
}

void ImageDB::blockList( const ImageInfoList& list )
{
    for( ImageInfoListIterator it( list ); *it; ++it) {
        _blockList << (*it)->fileName( true );
        _images.removeRef( *it );
    }
}

void ImageDB::deleteList( const ImageInfoList& list )
{
    for( ImageInfoListIterator it( list ); *it; ++it ) {
        _images.removeRef( *it );
    }
}

void ImageDB::renameOption( const QString& optionGroup, const QString& oldName, const QString& newName )
{
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        (*it)->renameOption( optionGroup, oldName, newName );
    }
}

void ImageDB::deleteOption( const QString& optionGroup, const QString& option )
{
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        (*it)->removeOption( optionGroup, option );
    }
}

void ImageDB::lockDB( bool lock, bool exclude  )
{
    ImageSearchInfo info = Options::instance()->currentScope();
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        if ( lock ) {
            bool match = info.match( *it );
            if ( !exclude )
                match = !match;
            (*it)->setLocked( match );
        }
        else
            (*it)->setLocked( false );
    }
}

void ImageDB::calculateMissingMD5sums( QValueList<ImageInfo*>& list )
{
    QProgressDialog dialog( i18n("<qt><p><b>Calculating md5sum of you images</b></p>"
                                 "<p>This should really only happen the first time you start KimDaBa "
                          "after upgrading to version 1.1</p></qt>"), i18n("Cancel"), list.count() );

    int count = 0;

    for( QValueList<ImageInfo*>::Iterator it = list.begin(); it != list.end(); ++it, ++count ) {
        if ( count % 10 == 0 ) {
            dialog.setProgress( count ); // ensure to call setProgress(0)
            qApp->eventLoop()->processEvents( QEventLoop::AllEvents );
            if ( dialog.wasCanceled() )
                return;
        }
        QString md5 = MD5Sum( (*it)->fileName() );
        (*it)->setMD5Sum( md5 );
        _md5Map.insert( md5, (*it)->fileName() );
    }
}

QString ImageDB::MD5Sum( const QString& fileName )
{
    QFile file( fileName );
    if ( !file.open( IO_ReadOnly ) ) {
        if ( KMessageBox::warningContinueCancel( 0, i18n("Couldn't open %1").arg( fileName ) ) == KMessageBox::No )
            return QString::null;
    }

    KMD5 md5calculator( 0 /* char* */);
    md5calculator.reset();
    md5calculator.update( file );
    QString md5 = md5calculator.hexDigest();
    return md5;
}

#include "imagedb.moc"
