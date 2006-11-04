/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "VideoDisplay.h"
#include <kmimetype.h>
#include <DB/ImageInfoPtr.h>
#include <DB/ImageInfo.h>

#include <kmediaplayer/player.h>
#include <kmimetype.h>
#include <kuserprofile.h>
#include <kdebug.h>
#include <qlayout.h>
#include <qtimer.h>
#include <MainWindow/FeatureDialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kapplication.h>


Viewer::VideoDisplay::VideoDisplay( QWidget* parent )
    :Viewer::Display( parent, "VideoDisplay" ), _playerPart( 0 )
{
}

bool Viewer::VideoDisplay::setImage( DB::ImageInfoPtr info, bool /*forward*/ )
{
    _info = info;
    // This code is inspired by similar code in Gwenview.
    delete _playerPart;
    _playerPart = 0;

    // Figure out the mime type associated to the file file name
    QString mimeType= mimeTypeForFileName(info->fileName());
    if ( mimeType.isEmpty() ) {
        showError( NoMimeType, info->fileName(), mimeType );
        return false;
    }


    // Ask for a part for this mime type
    KService::Ptr service = KServiceTypeProfile::preferredService(mimeType, QString::fromLatin1("KParts/ReadOnlyPart"));
    if (!service.data()) {
        showError( NoKPart, info->fileName(), mimeType );
        kdWarning() << "Couldn't find a KPart for " << mimeType << endl;
        return false;
    }

    QString library=service->library();
    if ( library.isNull() ) {
        showError( NoLibrary, info->fileName(), mimeType );
        kdWarning() << "The library returned from the service was null, indicating we could not display videos." << endl;
        return false;
    }

    _playerPart = KParts::ComponentFactory::createPartInstanceFromService<KParts::ReadOnlyPart>(service, this );
    if (!_playerPart) {
        showError( NoPartInstance, info->fileName(), mimeType );
        kdWarning() << "Failed to instantiate KPart from library " << library << endl;
        return false;
    }

    QWidget* widget = _playerPart->widget();
    if ( !widget ) {
        showError(NoWidget, info->fileName(), mimeType );
        return false;
    }

    _playerPart->openURL(info->fileName());

    // If the part implements the KMediaPlayer::Player interface, start
    // playing (needed for Kaboodle)
    KMediaPlayer::Player* player=dynamic_cast<KMediaPlayer::Player *>(_playerPart);
    if (player) {
        player->play();
    }

    connect( player, SIGNAL( stateChanged( int ) ), this, SLOT( stateChanged( int ) ) );

    zoomStandard();
    return true;
}

void Viewer::VideoDisplay::stateChanged( int state)
{
    if ( state == KMediaPlayer::Player::Stop )
        emit stopped();
}


QString Viewer::VideoDisplay::mimeTypeForFileName( const QString& fileName ) const
{
    QString res = KMimeType::findByURL(fileName)->name();
    if ( res == QString::fromLatin1("application/vnd.rn-realmedia") && !MainWindow::FeatureDialog::hasVideoSupport( res ) )
        res = QString::fromLatin1( "video/vnd.rn-realvideo" );

    return res;
}

void Viewer::VideoDisplay::showError( const ErrorType type, const QString& fileName, const QString& mimeType )
{
    const QString failed = QString::fromLatin1( "<font color=\"red\"><b>%1</b></font>" ).arg( i18n("Failed") );
    const QString OK = QString::fromLatin1( "<font color=\"green\"><b>%1</b></font>" ).arg( i18n("OK") );
    const QString untested = QString::fromLatin1( "<b>%1</b>" ).arg( i18n("Untested") );

    QString msg = i18n("<h1><b>Error Loading Video</font></b></h1>");
    msg += QString::fromLatin1( "<table cols=\"2\">" );
    msg += i18n("<tr><td>Finding mime type for file</td><td>%1</td><tr>").arg( type == NoMimeType ? failed :
                                                                                QString::fromLatin1("<font color=\"green\"><b>%1</b></font>")
                                                                                .arg(mimeType ) );

    msg += i18n("<tr><td>Getting a KPart for the mime type</td><td>%1</td></tr>").arg( type < NoKPart ? untested : ( type == NoKPart ? failed : OK ) );
    msg += i18n("<tr><td>Getting a library for the part</tr><td>%1</td></tr>")
           .arg( type < NoLibrary ? untested : ( type == NoLibrary ? failed : OK ) );
    msg += i18n("<tr><td>Instantiating Part</td><td>%1</td></tr>").arg( type < NoPartInstance ? untested : (type == NoPartInstance ? failed : OK ) );
    msg += i18n("<tr><td>Fetching Widget from part</td><td>%1</td></tr>")
           .arg( type < NoWidget ? untested : (type == NoWidget ? failed : OK ) );
    msg += QString::fromLatin1( "</table>" );

    int ret = KMessageBox::questionYesNo( this, msg, i18n( "Unable to show video %1" ).arg(fileName ), i18n("Show More Help"), i18n("Close") );
    if ( ret == KMessageBox::Yes )
        kapp->invokeBrowser( QString::fromLatin1("http://wiki.kde.org/tiki-index.php?page=KPhotoAlbum+Video+Support"));
}

void Viewer::VideoDisplay::zoomIn()
{
    resize( 1.25 );
}

void Viewer::VideoDisplay::zoomOut()
{
    resize( 0.8 );
}

void Viewer::VideoDisplay::zoomFull()
{
    QWidget* widget = _playerPart->widget();
    if ( !widget )
        return;

    widget->resize( size() );
    widget->move(0,0);
}

void Viewer::VideoDisplay::zoomPixelForPixel()
{
    QWidget* widget = _playerPart->widget();
    if ( !widget )
        return;

    const QSize size = _info->size();
    widget->resize( size );
    widget->move( (width() - size.width())/2, (height() - size.height())/2 );
}

void Viewer::VideoDisplay::resize( const float factor )
{
    QWidget* widget = _playerPart->widget();
    if ( !widget )
        return;

    const QSize size( static_cast<int>( factor * widget->width() ) , static_cast<int>( factor * widget->width() ) );
    widget->resize( size );
    widget->move( (width() - size.width())/2, (height() - size.height())/2 );
}

void Viewer::VideoDisplay::resizeEvent( QResizeEvent* event )
{
    Display::resizeEvent( event );

    if ( !_playerPart )
        return;

    QWidget* widget = _playerPart->widget();
    if ( !widget )
        return;

    if ( widget->width() == event->oldSize().width() || widget->height() == event->oldSize().height() )
        widget->resize( size() );
    else
        widget->resize( QMIN( widget->width(), width() ), QMIN( widget->height(), height() ) );
    widget->move( (width() - widget->width())/2, (height() - widget->height())/2 );
}

#include "VideoDisplay.moc"
