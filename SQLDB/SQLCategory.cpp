/*
  Copyright (C) 2005-2006 Jesper K. Pedersen <blackie@kde.org>
  Copyright (C) 2006 Tuomas Suutari <thsuut@utu.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program (see the file COPYING); if not, write to the
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
  MA 02110-1301 USA.
*/

#include "SQLCategory.h"
#include "QueryHelper.h"

SQLDB::SQLCategory::SQLCategory(int categoryId):
    _categoryId(categoryId)
{
}

QString SQLDB::SQLCategory::name() const
{
    return QueryHelper::instance()->categoryForId(_categoryId);
}

void SQLDB::SQLCategory::setName(const QString& name)
{
    QueryHelper::instance()->changeCategoryName(_categoryId, name);
    emit changed();
}

QString SQLDB::SQLCategory::iconName() const
{
    return QueryHelper::instance()->
        executeQuery("SELECT icon FROM category WHERE id=%s",
                     QueryHelper::Bindings() << _categoryId
                     ).firstItem().toString();
}

void SQLDB::SQLCategory::setIconName(const QString& name)
{
    QueryHelper::instance()->changeCategoryIcon(_categoryId, name);
    emit changed();
}

DB::Category::ViewSize SQLDB::SQLCategory::viewSize() const
{
    return static_cast<DB::Category::ViewSize>
        (QueryHelper::instance()->
         executeQuery("SELECT viewsize FROM category WHERE id=%s",
                      QueryHelper::Bindings() << _categoryId
                      ).firstItem().toInt());
}

void SQLDB::SQLCategory::setViewSize(ViewSize size)
{
    QueryHelper::instance()->changeCategoryViewSize(_categoryId, size);
    emit changed();
}

DB::Category::ViewType SQLDB::SQLCategory::viewType() const
{
    return static_cast<DB::Category::ViewType>
        (QueryHelper::instance()->
         executeQuery("SELECT viewtype FROM category WHERE id=%s",
                      QueryHelper::Bindings() << _categoryId
                      ).firstItem().toInt());
}

void SQLDB::SQLCategory::setViewType(ViewType type)
{
    QueryHelper::instance()->changeCategoryViewType(_categoryId, type);
    emit changed();
}

bool SQLDB::SQLCategory::doShow() const
{
    return QueryHelper::instance()->
        executeQuery("SELECT visible FROM category WHERE id=%s",
                     QueryHelper::Bindings() << _categoryId
                     ).firstItem().toInt();
}

void SQLDB::SQLCategory::setDoShow(bool b)
{
    QueryHelper::instance()->changeCategoryVisible(_categoryId, b);
    emit changed();
}

bool SQLDB::SQLCategory::isSpecialCategory() const
{
    // Special categories have their own classes, so return false here
    return false;
}

void SQLDB::SQLCategory::setSpecialCategory(bool b)
{
    // Can't be special category, so do nothing
    Q_UNUSED(b);
}

QStringList SQLDB::SQLCategory::items() const
{
    return QueryHelper::instance()->tagNamesOfCategory(_categoryId);
}

void SQLDB::SQLCategory::setItems(const QStringList& items)
{
    QueryHelper::instance()->
        executeStatement("DELETE FROM tag WHERE categoryId=%s",
                         QueryHelper::Bindings() << _categoryId);

    uint place = items.count();
    for(QStringList::const_iterator it = items.begin();
        it != items.end(); ++it ) {
        QueryHelper::instance()->
            executeStatement("INSERT INTO tag(name, categoryId, place) "
                             "VALUES(%s, %s, %s)",
                             QueryHelper::Bindings() << *it <<
                             _categoryId << place);
        --place;
    }
}

void SQLDB::SQLCategory::addItem(const QString& item)
{
    QueryHelper::instance()->insertTagFirst(_categoryId, item);
}

void SQLDB::SQLCategory::removeItem(const QString& item)
{
    QueryHelper::instance()->removeTag(_categoryId, item);
    emit itemRemoved(item);
}

void SQLDB::SQLCategory::renameItem(const QString& oldValue, const QString& newValue)
{
    QueryHelper::instance()->
        executeStatement("UPDATE tag SET name=%s "
                         "WHERE name=%s AND categoryId=%s",
                         QueryHelper::Bindings() <<
                         newValue << oldValue << _categoryId);
    emit itemRenamed(oldValue, newValue);
}

#include "SQLCategory.moc"
