#ifndef FAVICON_H_
#define FAVICON_H_
#include <QIcon>
#include <QObject>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPixmap>
#include <QByteArray>
#include <QDebug>
#include <QRect>
#include <QPainter>
#include <QTimer>
#include <cmath>

#include <siilihai/parser/parserengine.h>

class ForumThread;
class ForumSubscription;

class Favicon : public QObject {
    Q_OBJECT

public:
    Favicon(QObject *parent, ForumSubscription *fs);
    void fetchIcon(const QUrl &url, const QPixmap &alt);
    virtual ~Favicon();
public slots:
    void replyReceived(QNetworkReply *reply);
    void update();
    void subscriptionChanged();/*
    void engineStateChanged(UpdateEngine::UpdateEngineState newState);
    void engineStateChanged(UpdateEngine *engine, UpdateEngine::UpdateEngineState newState);*/
signals:
    void iconChanged(ForumSubscription *e, QIcon newIcon);
private:
    ForumSubscription *subscription;
    bool reloading;//, engineSet;
    QNetworkAccessManager nam;
    QPixmap currentpic;
    float currentProgress;
    float blinkAngle;
    QTimer blinkTimer;
};

#endif /* FAVICON_H_ */
