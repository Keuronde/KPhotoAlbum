/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef REMOTECONTROL_SEARCHINFO_H
#define REMOTECONTROL_SEARCHINFO_H

#include <QStack>
#include <QString>
#include <QDataStream>

#include <tuple>

namespace RemoteControl {

class SearchInfo
{
public:
    void addCategory(const QString& category);
    void addValue(const QString& value);
    void pop();
    void clear();
    QString currentCategory() const;
    QList<std::tuple<QString,QString>> values() const;

    friend QDataStream& operator<<(QDataStream& stream, const SearchInfo& searchInfo);
    friend QDataStream& operator>>(QDataStream& stream, SearchInfo& searchInfo);

private:
    QStack<QString> m_categories;
    QStack<QString> m_values;
};

QDataStream& operator<<(QDataStream& stream, const SearchInfo& searchInfo);
QDataStream& operator>>(QDataStream& stream, SearchInfo& searchInfo);

} // namespace RemoteControl

#endif // REMOTECONTROL_SEARCHINFO_H
