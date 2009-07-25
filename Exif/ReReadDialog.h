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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef REREADDIALOG_H
#define REREADDIALOG_H
#include <KDialog>
class QListWidget;
class QLabel;
class QCheckBox;

namespace Exif
{

class ReReadDialog :public KDialog {
    Q_OBJECT

public:
    ReReadDialog( QWidget* parent );
    int exec( const QStringList& );

protected slots:
    void readInfo();
    void warnAboutDates( bool );

private:
    QStringList _list;
    QCheckBox* _exifDB;
    QCheckBox* _date;
    QCheckBox* _orientation;
    QCheckBox* _description;
    QCheckBox* _force_date;
    QListWidget* _fileList;
};

}

#endif /* REREADDIALOG_H */

