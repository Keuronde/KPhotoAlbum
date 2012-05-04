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

#ifndef IMAGECLIENTINTERFACE_H
#define IMAGECLIENTINTERFACE_H
class QSize;
class QImage;
class QString;

namespace ImageManager
{

/**
 * An ImageClient is part of the ImageRequest and is called back when
 * an image has been loaded.
 */
class ImageClientInterface {
public:
    virtual ~ImageClientInterface();

    /**
     * Callback on loaded image.
     */
    virtual void pixmapLoaded( const QString& fileName,
                               const QSize& size, const QSize& fullSize,
                               int angle, const QImage& image,
                               const bool loadedOK) = 0;
    virtual void requestCanceled() {}
};

}

#endif /* IMAGECLIENTINTERFACE_H */
