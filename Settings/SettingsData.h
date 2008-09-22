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

#ifndef SETTINGS_SETTINGS_H
#   define SETTINGS_SETTINGS_H

#include <QPixmap>
#include "DB/ImageSearchInfo.h"
#include "DB/Category.h"
#include <config-kpa-exiv2.h>

#ifdef HAVE_EXIV2
#   include "Exif/Info.h"
#endif

#include "Utilities/Set.h"
#include <config-kpa-sqldb.h>

#ifdef SQLDB_SUPPORT
    namespace SQLDB { class DatabaseAddress; }
#endif

#define property( GET_TYPE,GET_FUNC, SET_FUNC,SET_TYPE ) \
    GET_TYPE GET_FUNC() const;                           \
    void SET_FUNC( const SET_TYPE )

#define property_copy( GET_FUNC, SET_FUNC , TYPE ) property( TYPE,GET_FUNC, SET_FUNC,TYPE  )
#define property_ref(  GET_FUNC, SET_FUNC , TYPE ) property( TYPE,GET_FUNC, SET_FUNC,TYPE& )

namespace DB
{
    class CategoryCollection;
}

namespace Settings
{
    using Utilities::StringSet;

    enum Position             { Bottom, Top, Left, Right, TopLeft, TopRight, BottomLeft, BottomRight };
    enum ViewSortType         { SortLastUse, SortAlpha };
    enum TimeStampTrust       { Always, Ask, Never};
    enum StandardViewSize     { FullSize, NaturalSize, NaturalSizeIfFits };
    enum ThumbnailAspectRatio { Aspect_1_1, Aspect_4_3, Aspect_3_2, Aspect_16_9, Aspect_3_4, Aspect_2_3, Aspect_9_16 };

    typedef const char* WindowType;
    extern const WindowType MainWindow, ConfigWindow;

class SettingsData : public QObject
{
    Q_OBJECT

public:
    static SettingsData* instance();
    static bool          ready();
    static void          setup( const QString& imageDirectory );

    /////////////////
    //// General ////
    /////////////////

    property_ref ( histogramSize                         , setHistogramSize                         , QSize          );
    property_ref ( backend                               , setBackend                               , QString        );
    property_copy( useEXIFRotate                         , setUseEXIFRotate                         , bool           );
    property_copy( useEXIFComments                       , setUseEXIFComments                       , bool           );
    property_copy( searchForImagesOnStartup              , setSearchForImagesOnStartup              , bool           );
    property_copy( dontReadRawFilesWithOtherMatchingFile , setDontReadRawFilesWithOtherMatchingFile , bool           );
    property_copy( useCompressedIndexXML                 , setUseCompressedIndexXML                 , bool           );
    property_copy( compressBackup                        , setCompressBackup                        , bool           );
    property_copy( showSplashScreen                      , setShowSplashScreen                      , bool           );
    property_copy( autoSave                              , setAutoSave                              , int            );
    property_copy( backupCount                           , setBackupCount                           , int            );
    property_copy( viewSortType                          , setViewSortType                          , ViewSortType   );
    property_copy( tTimeStamps                           , setTTimeStamps                           , TimeStampTrust );

    bool trustTimeStamps();

    ////////////////////
    //// Thumbnails ////
    ////////////////////

    property_copy( displayLabels            , setDisplayLabels           , bool                 );
    property_copy( displayCategories        , setDisplayCategories       , bool                 );
    property_copy( autoShowThumbnailView    , setAutoShowThumbnailView   , bool                 );
    property_copy( showNewestThumbnailFirst , setShowNewestFirst         , bool                 );
    property_copy( thumbnailDarkBackground  , setThumbnailDarkBackground , bool                 );
    property_copy( thumbnailDisplayGrid     , setThumbnailDisplayGrid    , bool                 );
    property_copy( previewSize              , setPreviewSize             , int                  );
    property_copy( thumbnailSpace           , setThumbnailSpace          , int                  ); // Border space around thumbnails.
    property_copy( thumbnailCacheScreens    , setThumbnailCacheScreens   , int                  );
    property_copy( thumbSize                , setThumbSize               , int                  );
    property_copy( thumbnailAspectRatio     , setThumbnailAspectRatio    , ThumbnailAspectRatio );

    size_t thumbnailCacheBytes() const;   // convenience method

    /**
     * Return an approximate figure of megabytes to cache to be able to
     * cache the amount of "screens" of caches.
     */
    static size_t thumbnailBytesForScreens(int screen);

    ////////////////
    //// Viewer ////
    ////////////////
    
    property_ref ( viewerSize                , setViewerSize                , QSize            );
    property_ref ( slideShowSize             , setSlideShowSize             , QSize            );
    property_copy( launchViewerFullScreen    , setLaunchViewerFullScreen    , bool             );
    property_copy( launchSlideShowFullScreen , setLaunchSlideShowFullScreen , bool             );
    property_copy( showInfoBox               , setShowInfoBox               , bool             );
    property_copy( showLabel                 , setShowLabel                 , bool             );
    property_copy( showDescription           , setShowDescription           , bool             );
    property_copy( showDate                  , setShowDate                  , bool             );
    property_copy( showImageSize             , setShowImageSize             , bool             );
    property_copy( showTime                  , setShowTime                  , bool             );
    property_copy( showFilename              , setShowFilename              , bool             );
    property_copy( showEXIF                  , setShowEXIF                  , bool             );
    property_copy( smoothScale               , setSmoothScale               , bool             );
    property_copy( slideShowInterval         , setSlideShowInterval         , int              );
    property_copy( viewerCacheSize           , setViewerCacheSize           , int              );
    property_copy( infoBoxWidth              , setInfoBoxWidth              , int              );
    property_copy( infoBoxHeight             , setInfoBoxHeight             , int              );
    property_copy( infoBoxPosition           , setInfoBoxPosition           , Position         );
    property_copy( viewerStandardSize        , setViewerStandardSize        , StandardViewSize );

    ////////////////////
    //// Categories ////
    ////////////////////

    property_ref( albumCategory, setAlbumCategory , QString);

    QString fileForCategoryImage ( const QString& category, QString member ) const;
    void    setCategoryImage     ( const QString& category, QString, const QImage& image );
    QPixmap categoryImage        ( const QString& category,  QString, int size ) const;

    //////////////
    //// EXIF ////
    //////////////

#ifdef HAVE_EXIV2
    property_ref( exifForViewer , setExifForViewer , StringSet );
    property_ref( exifForDialog , setExifForDialog , StringSet );
    property_ref( iptcCharset   , setIptcCharset   , QString   );
#endif

    ///////////////
    //// SQLDB ////
    ///////////////

#ifdef SQLDB_SUPPORT
    property_ref( SQLParameters, setSQLParameters , SQLDB::DatabaseAddress);
#endif

    ///////////////////////
    //// Miscellaneous ////
    ///////////////////////

    property_copy( delayLoadingPlugins, setDelayLoadingPlugins , bool);

    property_ref( fromDate , setFromDate , QDate );
    property_ref( toDate   , setToDate   , QDate );

    property_ref( HTMLBaseDir, setHTMLBaseDir , QString);
    property_ref( HTMLBaseURL, setHTMLBaseURL , QString);
    property_ref( HTMLDestURL, setHTMLDestURL , QString);

    property_ref( password, setPassword , QString);

    QString imageDirectory() const;

    QString groupForDatabase( const char* setting ) const;

    void setCurrentLock( const DB::ImageSearchInfo&, bool exclude );
    DB::ImageSearchInfo currentLock() const;

    void setLocked( bool locked, bool force );
    bool isLocked() const;
    bool lockExcludes() const;

    void  setWindowGeometry( WindowType, const QRect& geometry );
    QRect windowGeometry( WindowType ) const;

private:
    int       value( const char*    group , const char* option , int              defaultValue ) const;
    QString   value( const char*    group , const char* option , const QString&   defaultValue ) const;
    QString   value( const QString& group , const char* option , const QString&   defaultValue ) const;
    bool      value( const char*    group , const char* option , bool             defaultValue ) const;
    bool      value( const QString& group , const char* option , bool             defaultValue ) const;
    QColor    value( const char*    group , const char* option , const QColor&    defaultValue ) const;
    QSize     value( const char*    group , const char* option , const QSize&     defaultValue ) const;
    StringSet value( const char*    group , const char* option , const StringSet& defaultValue ) const;

    void setValue( const char*    group , const char* option , int              value );
    void setValue( const char*    group , const char* option , const QString&   value );
    void setValue( const QString& group , const char* option , const QString&   value );
    void setValue( const char*    group , const char* option , bool             value );
    void setValue( const QString& group , const char* option , bool             value );
    void setValue( const char*    group , const char* option , const QColor&    value );
    void setValue( const char*    group , const char* option , const QSize&     value );
    void setValue( const char*    group , const char* option , const StringSet& value );

signals:
    void locked( bool lock, bool exclude );
    void viewSortTypeChanged( Settings::ViewSortType );
    void histogramSizeChanged( const QSize& );

private:
    SettingsData( const QString& imageDirectory  );

    bool                 _trustTimeStamps;
    bool                 _hasAskedAboutTimeStamps;
    QString              _imageDirectory;
    static SettingsData* _instance;

    friend class DB::CategoryCollection;
};
} // end of namespace

#undef property
#undef property_copy
#undef property_ref


#endif /* SETTINGS_SETTINGS_H */
