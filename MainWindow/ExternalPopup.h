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

#ifndef EXTERNALPOPUP_H
#define EXTERNALPOPUP_H
#include <q3popupmenu.h>
//Added by qt3to4:
#include <QPixmap>
#include "DB/ImageInfoList.h"
#include <Utilities/Set.h>
#include <qpair.h>

namespace DB
{
    class ImageInfo;
}

namespace MainWindow
{
typedef Set< QPair<QString,QString> > OfferType;

class ExternalPopup :public QMenu {
    Q_OBJECT

public:
    ExternalPopup( QWidget* parent );
    void populate( DB::ImageInfoPtr current, const QStringList& list );

protected slots:
    void slotExecuteService( QAction* );

protected:
    QString mimeType( const QString& file );
    StringSet mimeTypes( const QStringList& files );
    OfferType appInfos( const QStringList& files );

private:
    QStringList _list;
    DB::ImageInfoPtr _currentInfo;
    QMap<QString,StringSet> _appToMimeTypeMap;
};

}

bool operator<( const QPair<QString,QPixmap>& a, const QPair<QString,QPixmap>& b );

#endif /* EXTERNALPOPUP_H */

