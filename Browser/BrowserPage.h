#ifndef BROWSERPAGE_H
#define BROWSERPAGE_H
#include "Breadcrumb.h"
#include <DB/ImageSearchInfo.h>
#include <DB/Category.h>
#include <QAbstractItemModel>

namespace Browser
{
class BrowserWidget;

enum Viewer { ShowBrowser, ShowImageViewer };

/**
 * \brief Information about a single page in the browser
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 *
 * This interface represent a single page in the browser (one that you can go
 * back/forward to using the back/forward buttons in the toolbar).
 */
class BrowserPage
{
public:
     BrowserPage( const DB::ImageSearchInfo& info, BrowserWidget* browser );
     virtual ~BrowserPage() {}

    /**
     * Construct the page. Result of activation may be to call \ref BrowserWidget::addAction.
     */
    virtual void activate() = 0;
    virtual BrowserPage* activateChild( const QModelIndex &);
    virtual Viewer viewer();
    virtual DB::Category::ViewType viewType() const;
    virtual bool isSearchable() const;
    virtual bool isViewChangeable() const;
    virtual Breadcrumb breadcrumb() const;
    virtual bool showDuringMovement() const;

    DB::ImageSearchInfo searchInfo() const;
    BrowserWidget* browser() const;

private:
    DB::ImageSearchInfo _info;
    BrowserWidget* _browser;
};

}


#endif /* BROWSERPAGE_H */
