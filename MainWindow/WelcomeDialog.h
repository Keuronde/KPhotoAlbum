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

#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <kdialog.h>
#include <qdialog.h>
class QLineEdit;

namespace MainWindow
{

class WelComeDialog : public QDialog
{
    Q_OBJECT

public:
    WelComeDialog( QWidget* parent = 0 );
    QString configFileName() const;

protected slots:
    void slotLoadDemo();
    void createSetup();
    void checkFeatures();

private:
    QString _configFile;
};


class FileDialog : public KDialog
{
    Q_OBJECT
public:
    FileDialog( QWidget* parent );
    QString getFileName();
protected slots:
    void slotBrowseForDirecory();
private:
    QLineEdit* _lineEdit;
};

}

#endif // WELCOMEDIALOG_H
