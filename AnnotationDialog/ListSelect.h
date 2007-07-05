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

#ifndef LISTSELECT_H
#define LISTSELECT_H

#include <qstringlist.h>
//Added by qt3to4:
#include <QEvent>
#include <QLabel>
#include "Settings/SettingsData.h"
#include <qtoolbutton.h>
#include "DB/Category.h"
#include <q3listview.h>
#include "enums.h"

class QRadioButton;
class QLabel;
class QCheckBox;

namespace DB { class ImageInfo; }
namespace CategoryListView { class DragableListView; }
namespace CategoryListView { class CheckDropItem; }

namespace AnnotationDialog
{
class CompletableLineEdit;

class ListSelect :public QWidget {
    Q_OBJECT

public:
    ListSelect( const DB::CategoryPtr& category, QWidget* parent );
    QString category() const;
    QString text() const;
    void setText( const QString& );
    void setSelection( const StringSet& on, const StringSet& partiallyOn = StringSet() );
    StringSet itemsOn() const;
    StringSet itemsOff() const;
    StringSet itemsUnchanged() const;

    bool isAND() const;
    void setMode( UsageMode );

    void populate();

    void showOnlyItemsMatching( const QString& text );

public slots:
    void slotReturn();
    void slotSortDate();
    void slotSortAlpha();
    void toggleSortType();
    void rePopulate();

protected slots:
    void itemSelected( Q3ListViewItem* );
    void showContextMenu( Q3ListViewItem*, const QPoint& );
    void setViewSortType( Settings::ViewSortType );
    void limitToSelection();
    void showAllChildren();

protected:
    virtual bool eventFilter( QObject* object, QEvent* event );
    void insertItems( DB::CategoryItem* item, Q3ListViewItem* parent );
    void populateAlphabetically();
    void populateMRU();
    void configureItem( CategoryListView::CheckDropItem* item );
    bool isInputMode() const;
    StringSet itemsOfState( Q3CheckListItem::ToggleState state ) const;
    void checkItem( const QString itemText, bool );
    void ensureAllInstancesAreStateChanged( Q3ListViewItem* item );

private:
    QLabel* _label;
    DB::CategoryPtr _category;
    CompletableLineEdit* _lineEdit;
    CategoryListView::DragableListView* _listView;
    QRadioButton* _or;
    QRadioButton* _and;
    UsageMode _mode;
    QToolButton* _alphaSort;
    QToolButton* _dateSort;
};

}

#endif /* LISTSELECT_H */

