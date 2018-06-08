#ifndef OKJSONGBOOKAPI_H
#define OKJSONGBOOKAPI_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>
#include <QDebug>
#include <QTimer>
#include "settings.h"

class OkjsSong
{
public:
    OkjsSong() {}
    OkjsSong(QString Artist, QString Title) { artist = Artist; title = Title; }
    QString artist;
    QString title;
};

inline bool operator==(const OkjsSong &s1, const OkjsSong &s2)
{
    return s1.artist.toLower() == s2.artist.toLower() && s1.title.toLower() == s2.title.toLower();
}

inline uint qHash(const OkjsSong &obj, uint seed) {
    return qHash(QString(obj.artist.toLower() + obj.title.toLower()), seed);
}

typedef QSet<OkjsSong> OkjsSongs;

class OkjsRequest
{
public:
    int requestId;
    QString singer;
    QString artist;
    QString title;
    int time;
    bool operator == (const OkjsRequest r) const;
};


typedef QList<OkjsRequest> OkjsRequests;

class OkjsVenue
{
public:
    int venueId;
    QString name;
    QString urlName;
    bool accepting;
    bool operator == (const OkjsVenue& v) const;
};

QDebug operator<<(QDebug dbg, const OkjsVenue &okjsvenue);

typedef QList<OkjsVenue> OkjsVenues;

class OKJSongbookAPI : public QObject
{
    Q_OBJECT
private:
    int serial;
    OkjsVenues venues;
    OkjsRequests requests;
    QNetworkAccessManager *manager;
    QTimer *timer;
    QTime lastSync;
    bool delayErrorEmitted;
    bool connectionReset;
    QUrl serverUrl;
    Settings settings;

public:
    explicit OKJSongbookAPI(QObject *parent = 0);
    void getSerial();
    void refreshRequests();
    void removeRequest(int requestId);
    bool getAccepting();
    void setAccepting(bool enabled);
    void refreshVenues(bool blocking = false);
    void clearRequests();
    void updateSongDb(OkjsSongs songs);
    int numRequests();

signals:
    void venuesChanged(OkjsVenues);
    void sslError();
    void remoteSongDbUpdateProgress(int);
    void remoteSongDbUpdateNumDocs(int);
    void remoteSongDbUpdateDone();
    void remoteSongDbUpdateStart();
    void requestsChanged(OkjsRequests);
    void synchronized(QTime);
    void delayError(int);

public slots:
    bool testApiKey(QString key);

private slots:
        void onSslErrors(QNetworkReply * reply, QList<QSslError> errors);
        void onNetworkReply(QNetworkReply* reply);
        void timerTimeout();
        void setInterval(int interval);
};

#endif // OKJSONGBOOKAPI_H
