/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef IMAGECONFIG_H
#define IMAGECONFIG_H
#include "imageinfo.h"
#include "imageclient.h"
#include "listselect.h"
#include "imagesearchinfo.h"
#include <kdockwidget.h>
#include <qspinbox.h>
#include "imagepreview.h"
#include "editor.h"
#include <qdialog.h>

class QSplitter;
class Viewer;
class QPushButton;
class KLineEdit;
class KDockWidget;

class ImageConfig :public QDialog {
    Q_OBJECT
public:
    ImageConfig( QWidget* parent, const char* name = 0 );
    int configure( ImageInfoList list,  bool oneAtATime );
    ImageSearchInfo search( ImageSearchInfo* search = 0 );
    void writeDockConfig( QDomElement& doc );
    void readDockConfig( QDomElement& doc );
    bool rotated() const;

signals:
    void changed();

protected slots:
    void slotRevert();
    void slotPrev();
    void slotNext();
    void slotOK();
    void slotClear();
    void viewerDestroyed();
    void slotOptions();
    void slotSaveWindowSetup();
    void slotDeleteOption( const QString&, const QString& );
    void slotRenameOption( const QString& , const QString& , const QString&  );
    virtual void reject();
    void rotateLeft();
    void rotateRight();
    void rotate( int angle );

protected:
    enum SetupType { SINGLE, MULTIPLE, SEARCH };
    void load();
    void writeToInfo();
    void setup();
    void loadInfo( const ImageSearchInfo& );
    int exec();
    virtual void closeEvent( QCloseEvent* );
    void showTornOfWindows();
    void hideTornOfWindows();
    virtual bool eventFilter( QObject*, QEvent* );
    KDockWidget* createListSel( const QString& optionGroup );
    bool hasChanges();

private:
    ImageInfoList _origList;
    QValueList<ImageInfo> _editList;
    int _current;
    SetupType _setup;
    QPtrList< ListSelect > _optionList;
    ImageSearchInfo _oldSearch;
    QSplitter* _splitter;
    Viewer* _viewer;
    int _accept;
    QValueList<KDockWidget*> _dockWidgets;
    QValueList<KDockWidget*> _tornOfWindows;
    bool _rotated;

    // Widgets
    KDockMainWindow* _dockWindow;
    KLineEdit* _imageLabel;
    QSpinBox* _dayStart;
    QSpinBox* _dayEnd;
    QSpinBox* _yearStart;
    QSpinBox* _yearEnd;
    QComboBox* _monthStart;
    QComboBox* _monthEnd;
    ImagePreview* _preview;
    QPushButton* _revertBut;
    QPushButton* _okBut;
    QPushButton* _prevBut;
    QPushButton* _nextBut;
    QPushButton* _rotateLeft;
    QPushButton* _rotateRight;
    Editor* _description;

};

#endif /* IMAGECONFIG_H */

