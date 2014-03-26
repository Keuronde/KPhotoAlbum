#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#include "CategoryModel.h"
#include "RemoteCommand.h"
#include "SearchInfo.h"
#include "CategoryItemsModel.h"

#include <QObject>
#include <QMap>
#include <QImage>
#include <QStringList>

class QTcpSocket;

namespace RemoteControl {

class Client;

class RemoteInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool connected READ isConnected NOTIFY connectionChanged )
    Q_PROPERTY(int imageCount MEMBER m_imageCount NOTIFY imageCountChanged)
    Q_PROPERTY(RemoteControl::CategoryModel* categories MEMBER m_categories NOTIFY categoriesChanged)
    Q_PROPERTY(RemoteControl::CategoryItemsModel* categoryItemsModel MEMBER m_categoryItems NOTIFY categoryItemChanged)
    Q_PROPERTY(QImage home MEMBER m_homeImage NOTIFY homeImageChanged)
    Q_PROPERTY(QImage kphotoalbum MEMBER m_kphotoalbumImage NOTIFY kphotoalbumImageChange)
    Q_PROPERTY(QString currentPage MEMBER m_currentPage NOTIFY currentPageChanged) //PENDING(blackie) convert into an enum


public:
    static RemoteInterface& instance();
    bool isConnected() const;
    QImage image(int index) const;

public slots:
    void goHome();
    void selectCategory(const QString& category);
    void selectCategoryValue(const QString& value);
    void showThumbnails();

signals:
    void connectionChanged();
    void imageUpdated(int index);
    void imageCountChanged();
    void categoriesChanged();
    void homeImageChanged();
    void kphotoalbumImageChange();    
    void categoryItemChanged();
    void currentPageChanged();

private slots:
    void requestInitialData();
    void handleCommand(const RemoteCommand&);
    void updateImage(const ImageUpdateCommand&);
    void updateImageCount(const ImageCountUpdateCommand&);
    void updateCategoryList(const CategoryListCommand&);
    void gotCategoryItems(const CategoryItemListCommand&);
private:
    RemoteInterface();
    void setCurrentPage(const QString& page);

    Client* m_connection = nullptr;
    int m_imageCount = 0;
    QMap<int,QImage> m_imageMap;
    CategoryModel* m_categories;
    CategoryItemsModel* m_categoryItems;
    QImage m_homeImage;
    QImage m_kphotoalbumImage;
    SearchInfo m_search;
    RemoteControl::CategoryItemsModel* m_categotyItems;
    QString m_currentPage = QStringLiteral("Unconnected");
};

}

#endif // REMOTEINTERFACE_H
