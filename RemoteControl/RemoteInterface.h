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

#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#include "ImageNameStore.h"
#include "RemoteCommand.h"
#include "DB/ImageSearchInfo.h"
#include <QObject>
#include <QHostAddress>
#include "ImageManager/ImageClientInterface.h"


namespace RemoteControl
{
class Server;

class RemoteInterface : public QObject, public ImageManager::ImageClientInterface
{
    Q_OBJECT
public:
    static RemoteInterface& instance();
    void pixmapLoaded(ImageManager::ImageRequest* request, const QImage& image);
    bool requestStillNeeded(const DB::FileName& fileName);


private slots:
    void handleCommand(const RemoteCommand&);

private:
    explicit RemoteInterface(QObject *parent = 0);

    void sendCategoryNames(const SearchCommand& searchInfo);
    void sendCategoryValues(const SearchCommand& search);
    void sendImageSearchResult(const SearchInfo& search);
    void requestThumbnail(const ThumbnailRequest& command);
    void cancelRequest(const CancelRequestCommand& command);
    void sendImageDetails(const RequestDetails& command);
    void sendHomePageImages(const RequestHomePageImages& command);

    DB::ImageSearchInfo convert(const RemoteControl::SearchInfo&) const;
    Server* m_connection;
    QSet<DB::FileName> m_activeReuqest;
    ImageNameStore m_imageNameStore;
};

}
#endif // REMOTEINTERFACE_H
