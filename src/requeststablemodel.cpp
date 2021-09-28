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

#include "requeststablemodel.h"
#include <QDebug>
#include <QDateTime>
#include <QSize>
#include <QPixmap>
#include <QFontMetrics>
#include <QApplication>


RequestsTableModel::RequestsTableModel(OKJSongbookAPI *sbApi, QObject *parent) :
    QAbstractTableModel(parent)
{
    songbookApi = sbApi;;
    connect(songbookApi, SIGNAL(requestsChanged(OkjsRequests)), this, SLOT(requestsChanged(OkjsRequests)));
    connect(songbookApi, SIGNAL(alertReceived(QString, QString)), this, SIGNAL(alertReceived(QString, QString)));
}

void RequestsTableModel::requestsChanged(OkjsRequests requests)
{
    emit layoutAboutToBeChanged();
    m_requests.clear();
    for (int i=0; i < requests.size(); i++)
    {
        int index = requests.at(i).requestId;
        QString singer = requests.at(i).singer;
        QString artist = requests.at(i).artist;
        QString title = requests.at(i).title;
        int reqtime = requests.at(i).time;
        int key = requests.at(i).key;
        m_requests << Request(index,singer,artist,title,reqtime,key);
    }
    emit layoutChanged();
}

int RequestsTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_requests.size();
}

int RequestsTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 8;
}

QVariant RequestsTableModel::data(const QModelIndex &index, int role) const
{
    QFont font = QApplication::font();
    QSize sbSize(QFontMetrics(font).height(), QFontMetrics(font).height());

    if(!index.isValid())
        return QVariant();

    if(index.row() >= m_requests.size() || index.row() < 0)
        return QVariant();
    if (role == Qt::ToolTipRole)
    {
        switch(index.column())
        {
        case DELETE:
            return QString("Delete request");
        case COPY:
            return QString("Copy artist and title to clipboard");
        case SEARCH:
            return QString("Search for song online in the OpenKJ Database");
        }
    }
    if (role == Qt::DecorationRole)
    {
        switch(index.column())
        {
        case DELETE:
            return QPixmap(":/resources/edit-delete.png").scaled(sbSize);
        case COPY:
            return QPixmap(":/resources/edit-copy-symbolic.symbolic.png").scaled(sbSize);
        case SEARCH:
            return QPixmap(":/resources/web-arrow-icon.png").scaled(sbSize);
        }
    }
    if (role == Qt::TextAlignmentRole)
        switch(index.column())
        {
        case KEY:
            return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
        default:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    if(role == Qt::DisplayRole)
    {
        switch(index.column())
        {
        case SINGER:
            return m_requests.at(index.row()).singer();
        case ARTIST:
            return m_requests.at(index.row()).artist();
        case TITLE:
            return m_requests.at(index.row()).title();
        case KEY:
            if (m_requests.at(index.row()).key() > 0)
                return "+" + QString::number(m_requests.at(index.row()).key());
            else if (m_requests.at(index.row()).key() == 0)
                return "";
            else
                return QString::number(m_requests.at(index.row()).key());
        case TIMESTAMP:
            QDateTime ts;
            ts.setTime_t(m_requests.at(index.row()).timeStamp());
            return ts.toString("M-d-yy h:mm ap");
        }
    }
    if (role == Qt::UserRole)
        return m_requests.at(index.row()).requestId();
    return QVariant();
}

QVariant RequestsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);
    if (role == Qt::DisplayRole)
    {
        switch(section) {
        case SINGER:
            return "Singer";
        case ARTIST:
            return "Artist";
        case TITLE:
            return "Title";
        case KEY:
            return "Key";
        case TIMESTAMP:
            return "Received";
        }
    }
    return QVariant();
}

Qt::ItemFlags RequestsTableModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int RequestsTableModel::count()
{
    return m_requests.count();
}

int Request::key() const
{
    return m_key;
}

void Request::setKey(int key)
{
    m_key = key;
}

Request::Request(int RequestId, QString Singer, QString Artist, QString Title, int ts, int key)
{
    m_requestId = RequestId;
    m_singer = Singer;
    m_artist = Artist;
    m_title = Title;
    m_timeStamp = ts;
    m_key = key;
}

int Request::requestId() const
{
    return m_requestId;
}

void Request::setRequestId(int requestId)
{
    m_requestId = requestId;
}

int Request::timeStamp() const
{
    return m_timeStamp;
}

void Request::setTimeStamp(int timeStamp)
{
    m_timeStamp = timeStamp;
}

QString Request::artist() const
{
    return m_artist;
}

void Request::setArtist(const QString &artist)
{
    m_artist = artist;
}

QString Request::title() const
{
    return m_title;
}

void Request::setTitle(const QString &title)
{
    m_title = title;
}

QString Request::singer() const
{
    return m_singer;
}

void Request::setSinger(const QString &singer)
{
    m_singer = singer;
}




