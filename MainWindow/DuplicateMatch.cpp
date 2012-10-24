/* Copyright 2012 Jesper K. Pedersen <blackie@kde.org>
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DuplicateMatch.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QImage>
#include <KLocale>
#include <QRadioButton>
#include "ImageManager/AsyncLoader.h"
#include <QCheckBox>
#include "DB/ImageDB.h"
#include "Utilities/DeleteFiles.h"

namespace MainWindow {

DuplicateMatch::DuplicateMatch(const DB::FileNameList& files )
{
    QVBoxLayout* topLayout = new QVBoxLayout(this);

    QHBoxLayout* horizontalLayout = new QHBoxLayout;
    topLayout->addLayout(horizontalLayout);
    m_image = new QLabel;
    horizontalLayout->addWidget(m_image);

    QVBoxLayout* rightSideLayout = new QVBoxLayout;
    horizontalLayout->addSpacing(20);
    horizontalLayout->addLayout(rightSideLayout);
    horizontalLayout->addStretch(1);
    rightSideLayout->addStretch(1);

    m_merge = new QCheckBox(i18n("Merge these images"));
    rightSideLayout->addWidget(m_merge);
    m_merge->setChecked(true);

    QWidget* options = new QWidget;
    rightSideLayout->addWidget(options);
    QVBoxLayout* optionsLayout = new QVBoxLayout(options);
    connect( m_merge, SIGNAL(toggled(bool)),options, SLOT(setEnabled(bool)));

    QLabel* label = new QLabel(i18n("Select target:"));
    optionsLayout->addWidget(label);

    bool first = true;
    Q_FOREACH(const DB::FileName& fileName, files) {
        QRadioButton* button = new QRadioButton(fileName.relative());
        button->setProperty("data",QVariant::fromValue(fileName));
        optionsLayout->addWidget(button);
        if ( first ) {
            button->setChecked(true);
            first = false;
        }
        m_buttons.append(button);
    }
    rightSideLayout->addStretch(1);

    QFrame* line = new QFrame;
    line->setFrameStyle(QFrame::HLine);
    topLayout->addWidget(line);

    ImageManager::ImageRequest* request = new ImageManager::ImageRequest(files.first(), QSize(300,300), 0, this);
    ImageManager::AsyncLoader::instance()->load(request);
}

void DuplicateMatch::pixmapLoaded(const DB::FileName&, const QSize&, const QSize&, int, const QImage& image, const bool)
{
    m_image->setPixmap( QPixmap::fromImage(image));
}

void DuplicateMatch::setMerge(bool b)
{
    m_merge->setChecked(b);
}

void DuplicateMatch::execute()
{
    if (!m_merge->isChecked())
        return;

    DB::FileName destination;
    Q_FOREACH( QRadioButton* button, m_buttons ) {
        if ( button->isChecked() ) {
            destination = button->property("data").value<DB::FileName>();
            break;
        }
    }

    DB::FileNameList list;
    Q_FOREACH( QRadioButton* button, m_buttons ) {
        if (button->isChecked())
            continue;
        DB::FileName fileName = button->property("data").value<DB::FileName>();
        DB::ImageDB::instance()->copyData(fileName, destination);
        list.append(fileName);
    }

    Utilities::DeleteFiles::deleteFiles(list, Utilities::DeleteFromDisk);
}

} // namespace MainWindow
