/*
 * Copyright (c) 2013-2017 Thomas Isaac Lightburn
 *
 *
 * This file is part of OpenKJ.
 *
 * OpenKJ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef REQUESTSTABLEMODEL_H
#define REQUESTSTABLEMODEL_H

#include <QAbstractTableModel>
#include <QItemDelegate>
#include <QImage>
#include "okjsongbookapi.h"
#include "settings.h"


class Request
{

private:
    int m_requestId;
    int m_timeStamp;
    QString m_artist;
    QString m_title;
    QString m_singer;
    int m_key;

public:
    Request(int RequestId, QString Singer, QString Artist, QString Title, int ts, int key);
    int requestId() const;
    void setRequestId(int requestId);
    int timeStamp() const;
    void setTimeStamp(int timeStamp);
    QString artist() const;
    void setArtist(const QString &artist);
    QString title() const;
    void setTitle(const QString &title);
    QString singer() const;
    void setSinger(const QString &singer);

    int key() const;
    void setKey(int key);
};

class ItemDelegateRequests : public QItemDelegate
{
Q_OBJECT
private:
    QImage m_iconDelete;
    QImage m_iconWebSearch;
    QImage m_iconCopy;
    int m_curFontHeight{0};
    Settings m_settings;

public:
    [[maybe_unused]] explicit ItemDelegateRequests(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

public slots:
    void resizeIconsForFont(const QFont &font);

};

class RequestsTableModel : public QAbstractTableModel
{
    Q_OBJECT

private:
    QList<Request> m_requests;
    OKJSongbookAPI *songbookApi;
    Settings m_settings;
    QFont m_curFont{m_settings.font()};
    QFontMetrics m_curFontMetrics{m_curFont};
    int m_curFontHeight{m_curFontMetrics.height()};

public:
    explicit RequestsTableModel(OKJSongbookAPI *sbApi, QObject *parent = nullptr);
    enum {SINGER=0,ARTIST,TITLE,COPY,SEARCH,TIMESTAMP,KEY,DELETE};
    int count();
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QList<Request> requests() {return m_requests;}

public slots:
    void fontChanged(const QFont &font);

private slots:
    void requestsChanged(OkjsRequests requests);

signals:
    void alertReceived(QString title, QString message);
};

#endif // REQUESTSTABLEMODEL_H
