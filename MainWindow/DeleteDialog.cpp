/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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

#include "DeleteDialog.h"
#include <klocale.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qfile.h>
#include <QVBoxLayout>
#include <kmessagebox.h>
#include "DB/ImageDB.h"
#include "Utilities/Util.h"
#include "DB/ImageInfo.h"
#include "Utilities/ShowBusyCursor.h"

using namespace MainWindow;

DeleteDialog::DeleteDialog( QWidget* parent )
    :KDialog( parent )
{
    setWindowTitle( i18n("Delete from database") );
    setButtons( Cancel|User1 );
    setButtonText( User1,i18n("Delete") );

    QWidget* top = new QWidget;
    QVBoxLayout* lay1 = new QVBoxLayout( top );
    setMainWidget( top );


    _label = new QLabel;
    lay1->addWidget( _label );

    _delete_file = new QCheckBox( i18n( "Delete file from disk as well" ) );
    lay1->addWidget( _delete_file );

    connect( this, SIGNAL( user1Clicked() ), this, SLOT( deleteImages() ) );
}

int DeleteDialog::exec( const QStringList& list )
{
    _label->setText( i18n("<p><b><center><font size=\"+3\">Delete Images/Videos from database<br>%1 selected</font></center></b></p>", list.count() ) );

    _delete_file->setChecked( true );
    _list = list;

    return KDialog::exec();
}

void DeleteDialog::deleteImages()
{
    Utilities::ShowBusyCursor dummy;

    QStringList listToDelete;
    QStringList listCouldNotDelete;

    for( QStringList::const_iterator it = _list.constBegin(); it != _list.constEnd(); ++it ) {
        if ( DB::ImageInfo::imageOnDisk(*it) ) {
            if ( _delete_file->isChecked() && ( !QFile( *it ).exists() || !QFile( *it ).remove() ) ) {
                listCouldNotDelete.append (*it );
            } else {
                listToDelete.append( *it );
                Utilities::removeThumbNail( *it );
            }
        } else {
            listCouldNotDelete.append( *it );
        }
    }

    if( ! listCouldNotDelete.isEmpty()) {
        KMessageBox::errorList( this, i18n("<p><b>Unable to physically delete %1 file(s). Do you have permission to delete these files?</b></p>",listCouldNotDelete.count()), listCouldNotDelete,
                            i18n("Error Deleting Files") );
    }

    if( ! listToDelete.isEmpty()) {
        if ( _delete_file->isChecked() )
            DB::ImageDB::instance()->deleteList( listToDelete );
        else
            DB::ImageDB::instance()->addToBlockList( listToDelete );

        accept();

    } else {

        reject();

    }

}

#include "DeleteDialog.moc"
