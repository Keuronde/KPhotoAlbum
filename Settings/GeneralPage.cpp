#include "GeneralPage.h"
#include <DB/ImageDB.h>
#include <DB/Category.h>
#include <KComboBox>
#include <klocale.h>
#include <QSpinBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QWidget>
#include <Q3VGroupBox>
#include <QVBoxLayout>
#include <KLineEdit>
#include "DB/CategoryCollection.h"
#include "SettingsData.h"

Settings::GeneralPage::GeneralPage( QWidget* parent )
    : QWidget( parent )
{
    QVBoxLayout* lay1 = new QVBoxLayout( this );

    Q3VGroupBox* box = new Q3VGroupBox( i18n( "New Images" ), this );
    lay1->addWidget( box );

    // Thrust time stamps
    QWidget* container = new QWidget( box );
    QLabel* timeStampLabel = new QLabel( i18n("Trust image dates:"), container );
    _trustTimeStamps = new KComboBox( container );
    _trustTimeStamps->addItems( QStringList() << i18n("Always") << i18n("Ask") << i18n("Never") );
    timeStampLabel->setBuddy( _trustTimeStamps );
    QHBoxLayout* hlay = new QHBoxLayout( container );
    hlay->addWidget( timeStampLabel );
    hlay->addWidget( _trustTimeStamps );
    hlay->addStretch( 1 );

    // Do EXIF rotate
    _useEXIFRotate = new QCheckBox( i18n( "Use EXIF orientation information" ), box );

    _useEXIFComments = new QCheckBox( i18n( "Use EXIF description" ), box );

    // Search for images on startup
    _searchForImagesOnStart = new QCheckBox( i18n("Search for new images and videos on startup"), box );
    _skipRawIfOtherMatches = new QCheckBox( i18n("Do not read RAW files if a matching JPEG/TIFF file exists"), box );

    // Use embedded thumbnail
    _useRawThumbnail = new QCheckBox( i18n("Use the embedded thumbnail in RAW file or halfsized RAW"), box );
    QWidget* sizeBox = new QWidget( box );
    QHBoxLayout* lay2 = new QHBoxLayout( sizeBox );

    QLabel* label = new QLabel( i18n("Required size for the thumbnail:"), sizeBox );
    lay2->addWidget( label );

    _useRawThumbnailWidth = new QSpinBox;
    _useRawThumbnailWidth->setRange( 100, 5000 );
    _useRawThumbnailWidth->setSingleStep( 64 );
    lay2->addWidget( _useRawThumbnailWidth );

    label = new QLabel( QString::fromLatin1("x"), sizeBox );
    lay2->addWidget( label );

    _useRawThumbnailHeight = new QSpinBox;
    _useRawThumbnailHeight->setRange( 100, 5000 );
    _useRawThumbnailHeight->setSingleStep( 64 );
    lay2->addWidget( _useRawThumbnailHeight );

    lay2->addStretch( 1 );

    // Exclude directories from search
    QLabel* excludeDirectoriesLabel = new QLabel( i18n("Directories to exclude from search:" ), box );
    _excludeDirectories = new KLineEdit( box );
    excludeDirectoriesLabel->setBuddy( _excludeDirectories );

    // Original/Modified File Support
    Q3VGroupBox* modifiedBox = new Q3VGroupBox( i18n("Modified File Detection Settings"), this );
    lay1->addWidget( modifiedBox );

    _detectModifiedFiles = new QCheckBox(i18n("Try and detect modified files"), modifiedBox);

    QLabel* modifiedFileComponentLabel = new QLabel( i18n("Modified file search regexp:" ), modifiedBox );
    _modifiedFileComponent = new QLineEdit(modifiedBox);

    QLabel* originalFileComponentLabel = new QLabel( i18n("Original file replacement text:" ), modifiedBox );
    _originalFileComponent = new QLineEdit(modifiedBox);

    _moveOriginalContents = new QCheckBox(i18n("Move meta-data (i.e. delete tags from the original):"), modifiedBox);

    _autoStackNewFiles = new QCheckBox(i18n("Auto-stack new files on top of old:"), modifiedBox);

    QLabel* copyFileComponentLabel = new QLabel( i18n("Copy file search regexp:" ), modifiedBox );
    _copyFileComponent = new QLineEdit(modifiedBox);

    QLabel* copyFileReplacementComponentLabel = new QLabel( i18n("Copy file replacement text:" ), modifiedBox );
    _copyFileReplacementComponent = new QLineEdit(modifiedBox);


    // Datebar size
    container = new QWidget( this );
    lay1->addWidget( container );
    hlay = new QHBoxLayout( container );
    QLabel* datebarSize = new QLabel( i18n("Size of histogram columns in date bar:"), container );
    hlay->addWidget( datebarSize );
    _barWidth = new QSpinBox;
    _barWidth->setRange( 1, 100 );
    _barWidth->setSingleStep( 1 );
    hlay->addWidget( _barWidth );
    datebarSize->setBuddy( _barWidth );
    QLabel* cross = new QLabel( QString::fromLatin1( " x " ), container );
    hlay->addWidget( cross );
    _barHeight = new QSpinBox;
    _barHeight->setRange( 15, 100 );
    hlay->addWidget( _barHeight );
    hlay->addStretch( 1 );

    // Show splash screen
    _showSplashScreen = new QCheckBox( i18n("Show splash screen"), this );
    lay1->addWidget( _showSplashScreen );

    // Album Category
    QLabel* albumCategoryLabel = new QLabel( i18n("Category for virtual albums:" ), this );
    _albumCategory = new QComboBox;
    albumCategoryLabel->setBuddy( _albumCategory );
    QHBoxLayout* lay7 = new QHBoxLayout;
    lay1->addLayout( lay7 );

    lay7->addWidget( albumCategoryLabel );
    lay7->addWidget( _albumCategory );
    lay7->addStretch(1);

     QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
     for( QList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
        _albumCategory->addItem( (*it)->text() );
    }

    lay1->addStretch( 1 );


    // Whats This
    QString txt;

    txt = i18n( "<p>KPhotoAlbum will try to read the image date from EXIF information in the image. "
                "If that fails it will try to get the date from the file's time stamp.</p>"
                "<p>However, this information will be wrong if the image was scanned in (you want the date the image "
                "was taken, not the date of the scan).</p>"
                "<p>If you only scan images, in contrast to sometimes using "
                "a digital camera, you should reply <b>no</b>. If you never scan images, you should reply <b>yes</b>, "
                "otherwise reply <b>ask</b>. This will allow you to decide whether the images are from "
                "the scanner or the camera, from session to session.</p>" );
    timeStampLabel->setWhatsThis( txt );
    _trustTimeStamps->setWhatsThis( txt );

    txt = i18n( "<p>JPEG images may contain information about rotation. "
                "If you have a reason for not using this information to get a default rotation of "
                "your images, uncheck this check box.</p>"
                "<p>Note: Your digital camera may not write this information into the images at all.</p>" );
    _useEXIFRotate->setWhatsThis( txt );

    txt = i18n( "<p>JPEG images may contain a description. "
               "Check this checkbox to specify if you want to use this as a "
               "default description for your images.</p>" );
    _useEXIFComments->setWhatsThis( txt );

    txt = i18n( "<p>KPhotoAlbum is capable of searching for new images and videos when started, this does, "
                "however, take some time, so instead you may wish to manually tell KPhotoAlbum to search for new images "
                "using <b>Maintenance->Rescan for new images</b></p>");
    _searchForImagesOnStart->setWhatsThis( txt );

    txt = i18n( "<p>KPhotoAlbum is capable of reading certain kinds of RAW images.  "
		"Some cameras store both a RAW image and a matching JPEG or TIFF image.  "
		"This causes duplicate images to be stored in KPhotoAlbum, which may be undesirable.  "
		"If this option is checked, KPhotoAlbum will not read RAW files for which matching image files also exist.</p>");
    _skipRawIfOtherMatches->setWhatsThis( txt );

    txt = i18n("<p>KPhotoAlbum shares plugins with other imaging applications, some of which have the concept of albums. "
               "KPhotoAlbum does not have this concept; nevertheless, for certain plugins to function, KPhotoAlbum behaves "
               "to the plugin system as if it did.</p>"
               "<p>KPhotoAlbum does this by defining the current album to be the current view - that is, all the images the "
               "browser offers to display.</p>"
               "<p>In addition to the current album, KPhotoAlbum must also be able to give a list of all albums; "
               "the list of all albums is defined in the following way:"
               "<ul><li>When KPhotoAlbum's browser displays the content of a category, say all People, then each item in this category "
               "will look like an album to the plugin.</li>"
               "<li>Otherwise, the category you specify using this option will be used; e.g. if you specify People "
               "with this option, then KPhotoAlbum will act as if you had just chosen to display people and then invoke "
               "the plugin which needs to know about all albums.</li></ul></p>"
               "<p>Most users would probably want to specify Events here.</p>");
    albumCategoryLabel->setWhatsThis( txt );
    _albumCategory->setWhatsThis( txt );

    txt = i18n( "Show the KPhotoAlbum splash screen on start up" );
    _showSplashScreen->setWhatsThis( txt );

    txt = i18n( "<p>When KPhotoAlbum searches for new files and finds a file that matches the <i>modified file search regexp</i> it is assumed that an original version of the image may exist.  The regexp pattern will be replaced with the <i>original file string</i> text and if that file exists, all associated metadata (category information, ratings, etc) will be copied from the original file to the new one.</p>");
    _detectModifiedFiles->setWhatsThis( txt );
    modifiedFileComponentLabel->setWhatsThis( txt );
    _modifiedFileComponent->setWhatsThis( txt );
    originalFileComponentLabel->setWhatsThis( txt );
    _originalFileComponent->setWhatsThis( txt );
    _moveOriginalContents->setWhatsThis( txt );
    _autoStackNewFiles->setWhatsThis( txt );

    txt = i18n("<p>KPhotoAlbum can make a copy of an image before opening it with an external program.  These settings set the original rexexp to search for and contents to replace it with when deciding what the new filename should be.</p>");
    copyFileComponentLabel->setWhatsThis( txt );
    _copyFileComponent->setWhatsThis( txt );
    copyFileReplacementComponentLabel->setWhatsThis( txt );
    _copyFileReplacementComponent->setWhatsThis( txt );
}

void Settings::GeneralPage::loadSettings( Settings::SettingsData* opt )
{
    _trustTimeStamps->setCurrentIndex( opt->tTimeStamps() );
    _useEXIFRotate->setChecked( opt->useEXIFRotate() );
    _useEXIFComments->setChecked( opt->useEXIFComments() );
    _searchForImagesOnStart->setChecked( opt->searchForImagesOnStart() );
    _skipRawIfOtherMatches->setChecked( opt->skipRawIfOtherMatches() );
    _useRawThumbnail->setChecked( opt->useRawThumbnail() );
    setUseRawThumbnailSize(QSize(opt->useRawThumbnailSize().width(), opt->useRawThumbnailSize().height()));
    _barWidth->setValue( opt->histogramSize().width() );
    _barHeight->setValue( opt->histogramSize().height() );
    _showSplashScreen->setChecked( opt->showSplashScreen() );
    _excludeDirectories->setText( opt->excludeDirectories() );
    DB::CategoryPtr cat = DB::ImageDB::instance()->categoryCollection()->categoryForName( opt->albumCategory() );
    if ( !cat )
        cat = DB::ImageDB::instance()->categoryCollection()->categories()[0];

    _albumCategory->setEditText( cat->text() );
    _detectModifiedFiles->setChecked( opt->detectModifiedFiles() );
    _modifiedFileComponent->setText( opt->modifiedFileComponent() );
    _originalFileComponent->setText( opt->originalFileComponent() );
    _moveOriginalContents->setChecked( opt->moveOriginalContents() );
    _autoStackNewFiles->setChecked( opt->autoStackNewFiles() );
    _copyFileComponent->setText( opt->copyFileComponent() );
    _copyFileReplacementComponent->setText( opt->copyFileReplacementComponent() );

}

void Settings::GeneralPage::saveSettings( Settings::SettingsData* opt )
{
    opt->setTTimeStamps( (TimeStampTrust) _trustTimeStamps->currentIndex() );
    opt->setUseEXIFRotate( _useEXIFRotate->isChecked() );
    opt->setUseEXIFComments( _useEXIFComments->isChecked() );
    opt->setSearchForImagesOnStart( _searchForImagesOnStart->isChecked() );
    opt->setSkipRawIfOtherMatches( _skipRawIfOtherMatches->isChecked() );
    opt->setUseRawThumbnail( _useRawThumbnail->isChecked() );
    opt->setUseRawThumbnailSize(QSize(useRawThumbnailSize()));
    opt->setShowSplashScreen( _showSplashScreen->isChecked() );
    opt->setExcludeDirectories( _excludeDirectories->text() );
    QString name = DB::ImageDB::instance()->categoryCollection()->nameForText( _albumCategory->currentText() );
    if ( name.isNull() )
        name = DB::ImageDB::instance()->categoryCollection()->categoryNames()[0];
    opt->setHistogramSize( QSize( _barWidth->value(), _barHeight->value() ) );

    opt->setAlbumCategory( name );
    opt->setDetectModifiedFiles( _detectModifiedFiles->isChecked() );
    opt->setModifiedFileComponent( _modifiedFileComponent->text() );
    opt->setOriginalFileComponent( _originalFileComponent->text() );
    opt->setAutoStackNewFiles( _autoStackNewFiles->isChecked() );
    opt->setCopyFileComponent( _copyFileComponent->text() );
    opt->setCopyFileReplacementComponent( _copyFileReplacementComponent->text() );
}

void Settings::GeneralPage::setUseRawThumbnailSize( const QSize& size  )
{
    _useRawThumbnailWidth->setValue( size.width() );
    _useRawThumbnailHeight->setValue( size.height() );
}

QSize Settings::GeneralPage::useRawThumbnailSize()
{
    return QSize( _useRawThumbnailWidth->value(), _useRawThumbnailHeight->value() );
}
