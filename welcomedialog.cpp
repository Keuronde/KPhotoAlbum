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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "welcomedialog.h"
#include <qlabel.h>
#include <klocale.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include "util.h"
#include <klineedit.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>
#include "kshell.h"

WelComeDialog::WelComeDialog( QWidget* parent, const char* name )
    : QDialog( parent, name, true )

{
    QVBoxLayout* lay1 = new QVBoxLayout( this, 6);
    QHBoxLayout* lay2 = new QHBoxLayout( lay1, 10 );

    QLabel* image = new QLabel( this, "image" );
    image->setMinimumSize( QSize( 273, 204 ) );
    image->setMaximumSize( QSize( 273, 204 ) );
    image->setPixmap( locate("data", QString::fromLatin1("kimdaba/pics/splash.png") ) );
    lay2->addWidget( image );

    QLabel* textLabel2 = new QLabel( this, "textLabel2" );
    lay2->addWidget( textLabel2 );
    textLabel2->setText( i18n( "<h1>Welcome to KimDaba</h1>"
                               "<p>If you are interested in trying out KimDaBa with a prebuilt set of images, "
                               "then simply choose the <b>Load Demo</b> "
                               "button. You may get to this demo at a later time from the <b>Help</b> menu.</p>"
                               "<p>Alternatively you may start making you own database of images, simply by pressing the "
                               "<b>Create my own database</b> button") );

    QHBoxLayout* lay3 = new QHBoxLayout( lay1, 6 );
    lay3->addStretch( 1 );

    QPushButton* loadDemo = new QPushButton( i18n("Load Demo"), this, "loadDemo" );
    lay3->addWidget( loadDemo );

    QPushButton* createSetup = new QPushButton( i18n("Create My Own Database..."), this );
    lay3->addWidget( createSetup );

    connect( loadDemo, SIGNAL( clicked() ), this, SLOT( slotLoadDemo() ) );
    connect( createSetup, SIGNAL( clicked() ), this, SLOT( createSetup() ) );
}


void WelComeDialog::slotLoadDemo()
{
    _configFile = Util::setupDemo();
    accept();
}

void WelComeDialog::createSetup()
{
    FileDialog dialog( this );
    _configFile = dialog.getFileName();
    accept();
}

QString WelComeDialog::configFileName() const
{
    return _configFile;
}

FileDialog::FileDialog( QWidget* parent, const char* name ) :QDialog( parent, name, true )
{
    QVBoxLayout* lay1 = new QVBoxLayout( this, 6 );
    QLabel* label = new QLabel( i18n("<qt><p>KimDaBa requires that all your images are stored with a common root directory. "
                                     "You are allowed to store your images in a directory tree under this directory. "
                                     "KimDaBa will not modify or edit any of your images, so you can simply point KimDaBa to the "
                                     "directory where you already have all your images located.</p></qt>" ), this );
    lay1->addWidget( label );

    QHBoxLayout* lay2 = new QHBoxLayout( lay1, 6 );
    label = new QLabel( i18n("Image root directory: "), this );
    lay2->addWidget( label );

    _lineEdit = new KLineEdit( this );
    _lineEdit->setText( QString::fromLatin1( "~/Images" ) );
    lay2->addWidget( _lineEdit );

    QPushButton* button = new QPushButton( QString::fromLatin1("..."), this );
    button->setMaximumWidth( 20 );
    lay2->addWidget( button );
    connect( button, SIGNAL( clicked() ), this, SLOT( slotBrowseForDirecory() ) );

    QHBoxLayout* lay3 = new QHBoxLayout( lay1, 6 );
    lay3->addStretch( 1 );
    button = new QPushButton( i18n("&OK"), this );
    button->setDefault(true);
    lay3->addWidget( button );

    connect( button, SIGNAL( clicked() ), this, SLOT( accept() ) );
}

void FileDialog::slotBrowseForDirecory()
{
    QString dir = KFileDialog::getExistingDirectory( _lineEdit->text(), this );
    if ( ! dir.isNull() )
        _lineEdit->setText( dir );
}

QString FileDialog::getFileName()
{
    bool ok = false;
    QString dir;
    while ( !ok ) {
        exec();
        dir =  KShell::tildeExpand( _lineEdit->text() );
        if ( !QFileInfo( dir ).exists() ) {
            int create = KMessageBox::questionYesNo( this, i18n("Directory does not exists, should I create it?") );
            if ( create == KMessageBox::Yes ) {
                bool ok2 = QDir().mkdir( dir );
                if ( !ok2 ) {
                    KMessageBox::sorry( this, i18n("Could not create directory %1").arg(dir) );
                }
                else
                    ok = true;
            }
        }
        else if ( !QFileInfo( dir ).isDir() ) {
            KMessageBox::sorry( this, i18n("%1 exists, but is not a directory").arg(dir) );
        }
        else
            ok = true;
    }

    QString file = dir + QString::fromLatin1("/index.xml");
    KSimpleConfig config( QString::fromLatin1("kimdaba") );
    config.writeEntry( QString::fromLatin1("configfile"), file );

    return file;
}

#include "welcomedialog.moc"
