/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "Util.h"
#include "Settings/SettingsData.h"
#include "DB/ImageInfo.h"
#include "ImageManager/ImageDecoder.h"
#include "ImageManager/AsyncLoader.h"
#include <klocale.h>
#include <qfileinfo.h>

#include <QtCore/QVector>
#include <QList>
#include <kmessagebox.h>
#include <kapplication.h>
#include <qdir.h>
#include <kstandarddirs.h>
#include <stdlib.h>
#include <qregexp.h>
#include <kimageio.h>
#include <kcmdlineargs.h>
#include <kio/netaccess.h>
#include "MainWindow/Window.h"
#include "ImageManager/RawImageDecoder.h"

#ifdef Q_WS_X11
#include "X11/X.h"
#endif

#include <QTextCodec>
#include "Utilities/JpeglibWithFix.h"

extern "C" {
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <sys/types.h>
}
#include "DB/CategoryCollection.h"
#include "DB/ImageDB.h"

#include <config-kpa-exiv2.h>
#ifdef HAVE_EXIV2
#  include "Exif/Info.h"
#endif

#include <kdebug.h>
#include <KMimeType>
#include <QImageReader>
#include <kcodecs.h>
#include "config-kpa-nepomuk.h"

/**
 * Add a line label + info text to the result text if info is not empty.
 * If the result already contains something, a HTML newline is added first.
 * To be used in createInfoText().
 */
static void AddNonEmptyInfo(const QString &label, const QString &info,
                            QString *result) {
    if (info.isEmpty())
        return;
    if (!result->isEmpty())
        *result += QString::fromLatin1("<br/>");
    result->append(label).append(info);
}

/**
 * Given an ImageInfoPtr this function will create an HTML blob about the
 * image. The blob is used in the viewer and in the tool tip box from the
 * thumbnail view.
 *
 * As the HTML text is created, the parameter linkMap is filled with
 * information about hyberlinks. The map maps from an index to a pair of
 * (categoryName, categoryItem). This linkMap is used when the user selects
 * one of the hyberlinks.
 */
QString Utilities::createInfoText( DB::ImageInfoPtr info, QMap< int,QPair<QString,QString> >* linkMap )
{
    Q_ASSERT( info );

    QString result;
    if ( Settings::SettingsData::instance()->showFilename() ) {
        AddNonEmptyInfo(i18n("<b>File Name: </b> "), info->fileName().relative(), &result);
    }

    if ( Settings::SettingsData::instance()->showDate() )  {
        AddNonEmptyInfo(i18n("<b>Date: </b> "), info->date().toString( Settings::SettingsData::instance()->showTime() ? true : false ),
                        &result);
    }

    /* XXX */
    if ( Settings::SettingsData::instance()->showImageSize() && info->mediaType() == DB::Image)  {
        const QSize imageSize = info->size();
        // Do not add -1 x -1 text
        if (imageSize.width() >= 0 && imageSize.height() >= 0) {
            const double megapix = imageSize.width() * imageSize.height() / 1000000.0;
            QString info =
                QString::number(imageSize.width()) + i18n("x") +
                QString::number(imageSize.height());
            if (megapix > 0.05) {
                info +=
                    QString::fromLatin1(" (") + QString::number(megapix, 'f', 1) +
                    i18nc("Short for Mega Pixels", "MP") + QString::fromLatin1(")");
            }
            AddNonEmptyInfo(i18n("<b>Image Size: </b> "), info, &result);
        }
    }

#ifdef HAVE_NEPOMUK
    if ( Settings::SettingsData::instance()->showRating() ) {
        if ( info->rating() != -1 ) {
            if ( ! result.isEmpty() )
                result += QString::fromLatin1("<br/>");
            result += QString::fromLatin1("<img src=\"KRatingWidget://%1\"/>"
                    ).arg( qMin( qMax( static_cast<short int>(0), info->rating() ), static_cast<short int>(10) ) );
        }
    }
#endif

     QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    int link = 0;
     for( QList<DB::CategoryPtr>::Iterator categoryIt = categories.begin(); categoryIt != categories.end(); ++categoryIt ) {
        const QString categoryName = (*categoryIt)->name();
        if ( (*categoryIt)->doShow() ) {
            const StringSet items = info->itemsOfCategory( categoryName );
            if (!items.empty()) {
                QString title = QString::fromLatin1( "<b>%1: </b> " ).arg( (*categoryIt)->text() );
                QString info;
                bool first = true;
                for( StringSet::const_iterator it2 = items.constBegin(); it2 != items.constEnd(); ++it2 ) {
                    QString item = *it2;
                    if ( first )
                        first = false;
                    else
                        info += QString::fromLatin1( ", " );

                    if ( linkMap ) {
                        ++link;
                        (*linkMap)[link] = QPair<QString,QString>( categoryName, item );
                        info += QString::fromLatin1( "<a href=\"%1\">%2</a>").arg( link ).arg( item );
                    }
                    else
                        info += item;
                }
                AddNonEmptyInfo(title, info, &result);
            }
        }
    }

    if ( Settings::SettingsData::instance()->showLabel()) {
        AddNonEmptyInfo(i18n("<b>Label: </b> "), info->label(), &result);
    }

    if ( Settings::SettingsData::instance()->showDescription())  {
        AddNonEmptyInfo(i18n("<b>Description: </b> "), info->description(),
                        &result);
    }

#ifdef HAVE_EXIV2
    QString exifText;
    if ( Settings::SettingsData::instance()->showEXIF() ) {
        typedef QMap<QString,QStringList> ExifMap;
        typedef ExifMap::const_iterator ExifMapIterator;
        ExifMap exifMap = Exif::Info::instance()->infoForViewer( info->fileName(), Settings::SettingsData::instance()->iptcCharset() );

        for( ExifMapIterator exifIt = exifMap.constBegin(); exifIt != exifMap.constEnd(); ++exifIt ) {
            if ( exifIt.key().startsWith( QString::fromAscii( "Exif." ) ) )
                for ( QStringList::const_iterator valuesIt = exifIt.value().constBegin(); valuesIt != exifIt.value().constEnd(); ++valuesIt ) {
                    QString exifName = exifIt.key().split( QChar::fromLatin1('.') ).last();
                    AddNonEmptyInfo(QString::fromLatin1( "<b>%1: </b> ").arg(exifName),
                                    *valuesIt, &exifText);
                }
        }

        QString iptcText;
        for( ExifMapIterator exifIt = exifMap.constBegin(); exifIt != exifMap.constEnd(); ++exifIt ) {
            if ( !exifIt.key().startsWith( QString::fromLatin1( "Exif." ) ) )
                for ( QStringList::const_iterator valuesIt = exifIt.value().constBegin(); valuesIt != exifIt.value().constEnd(); ++valuesIt ) {
                    QString iptcName = exifIt.key().split( QChar::fromLatin1('.') ).last();
                    AddNonEmptyInfo(QString::fromLatin1( "<b>%1: </b> ").arg(iptcName),
                                    *valuesIt, &iptcText);
                }
        }

        if ( !iptcText.isEmpty() ) {
            if ( exifText.isEmpty() )
                exifText = iptcText;
            else
                exifText += QString::fromLatin1( "<hr>" ) + iptcText;
        }
    }

    if ( !result.isEmpty() && !exifText.isEmpty() )
        result += QString::fromLatin1( "<hr>" );
    result += exifText;
#endif

    return result;
}

void Utilities::checkForBackupFile( const QString& fileName, const QString& message )
{
    QString backupName = QFileInfo( fileName ).absolutePath() + QString::fromLatin1("/.#") + QFileInfo( fileName ).fileName();
    QFileInfo backUpFile( backupName);
    QFileInfo indexFile( fileName );

    if ( !backUpFile.exists() || indexFile.lastModified() > backUpFile.lastModified() || backUpFile.size() == 0 )
        if ( !( backUpFile.exists() && !message.isNull() ) )
            return;

    int code;
    if ( message.isNull() )
        code = KMessageBox::questionYesNo( 0, i18n("Autosave file '%1' exists (size %3 KB) and is newer than '%2'. "
                "Should the autosave file be used?", backupName, fileName, backUpFile.size() >> 10 ),
                i18n("Found Autosave File") );
    else if ( backUpFile.size() > 0 )
        code = KMessageBox::warningYesNo( 0,i18n( "<p>Error: Cannot use current database file '%1':</p><p>%2</p>"
                "<p>Do you want to use autosave (%3 - size %4 KB) instead of exiting?</p>"
                "<p><small>(Manually verifying and copying the file might be a good idea)</small></p>", fileName, message, backupName, backUpFile.size() >> 10 ),
                i18n("Recover from Autosave?") );
    else {
        KMessageBox::error( 0, i18n( "<p>Error: %1</p><p>Also autosave file is empty, check manually "
                        "if numbered backup files exist and can be used to restore index.xml.</p>", message ) );
        exit(-1);
    }
 
    if ( code == KMessageBox::Yes ) {
        QFile in( backupName );
        if ( in.open( QIODevice::ReadOnly ) ) {
            QFile out( fileName );
            if (out.open( QIODevice::WriteOnly ) ) {
                char data[1024];
                int len;
                while ( (len = in.read( data, 1024 ) ) )
                    out.write( data, len );
            }
        }
    } else if ( !message.isNull() )
        exit(-1);
}

bool Utilities::ctrlKeyDown()
{
    return QApplication::keyboardModifiers() & Qt::ControlModifier;
}

void Utilities::copyList( const QStringList& from, const QString& directoryTo )
{
    for( QStringList::ConstIterator it = from.constBegin(); it != from.constEnd(); ++it ) {
        QString destFile = directoryTo + QString::fromLatin1( "/" ) + QFileInfo(*it).fileName();
        if ( ! QFileInfo( destFile ).exists() ) {
            const bool ok = copy( *it, destFile );
            if ( !ok ) {
                KMessageBox::error( 0, i18n("Unable to copy '%1' to '%2'.", *it , destFile ), i18n("Error Running Demo") );
                exit(-1);
            }
        }
    }
}

QString Utilities::setupDemo()
{
    QString dir = QString::fromLatin1( "%1/kphotoalbum-demo-%2" ).arg(QDir::tempPath()).arg(QString::fromLocal8Bit( qgetenv( "LOGNAME" ) ));
    QFileInfo fi(dir);
    if ( ! fi.exists() ) {
        bool ok = QDir().mkdir( dir );
        if ( !ok ) {
            KMessageBox::error( 0, i18n("Unable to create directory '%1' needed for demo.", dir ), i18n("Error Running Demo") );
            exit(-1);
        }
    }

    // index.xml
    QString str = readFile(locateDataFile(QString::fromLatin1("demo/index.xml")));
    if ( str.isNull() )
        exit(-1);

    str = str.replace( QRegExp( QString::fromLatin1("imageDirectory=\"[^\"]*\"")), QString::fromLatin1("imageDirectory=\"%1\"").arg(dir) );
    str = str.replace( QRegExp( QString::fromLatin1("htmlBaseDir=\"[^\"]*\"")), QString::fromLatin1("") );
    str = str.replace( QRegExp( QString::fromLatin1("htmlBaseURL=\"[^\"]*\"")), QString::fromLatin1("") );

    QString configFile = dir + QString::fromLatin1( "/index.xml" );
    if ( ! QFileInfo( configFile ).exists() ) {
        QFile out( configFile );
        if ( !out.open( QIODevice::WriteOnly ) ) {
            KMessageBox::error( 0, i18n("Unable to open '%1' for writing.", configFile ), i18n("Error Running Demo") );
            exit(-1);
        }
        QTextStream( &out ) << str;
        out.close();
    }

    // Images
    copyList( KStandardDirs().findAllResources( "data", QString::fromLatin1("kphotoalbum/demo/*.jpg" ) ), dir );
    copyList( KStandardDirs().findAllResources( "data", QString::fromLatin1("kphotoalbum/demo/*.avi" ) ), dir );

    // CategoryImages
    dir = dir + QString::fromLatin1("/CategoryImages");
    fi = QFileInfo(dir);
    if ( ! fi.exists() ) {
        bool ok = QDir().mkdir( dir  );
        if ( !ok ) {
            KMessageBox::error( 0, i18n("Unable to create directory '%1' needed for demo.", dir ), i18n("Error Running Demo") );
            exit(-1);
        }
    }

    copyList( KStandardDirs().findAllResources( "data", QString::fromLatin1("kphotoalbum/demo/CategoryImages/*.jpg" ) ), dir );

    return configFile;
}

bool Utilities::copy( const QString& from, const QString& to )
{
    QFile in( from );
    QFile out( to );

    if ( !in.open(QIODevice::ReadOnly) ) {
        kWarning() << "Couldn't open " << from << " for reading\n";
        return false;
    }
    if ( !out.open(QIODevice::WriteOnly) ) {
        kWarning() << "Couldn't open " << to << " for writing\n";
        in.close();
        return false;
    }

    char buf[4096];
    while( !in.atEnd() ) {
        unsigned long int len = in.read( buf, sizeof(buf));
        out.write( buf, len );
    }

    in.close();
    out.close();
    return true;
}

bool Utilities::makeHardLink( const QString& from, const QString& to )
{
    if (link(from.toLocal8Bit(), to.toLocal8Bit()) != 0)
        return false;
    else
        return true;
}

bool Utilities::makeSymbolicLink( const QString& from, const QString& to )
{
    if (symlink(from.toLocal8Bit(), to.toLocal8Bit()) != 0)
        return false;
    else
        return true;
}

bool Utilities::canReadImage( const DB::FileName& fileName )
{
	bool fastMode = !Settings::SettingsData::instance()->ignoreFileExtension();
    return ! KImageIO::typeForMime( KMimeType::findByPath( fileName.absolute(), 0, fastMode )->name() ).isEmpty() ||
        ImageManager::ImageDecoder::mightDecode( fileName );
    // KMimeType::findByPath() never returns null pointer
}


QString Utilities::locateDataFile(const QString& fileName)
{
    return
        KStandardDirs::
        locate("data", QString::fromLatin1("kphotoalbum/") + fileName);
}

QString Utilities::readFile( const QString& fileName )
{
    if ( fileName.isEmpty() ) {
        KMessageBox::error( 0, i18n("<p>Unable to find file %1</p>", fileName ) );
        return QString();
    }

    QFile file( fileName );
    if ( !file.open( QIODevice::ReadOnly ) ) {
        //KMessageBox::error( 0, i18n("Could not open file %1").arg( fileName ) );
        return QString();
    }

    QTextStream stream( &file );
    QString content = stream.readAll();
    file.close();

    return content;
}

struct myjpeg_error_mgr : public jpeg_error_mgr
{
    jmp_buf setjmp_buffer;
};

extern "C"
{
    static void myjpeg_error_exit(j_common_ptr cinfo)
    {
        myjpeg_error_mgr* myerr =
            (myjpeg_error_mgr*) cinfo->err;

        char buffer[JMSG_LENGTH_MAX];
        (*cinfo->err->format_message)(cinfo, buffer);
        //kWarning() << buffer;
        longjmp(myerr->setjmp_buffer, 1);
    }
}

namespace Utilities
{
    bool loadJPEG(QImage *img, FILE* inputFile, QSize* fullSize, int dim );
}

bool Utilities::loadJPEG(QImage *img, const DB::FileName& imageFile, QSize* fullSize, int dim)
{
    FILE* inputFile=fopen( QFile::encodeName(imageFile.absolute()), "rb");
    if(!inputFile)
        return false;
    bool ok = loadJPEG( img, inputFile, fullSize, dim );
    fclose(inputFile);
    return ok;
}

bool Utilities::loadJPEG(QImage *img, FILE* inputFile, QSize* fullSize, int dim )
{
    struct jpeg_decompress_struct    cinfo;
    struct myjpeg_error_mgr jerr;

    // JPEG error handling - thanks to Marcus Meissner
    cinfo.err             = jpeg_std_error(&jerr);
    cinfo.err->error_exit = myjpeg_error_exit;

    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        return false;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, inputFile);
    jpeg_read_header(&cinfo, TRUE);
    *fullSize = QSize( cinfo.image_width, cinfo.image_height );

    int imgSize = qMax(cinfo.image_width, cinfo.image_height);

    //libjpeg supports a sort of scale-while-decoding which speeds up decoding
    int scale=1;
    if (dim != -1) {
        while(dim*scale*2<=imgSize) {
            scale*=2;
        }
        if(scale>8) scale=8;
    }

    cinfo.scale_num=1;
    cinfo.scale_denom=scale;

    // Create QImage
    jpeg_start_decompress(&cinfo);

    switch(cinfo.output_components) {
    case 3:
    case 4:
        *img = QImage(
            cinfo.output_width, cinfo.output_height, QImage::Format_RGB32);
        if (img->isNull())
            return false;
        break;
    case 1: // B&W image
        *img = QImage(
            cinfo.output_width, cinfo.output_height, QImage::Format_Indexed8);
        if (img->isNull())
            return false;
        img->setNumColors(256);
        for (int i=0; i<256; i++)
            img->setColor(i, qRgb(i,i,i));
        break;
    default:
        return false;
    }

    QVector<uchar*> linesVector;
    linesVector.reserve(img->height());
    for (int i = 0; i < img->height(); ++i)
        linesVector.push_back(img->scanLine(i));
    uchar** lines = linesVector.data();
    while (cinfo.output_scanline < cinfo.output_height)
        jpeg_read_scanlines(&cinfo, lines + cinfo.output_scanline,
                            cinfo.output_height);
    jpeg_finish_decompress(&cinfo);

    // Expand 24->32 bpp
    if ( cinfo.output_components == 3 ) {
        for (uint j=0; j<cinfo.output_height; j++) {
            uchar *in = img->scanLine(j) + cinfo.output_width*3;
            QRgb *out = (QRgb*)( img->scanLine(j) );

            for (uint i=cinfo.output_width; i--; ) {
                in-=3;
                out[i] = qRgb(in[0], in[1], in[2]);
            }
        }
    }

    /*int newMax = qMax(cinfo.output_width, cinfo.output_height);
      int newx = size_*cinfo.output_width / newMax;
      int newy = size_*cinfo.output_height / newMax;*/

    jpeg_destroy_decompress(&cinfo);

    //image = img.smoothScale(newx,newy);
    return true;
}

bool Utilities::isJPEG( const DB::FileName& fileName )
{
    QString format= QString::fromLocal8Bit( QImageReader::imageFormat( fileName.relative() ) );
    return format == QString::fromLocal8Bit( "jpeg" );
}

namespace Utilities
{
QString normalizedFileName( const QString& fileName )
{
    return QFileInfo(fileName).absoluteFilePath();
}

QString dereferenceSymLinks( const QString& fileName )
{
    QFileInfo fi(fileName);
    int rounds = 256;
    while (fi.isSymLink() && --rounds > 0)
        fi = QFileInfo(fi.readLink());
    if (rounds == 0)
        return QString();
    return fi.filePath();
}
}

bool Utilities::areSameFile( const QString fileName1, const QString fileName2 )
{
    if (fileName1 == fileName2)
        return true;

    // If filenames are symbolic links, relative or contain more than
    // one consecutive slashes, above test won't work, so try with
    // normalized filenames.
    return (normalizedFileName(dereferenceSymLinks(fileName1)) ==
            normalizedFileName(dereferenceSymLinks(fileName2)));

    // FIXME: Hard links. Different paths to same file (with symlinks)
    // Maybe use inode numbers to solve those problems?
}

QString Utilities::stripEndingForwardSlash( const QString& fileName )
{
    if ( fileName.endsWith( QString::fromLatin1( "/" ) ) )
        return fileName.left( fileName.length()-1);
    else
        return fileName;
}

QString Utilities::relativeFolderName( const QString& fileName)
{
    int index= fileName.lastIndexOf( QChar::fromLatin1('/'), -1);
    if (index == -1)
        return QString();
    else
        return fileName.left( index );
}

bool Utilities::runningDemo()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    return args->isSet( "demo" );
}

void Utilities::deleteDemo()
{
    QString dir = QString::fromLatin1( "%1/kphotoalbum-demo-%2" ).arg(QDir::tempPath()).arg(QString::fromLocal8Bit( qgetenv( "LOGNAME" ) ) );
    KUrl url;
    url.setPath( dir );
    (void) KIO::NetAccess::del( dir, MainWindow::Window::theMainWindow() );
}

QString Utilities::stripImageDirectory( const QString& fileName )
{
    if ( fileName.startsWith( Settings::SettingsData::instance()->imageDirectory() ) )
        return fileName.mid( Settings::SettingsData::instance()->imageDirectory().length() );
    else
        return fileName;
}

QString Utilities::absoluteImageFileName( const QString& relativeName )
{
    return stripEndingForwardSlash( Settings::SettingsData::instance()->imageDirectory() ) + QString::fromLatin1( "/" ) + relativeName;
}

QString Utilities::imageFileNameToAbsolute( const QString& fileName )
{
    if ( fileName.startsWith( Settings::SettingsData::instance()->imageDirectory() ) )
        return fileName;
    else if ( fileName.startsWith( QString::fromAscii("file://") ) )
        return imageFileNameToAbsolute( fileName.mid( 7 ) ); // 7 == length("file://")
    else if ( fileName.startsWith( QString::fromAscii("/") ) )
        return QString(); // Not within our image root
    else
        return absoluteImageFileName( fileName );
}

bool operator>( const QPoint& p1, const QPoint& p2)
{
    return p1.y() > p2.y() || (p1.y() == p2.y() && p1.x() > p2.x() );
}

bool operator<( const QPoint& p1, const QPoint& p2)
{
    return p1.y() < p2.y() || ( p1.y() == p2.y() && p1.x() < p2.x() );
}

bool Utilities::isVideo( const DB::FileName& fileName )
{
    static StringSet videoExtensions;
    if ( videoExtensions.empty() ) {
        videoExtensions.insert( QString::fromLatin1( "3gp" ) );
        videoExtensions.insert( QString::fromLatin1( "avi" ) );
        videoExtensions.insert( QString::fromLatin1( "mp4" ) );
        videoExtensions.insert( QString::fromLatin1( "m4v" ) );
        videoExtensions.insert( QString::fromLatin1( "mpeg" ) );
        videoExtensions.insert( QString::fromLatin1( "mpg" ) );
        videoExtensions.insert( QString::fromLatin1( "qt" ) );
        videoExtensions.insert( QString::fromLatin1( "mov" ) );
        videoExtensions.insert( QString::fromLatin1( "moov" ) );
        videoExtensions.insert( QString::fromLatin1( "qtvr" ) );
        videoExtensions.insert( QString::fromLatin1( "rv" ) );
        videoExtensions.insert( QString::fromLatin1( "3g2" ) );
        videoExtensions.insert( QString::fromLatin1( "fli" ) );
        videoExtensions.insert( QString::fromLatin1( "flc" ) );
        videoExtensions.insert( QString::fromLatin1( "mkv" ) );
        videoExtensions.insert( QString::fromLatin1( "mng" ) );
        videoExtensions.insert( QString::fromLatin1( "asf" ) );
        videoExtensions.insert( QString::fromLatin1( "asx" ) );
        videoExtensions.insert( QString::fromLatin1( "wmp" ) );
        videoExtensions.insert( QString::fromLatin1( "wmv" ) );
        videoExtensions.insert( QString::fromLatin1( "ogm" ) );
        videoExtensions.insert( QString::fromLatin1( "rm" ) );
        videoExtensions.insert( QString::fromLatin1( "flv" ) );
        videoExtensions.insert( QString::fromLatin1( "webm" ) );
        videoExtensions.insert( QString::fromLatin1( "mts" ) );
        videoExtensions.insert( QString::fromLatin1( "ogg" ) );
        videoExtensions.insert( QString::fromLatin1( "ogv" ) );
    }

    QFileInfo fi( fileName.relative() );
    QString ext = fi.suffix().toLower();
    return videoExtensions.contains( ext );
}

bool Utilities::isRAW( const DB::FileName& fileName )
{
    return ImageManager::RAWImageDecoder::isRAW( fileName );
}


QImage Utilities::scaleImage(const QImage &image, int w, int h, Qt::AspectRatioMode mode )
{
    return image.scaled( w, h, mode, Settings::SettingsData::instance()->smoothScale() ? Qt::SmoothTransformation : Qt::FastTransformation );
}

QImage Utilities::scaleImage(const QImage &image, const QSize& s, Qt::AspectRatioMode mode )
{
    return scaleImage( image, s.width(), s.height(), mode );
}

QString Utilities::cStringWithEncoding( const char *c_str, const QString& charset )
{
    QTextCodec* codec = QTextCodec::codecForName( charset.toAscii() );
    if (!codec)
        codec = QTextCodec::codecForLocale();
    return codec->toUnicode( c_str );
}

DB::MD5 Utilities::MD5Sum( const DB::FileName& fileName )
{
    QFile file( fileName.absolute() );
    if ( !file.open( QIODevice::ReadOnly ) )
        return DB::MD5();

    KMD5 md5calculator( 0 /* char* */);
    md5calculator.reset();
    md5calculator.update( file );
    return DB::MD5(QString::fromLatin1(md5calculator.hexDigest()));
}

QColor Utilities::contrastColor( const QColor& col )
{
    if ( col.red() < 127 && col.green() < 127 && col.blue() < 127 )
        return Qt::white;
    else
        return Qt::black;
}
