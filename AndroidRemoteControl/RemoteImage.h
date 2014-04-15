#ifndef REMOTEIMAGE_H
#define REMOTEIMAGE_H

#include <QQuickPaintedItem>
#include <QImage>
#include "Types.h"

namespace RemoteControl {

class RemoteImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(int imageId READ imageId WRITE setImageId NOTIFY imageIdChanged)
    Q_PROPERTY(int type MEMBER m_type NOTIFY typeChanged) // Should be ViewType
    Q_PROPERTY(QString label MEMBER m_label NOTIFY labelChanged)

    //Q_ENUMS(ViewType);
public:
    explicit RemoteImage(QQuickItem *parent = 0);
    void paint(QPainter *painter) override;
    int imageId() const;
    QSize size() const;
    void setLabel(const QString& label);

public slots:
    void setImageId(int imageId);

signals:
    void imageIdChanged();
    void typeChanged();
    void labelChanged();

private:
    int m_imageId;
    int m_type;
    QString m_label;
};

}
#endif // REMOTEIMAGE_H
