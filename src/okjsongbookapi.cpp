#include "okjsongbookapi.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QMessageBox>
#include "version.h"


QDebug operator<<(QDebug dbg, const OkjsVenue &okjsvenue)
{
    dbg.nospace() << "venue_id: " << okjsvenue.venueId << " name: " << okjsvenue.name << " urlName: " << okjsvenue.urlName << " accepting: " << okjsvenue.accepting;
    return dbg.maybeSpace();
}


OKJSongbookAPI::OKJSongbookAPI(QObject *parent) : QObject(parent)
{
    serverUrl = QUrl("https://api.okjsongbook.com");
    delayErrorEmitted = false;
    connectionReset = false;
    serial = 0;
    entitledSystems = 1;
    timer = new QTimer(this);
    timer->setInterval(10000);
    alertTimer = new QTimer(this);
    alertTimer->start(300000);
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(onSslErrors(QNetworkReply*,QList<QSslError>)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onNetworkReply(QNetworkReply*)));
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
    connect(alertTimer, SIGNAL(timeout()), this, SLOT(alertTimerTimeout()));
    getEntitledSystemCount();
    refreshVenues();
    timer->start();
    alertCheck();

}

void OKJSongbookAPI::getSerial()
{
    QJsonObject mainObject;
    mainObject.insert("api_key", settings.apiKey());
    mainObject.insert("command","getSerial");
    QJsonDocument jsonDocument;
    jsonDocument.setObject(mainObject);
    QNetworkRequest request(serverUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->post(request, jsonDocument.toJson());
}

void OKJSongbookAPI::refreshRequests()
{
    QJsonObject jsonObject;
    jsonObject.insert("api_key", settings.apiKey());
    jsonObject.insert("command","getRequests");
    jsonObject.insert("venue_id", settings.lastVenue());
    QJsonDocument jsonDocument;
    jsonDocument.setObject(jsonObject);
    QNetworkRequest request(serverUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->post(request, jsonDocument.toJson());
}

void OKJSongbookAPI::removeRequest(int requestId)
{
    QJsonObject mainObject;
    mainObject.insert("api_key", settings.apiKey());
    mainObject.insert("command","deleteRequest");
    mainObject.insert("venue_id", settings.lastVenue());
    mainObject.insert("request_id", requestId);
    QJsonDocument jsonDocument;
    jsonDocument.setObject(mainObject);
    QNetworkRequest request(serverUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->post(request, jsonDocument.toJson());
}

bool OKJSongbookAPI::getAccepting()
{
    for (const auto & venue : venues)
    {
        if (venue.venueId == settings.lastVenue())
            return venue.accepting;
    }
    return false;
}

void OKJSongbookAPI::setAccepting(bool enabled)
{
    alertCheck();
    QJsonObject mainObject;
    mainObject.insert("api_key", settings.apiKey());
    mainObject.insert("command","setAccepting");
    mainObject.insert("venue_id", settings.lastVenue());
    mainObject.insert("accepting", enabled);
    mainObject.insert("system_id", settings.systemId());
    QJsonDocument jsonDocument;
    jsonDocument.setObject(mainObject);
    QNetworkRequest request(serverUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->post(request, jsonDocument.toJson());
}

void OKJSongbookAPI::refreshVenues(bool blocking)
{
    QJsonObject mainObject;
    mainObject.insert("api_key", settings.apiKey());
    mainObject.insert("command","getVenues");
    QJsonDocument jsonDocument;
    jsonDocument.setObject(mainObject);
    QNetworkRequest request(serverUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = manager->post(request, jsonDocument.toJson());
    if (blocking)
    {
        while (!reply->isFinished())
            QApplication::processEvents();
    }
}

void OKJSongbookAPI::clearRequests()
{
    QJsonObject mainObject;
    mainObject.insert("api_key", settings.apiKey());
    mainObject.insert("command","clearRequests");
    mainObject.insert("venue_id", settings.lastVenue());
    QJsonDocument jsonDocument;
    jsonDocument.setObject(mainObject);
    QNetworkRequest request(serverUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->post(request, jsonDocument.toJson());
}

void OKJSongbookAPI::updateSongDb(const OkjsSongs& songs)
{
    QList<OkjsSong> songList = songs.toList();
    emit remoteSongDbUpdateStart();
    int songsPerDoc = 1000;
    QList<QJsonDocument> jsonDocs;
    int numEntries = songList.size();
    qWarning() << "Number of songs: " << numEntries;
    int numDocs = numEntries / songsPerDoc;
    if (numEntries % songsPerDoc > 0)
        numDocs++;
    emit remoteSongDbUpdateNumDocs(numDocs);
    qWarning() << "Emitted remoteSongDbUpdateNumDocs(" << numDocs << ")";
    int docs = 0;
    for (int d=0; d < numDocs; d++)
    {
        if (songs.isEmpty())
            return;
        QApplication::processEvents();
        QJsonArray songsArray;
        int count = 0;
        for (int i=0; i < songsPerDoc; i++)
        {
            if (songList.isEmpty())
                break;
            OkjsSong song = songList.takeFirst();
            QJsonObject songObject;
            songObject.insert("artist", song.artist);
            songObject.insert("title", song.title);
            songsArray.insert(0, songObject);
            QApplication::processEvents();
            count++;
        }
        docs++;
        QJsonObject mainObject;
        mainObject.insert("api_key", settings.apiKey());
        mainObject.insert("command","addSongs");
        mainObject.insert("songs", songsArray);
        mainObject.insert("system_id", settings.systemId());
        QJsonDocument jsonDocument;
        jsonDocument.setObject(mainObject);
        jsonDocs.append(jsonDocument);
        qWarning() << "Created json doc #" << docs;
    }
    qWarning() << "Done creating docs";
    QUrl url(serverUrl);
    QJsonObject mainObject;
    mainObject.insert("api_key", settings.apiKey());
    mainObject.insert("command","clearDatabase");
    mainObject.insert("system_id", settings.systemId());
    QJsonDocument jsonDocument;
    jsonDocument.setObject(mainObject);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->post(request, jsonDocument.toJson());
    while (!reply->isFinished())
        QApplication::processEvents();
    for (int i=0; i < jsonDocs.size(); i++)
    {
        QApplication::processEvents();
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        QNetworkReply *reply = manager->post(request, jsonDocs.at(i).toJson());
        while (!reply->isFinished())
            QApplication::processEvents();
        emit remoteSongDbUpdateProgress(i + 1);
        qWarning() << "Emitted remoteSongDbUpdateProgress(" << i << ")";
    }

    emit remoteSongDbUpdateDone();
}

int OKJSongbookAPI::numRequests()
{
    return requests.size();
}

bool OKJSongbookAPI::testApiKey(const QString& key)
{
    QJsonObject mainObject;
    mainObject.insert("api_key", key);
    mainObject.insert("command","getSerial");
    QJsonDocument jsonDocument;
    jsonDocument.setObject(mainObject);
    QNetworkAccessManager m_NetworkMngr;
    QNetworkRequest request(serverUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply= m_NetworkMngr.post(request, jsonDocument.toJson());
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()),&loop, SLOT(quit()));
    loop.exec();
    if (reply->error() != QNetworkReply::NoError)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Network Error");
        msgBox.setText(reply->errorString());
        msgBox.exec();
        qWarning() << "Got network error: " << reply->errorString();
        return false;
    }
    QByteArray data = reply->readAll();
    delete reply;
    QJsonDocument json = QJsonDocument::fromJson(data);
    QString command = json.object().value("command").toString();
    bool error = json.object().value("error").toBool();
    if (error)
    {
        qWarning() << "Got error json reply";
        qWarning() << "Error string: " << json.object().value("errorString");
        return false;
    }
    if (command == "getSerial")
    {
        int newSerial = json.object().value("serial").toInt();
        if (newSerial == 0)
        {
            qWarning() << "SongbookAPI - Server didn't return valid serial";
            return false;
        }
        else
        {
            return true;
        }
    }
    return false;
}

void OKJSongbookAPI::onSslErrors(QNetworkReply *reply, const QList<QSslError>& errors)
{
    Q_UNUSED(errors)
    static QUrl lastUrl;
    static bool errorEmitted = false;
    if (lastUrl != serverUrl)
        errorEmitted = false;
    else if (!errorEmitted)
    {
        emit sslError();
        errorEmitted = true;

    }
    lastUrl = serverUrl;
}

void OKJSongbookAPI::alertCheck()
{
    QJsonObject mainObject;
    mainObject.insert("api_key", settings.apiKey());
    mainObject.insert("command","getAlert");
    mainObject.insert("app", "standalone");
    QJsonDocument jsonDocument;
    jsonDocument.setObject(mainObject);
    QNetworkRequest request(serverUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->post(request, jsonDocument.toJson());
}

void OKJSongbookAPI::versionCheck()
{
    QJsonObject mainObject;
    mainObject.insert("api_key", settings.apiKey());
    mainObject.insert("command","sacCurVersion");
    QJsonDocument jsonDocument;
    jsonDocument.setObject(mainObject);
    QNetworkRequest request(serverUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->post(request, jsonDocument.toJson());
}

void OKJSongbookAPI::getEntitledSystemCount()
{
    QJsonObject mainObject;
    mainObject.insert("api_key", settings.apiKey());
    mainObject.insert("command","getEntitledSystemCount");
    QJsonDocument jsonDocument;
    jsonDocument.setObject(mainObject);
    QNetworkRequest request(serverUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    manager->post(request, jsonDocument.toJson());
}

void OKJSongbookAPI::onNetworkReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << reply->errorString();
        //output some meaningful error msg
        return;
    }
    QByteArray data = reply->readAll();
    QJsonDocument json = QJsonDocument::fromJson(data);
    QString command = json.object().value("command").toString();
    bool error = json.object().value("error").toBool();
    if (error)
    {
        qWarning() << "Got error json reply";
        qWarning() << "Error string: " << json.object().value("errorString");
        return;
    }
    if (command == "sacCurVersion")
    {
        qWarning() << json;
        QString os = "unk";
#ifdef Q_OS_LINUX
        os = "lin";
#elif defined(Q_OS_WIN64)
        os = "win64";
#elif defined(Q_OS_WIN32)
        os = "win32";
#elif defined(Q_OS_MAC)
        os = "mac";
#endif
        if (os == "unk")
        {
            qWarning() << "Unable to determine OS platform";
            return;
        }
        QString curVersion = QString(VERSION_STRING);
        QString branch("stable");
        QString versionKey = branch + "_" + os;
        QString urlKey = branch + "_url_" + os;
        qWarning() << "version key: " << versionKey << " url key:" << urlKey;
        QString latestVersion = json.object().value(versionKey).toString();
        int latestMajor = json.object().value(versionKey + "_major").toInt();
        int latestMinor = json.object().value(versionKey + "_minor").toInt();
        int latestBuild = json.object().value(versionKey + "_build").toInt();
        QString latestVersionUrl = json.object().value(urlKey).toString();
        qWarning() << "Current Version: " << curVersion;
        qWarning() << "Latest Version: " << latestVersion;
        qWarning() << "Latest version maj: " << latestMajor << " min: " << latestMinor << " bld: " << latestBuild;
        qWarning() << "Latest Version DL URL: " << latestVersionUrl;
        if (latestMajor > VERSION_MAJOR || (latestMajor == VERSION_MAJOR && latestMinor > VERSION_MINOR) || (latestMajor == VERSION_MAJOR && latestMinor == VERSION_MINOR && latestBuild > VERSION_BUILD))
        {
            qWarning() << "Newer version available, emitting signal";
            emit newVersionAvailable(curVersion, latestVersion, branch, os, latestVersionUrl);
        }

    }
    if (command == "getAlert")
    {
        if(json.object().value("alert").toBool())
        {
            emit alertReceived(json.object().value("title").toString(), json.object().value("message").toString());
            venues.clear();
            refreshVenues();
        }
    }
    if (command == "getEntitledSystemCount")
    {
        qWarning() << json;
        entitledSystems = json.object().value("count").toInt();
        emit entitledSystemCountChanged(entitledSystems);
        qWarning() << "OKJSongbookAPI: Server reports entitled to run on " << entitledSystems << " systems";
    }
    if (command == "getSerial")
    {
        int newSerial = json.object().value("serial").toInt();
        if (newSerial == 0)
        {
            qWarning() << "SongbookAPI - Server didn't return valid serial";
            return;
        }
        if (serial == newSerial)
        {
            lastSync = QTime::currentTime();
            emit synchronized(lastSync);
        }
        else
        {
            serial = newSerial;
            refreshRequests();
            refreshVenues();
            lastSync = QTime::currentTime();
            emit synchronized(lastSync);
        }
    }
    if (command == "getVenues")
    {
        QJsonArray venuesArray = json.object().value("venues").toArray();
        OkjsVenues l_venues;
        for (auto && i : venuesArray)
        {
            OkjsVenue venue;
            QJsonObject jsonObject = i.toObject();
            venue.venueId = jsonObject.value("venue_id").toInt();
            venue.name = jsonObject.value("name").toString();
            venue.urlName = jsonObject.value("url_name").toString();
            venue.accepting = jsonObject.value("accepting").toBool();
            l_venues.append(venue);
        }
        if (venues != l_venues)
        {
            venues = l_venues;
            emit venuesChanged(venues);
        }
        lastSync = QTime::currentTime();
        emit synchronized(lastSync);
        refreshRequests();
    }
    if (command == "clearRequests")
    {
        refreshRequests();
        refreshVenues();
    }
    if (command == "getRequests")
    {
        QJsonArray requestsArray = json.object().value("requests").toArray();
        OkjsRequests l_requests;
        for (auto && i : requestsArray)
        {
            OkjsRequest request;
            QJsonObject jsonObject = i.toObject();
            request.requestId = jsonObject.value("request_id").toInt();
            request.artist = jsonObject.value("artist").toString();
            request.title = jsonObject.value("title").toString();
            request.singer = jsonObject.value("singer").toString();
            request.time = jsonObject.value("request_time").toInt();
            request.key = jsonObject.value("key_change").toInt();
            l_requests.append(request);
        }
        if (requests != l_requests)
        {
            if (containsNewRequests(requests, l_requests))
                emit newRequestsReceived();
            requests = l_requests;
            emit requestsChanged(requests);
        }
        lastSync = QTime::currentTime();
        emit synchronized(lastSync);
    }
    if (command == "setAccepting")
    {
        refreshVenues();
        refreshRequests();
    }
    if (command == "deleteRequest")
    {
        refreshRequests();
        refreshVenues();
    }
}

void OKJSongbookAPI::alertTimerTimeout()
{
    alertCheck();
}

void OKJSongbookAPI::timerTimeout()
{
        if ((lastSync.secsTo(QTime::currentTime()) > 300) && (!delayErrorEmitted))
        {
            emit delayError(lastSync.secsTo(QTime::currentTime()));
            delayErrorEmitted = true;
        }
        else if ((lastSync.secsTo(QTime::currentTime()) > 200) && (!connectionReset))
        {
            refreshRequests();
            refreshVenues();
            connectionReset = true;
        }
        else
        {
            connectionReset = false;
            delayErrorEmitted = false;
        }
        getSerial();
}

void OKJSongbookAPI::setInterval(int interval)
{
    timer->setInterval(interval * 1000);
}

bool OKJSongbookAPI::containsNewRequests(const OkjsRequests &previous, const OkjsRequests &current) {

    for (const auto &request : current)
    {
        auto result = std::find(previous.begin(), previous.end(), request);
        if (result == previous.end())
            return true;
    }
    return false;
}

bool OkjsVenue::operator ==(const OkjsVenue &v) const
{
    if (venueId != v.venueId)
        return false;
    if (name != v.name)
        return false;
    if (urlName != v.urlName)
        return false;
    if (accepting != v.accepting)
        return false;
    return true;
}

bool OkjsRequest::operator ==(const OkjsRequest& r) const
{
    if (r.requestId != requestId)
        return false;
    if (r.artist != artist)
        return false;
    if (r.title != title)
        return false;
    if (r.time != time)
        return false;
    if (r.singer != singer)
        return false;
    return true;
}
