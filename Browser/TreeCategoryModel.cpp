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

#include "TreeCategoryModel.h"
#include <DB/ImageDB.h>
#include <DB/CategoryItem.h>
#include "DB/Category.h"
struct Browser::TreeCategoryModel::Data
{
    Data( const QString& name )
        : name( name ), parent(nullptr) {}
    ~Data() {
        qDeleteAll( children );
    }
    void addChild( Data* child )
    {
        child->parent = this;
        children.append(child);
    }


    QString name;
    QList<Data*> children;
    Data* parent;
};

Browser::TreeCategoryModel::TreeCategoryModel( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info )
    : AbstractCategoryModel( category, info )
{
    m_data = new Data( QString() );
    createData(  m_category->itemsCategories().data(), 0 );
    if ( hasNoneEntry() ) {
        Data* data = new Data( DB::ImageDB::NONE() );
        data->parent = m_data;
        m_data->children.prepend( data );
    }
}

int Browser::TreeCategoryModel::rowCount( const QModelIndex& index ) const
{
    return indexToData(index)->children.count();
}

int Browser::TreeCategoryModel::columnCount( const QModelIndex& ) const
{
    return 3;
}

QModelIndex Browser::TreeCategoryModel::index( int row, int column, const QModelIndex & parent ) const
{
    const Data* data = indexToData(parent);
    QList<Data*> children = data->children;
    int size = children.count();
    if ( row >= size || row < 0 || column >= columnCount( parent ) || column < 0) {
        // Invalid index
        return QModelIndex();
    }
    else
        return createIndex( row, column, children[row] );
}

QModelIndex Browser::TreeCategoryModel::parent( const QModelIndex & index ) const
{
    Data* me = indexToData( index );
    if ( me == m_data )
        return QModelIndex();

    Data* parent = me->parent;
    if ( parent == m_data )
        return QModelIndex();

    Data* grandParent = parent->parent;

    return createIndex( grandParent->children.indexOf( parent ), 0, parent );
}

Browser::TreeCategoryModel::~TreeCategoryModel()
{
    delete m_data;
}

bool Browser::TreeCategoryModel::createData( DB::CategoryItem* parentCategoryItem, Data* parent )
{
    const QString name = parentCategoryItem->mp_name;
    const int imageCount = m_images.contains(name) ? m_images[name] : 0;
    const int videoCount = m_videos.contains(name) ? m_videos[name] : 0;

    Data* myData = new Data( name );
    bool anyItems = imageCount != 0 || videoCount != 0;

    for( QList<DB::CategoryItem*>::ConstIterator subCategoryIt = parentCategoryItem->mp_subcategories.constBegin();
         subCategoryIt != parentCategoryItem->mp_subcategories.constEnd(); ++subCategoryIt ) {
        anyItems = createData( *subCategoryIt, myData ) || anyItems;
    }

    if ( parent ) {
        if ( anyItems )
            parent->addChild( myData );
        else
            delete myData;
    }
    else {
        m_data = myData;
    }

    return anyItems;

}

Browser::TreeCategoryModel::Data* Browser::TreeCategoryModel::indexToData( const QModelIndex& index ) const
{
    if ( !index.isValid() )
        return m_data;
    else
        return static_cast<Browser::TreeCategoryModel::Data*>( index.internalPointer() );
}

QString Browser::TreeCategoryModel::indexToName(const QModelIndex& index ) const
{
    const Browser::TreeCategoryModel::Data* data = indexToData( index );
    return data->name;

}
// vi:expandtab:tabstop=4 shiftwidth=4:
