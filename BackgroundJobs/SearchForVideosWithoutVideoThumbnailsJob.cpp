/* Copyright (C) 2012 Jesper K. Pedersen <blackie@kde.org>

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

#include "SearchForVideosWithoutVideoThumbnailsJob.h"
#include <ImageManager/VideoThumbnailsExtractor.h>
#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <QFile>
#include "CreateVideoThumbnailsJob.h"
#include <BackgroundTaskManager/JobManager.h>
#include <klocale.h>
#include <BackgroundTaskManager/JobInfo.h>
#include "ExtractOneThumbnailJob.h"
#include "ReadVideoLengthJob.h"

using namespace BackgroundJobs;

void BackgroundJobs::SearchForVideosWithoutVideoThumbnailsJob::execute()
{
    const DB::FileNameList images = DB::ImageDB::instance()->images();

    Q_FOREACH( const DB::FileName& image, images ) {
        const DB::ImageInfoPtr info = image.info();
        if ( !info->isVideo() )
            continue;

        const DB::FileName thumbnailName = ImageManager::VideoThumbnailsExtractor::frameName(info->fileName(),9);
        if ( thumbnailName.exists() )
            continue;

        BackgroundJobs::ReadVideoLengthJob* readVideoLengthJob = new BackgroundJobs::ReadVideoLengthJob(info->fileName());

        for (int i=0; i<10;++i) {
            ExtractOneThumbnailJob* extractJob = new ExtractOneThumbnailJob( info->fileName(), i );
            extractJob->addDependency(readVideoLengthJob);
        }

        BackgroundTaskManager::JobManager::instance()->addJob( readVideoLengthJob,
                                                               BackgroundTaskManager::BackgroundVideoPreviewRequest);
    }
    emit completed();
}

QString BackgroundJobs::SearchForVideosWithoutVideoThumbnailsJob::title() const
{
    return i18n("Searching for videos without video thumbnails");
}

QString BackgroundJobs::SearchForVideosWithoutVideoThumbnailsJob::details() const
{
    return QString();
}

