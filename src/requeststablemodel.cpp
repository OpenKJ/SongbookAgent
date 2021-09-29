/*
 * Copyright (c) 2013-2021 Thomas Isaac Lightburn
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
#include <QPainter>
#include <QSvgRenderer>
#include <utility>


RequestsTableModel::RequestsTableModel(OKJSongbookAPI &sbApi, QObject *parent) :
        songbookApi(sbApi),
        QAbstractTableModel(parent) {
    connect(&songbookApi, &OKJSongbookAPI::requestsChanged, this, &RequestsTableModel::requestsChanged);
    connect(&songbookApi, &OKJSongbookAPI::alertReceived, this, &RequestsTableModel::alertReceived);
}

void RequestsTableModel::requestsChanged(const OkjsRequests& requests) {
    emit layoutAboutToBeChanged();
    m_requests.clear();
    for (const auto & request : requests) {
        int index = request.requestId;
        QString singer = request.singer;
        QString artist = request.artist;
        QString title = request.title;
        int reqtime = request.time;
        int key = request.key;
        m_requests << Request(index, singer, artist, title, reqtime, key);
    }
    emit layoutChanged();
}

int RequestsTableModel::rowCount(const QModelIndex &parent) const {
    return m_requests.size();
}

int RequestsTableModel::columnCount(const QModelIndex &parent) const {
    return 8;
}

QVariant RequestsTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_requests.size() || index.row() < 0)
        return QVariant();
    if (role == Qt::ToolTipRole) {
        switch (index.column()) {
            case DELETE:
                return QString("Delete request");
            case COPY:
                return QString("Copy artist and title to clipboard");
            case SEARCH:
                return QString("Search for song online in the OpenKJ Database");
        }
    }
    if (role == Qt::TextAlignmentRole)
        switch (index.column()) {
            case KEY:
                return Qt::AlignHCenter + Qt::AlignVCenter;
            case TIMESTAMP:
                return Qt::AlignRight + Qt::AlignVCenter;
            default:
                return Qt::AlignLeft + Qt::AlignVCenter;
        }
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
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

QVariant RequestsTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::SizeHintRole && orientation == Qt::Horizontal) {
        switch (section) {
            case COPY:
            case DELETE:
            case SEARCH:
                return QSize{m_curFontHeight * 2, m_curFontHeight};
            case KEY:
                return m_curFontMetrics.size(Qt::TextSingleLine, "_Key_");
            case TIMESTAMP:
                return m_curFontMetrics.size(Qt::TextSingleLine, "_18-99-99 99:99 AM");
            case SINGER:
                return m_curFontMetrics.size(Qt::TextSingleLine, "_Isaac Lightburn");
            default:
                return {};
        }
    }
    if (role == Qt::DisplayRole) {
        switch (section) {
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
            default:
                return {};
        }
    }
    return QVariant();
}

Qt::ItemFlags RequestsTableModel::flags(const QModelIndex &index) const {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int RequestsTableModel::count() {
    return m_requests.count();
}

void RequestsTableModel::fontChanged(const QFont &font) {
    m_curFont = font;
    m_curFontMetrics = QFontMetrics(font);
    m_curFontHeight = m_curFontMetrics.height();
}

int Request::key() const {
    return m_key;
}

void Request::setKey(int key) {
    m_key = key;
}

Request::Request(int RequestId, QString Singer, QString Artist, QString Title, int ts, int key) {
    m_requestId = RequestId;
    m_singer = std::move(Singer);
    m_artist = std::move(Artist);
    m_title = std::move(Title);
    m_timeStamp = ts;
    m_key = key;
}

int Request::requestId() const {
    return m_requestId;
}

void Request::setRequestId(int requestId) {
    m_requestId = requestId;
}

int Request::timeStamp() const {
    return m_timeStamp;
}

void Request::setTimeStamp(int timeStamp) {
    m_timeStamp = timeStamp;
}

QString Request::artist() const {
    return m_artist;
}

void Request::setArtist(const QString &artist) {
    m_artist = artist;
}

QString Request::title() const {
    return m_title;
}

void Request::setTitle(const QString &title) {
    m_title = title;
}

QString Request::singer() const {
    return m_singer;
}

void Request::setSinger(const QString &singer) {
    m_singer = singer;
}


void ItemDelegateRequests::resizeIconsForFont(const QFont &font) {
    if (m_curFontHeight == QFontMetrics(font).height())
        return;
    m_curFontHeight = QFontMetrics(font).height();
    m_iconDelete = QImage(m_curFontHeight, m_curFontHeight, QImage::Format_ARGB32);
    m_iconCopy = QImage(m_curFontHeight, m_curFontHeight, QImage::Format_ARGB32);
    m_iconWebSearch = QImage(m_curFontHeight, m_curFontHeight, QImage::Format_ARGB32);
    m_iconDelete.fill(Qt::transparent);
    m_iconCopy.fill(Qt::transparent);
    m_iconWebSearch.fill(Qt::transparent);
    QPainter painterDelete(&m_iconDelete);
    QPainter painterCopy(&m_iconCopy);
    QPainter painterWebSearch(&m_iconWebSearch);
    QSvgRenderer svgRendererDelete(QString(":/resources/delete.svg"));
    QSvgRenderer svgRendererCopy(QString(":/resources/copy.svg"));
    QSvgRenderer svgRendererWebSearch(QString(":/resources/web-search.svg"));
    svgRendererDelete.render(&painterDelete);
    svgRendererCopy.render(&painterCopy);
    svgRendererWebSearch.render(&painterWebSearch);
}

void
ItemDelegateRequests::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    int topPad = (option.rect.height() - m_curFontHeight) / 2;
    int leftPad = (option.rect.width() - m_curFontHeight) / 2;
    switch (index.column()) {
        case RequestsTableModel::DELETE:
            painter->drawImage(
                    QRect(option.rect.x() + leftPad, option.rect.y() + topPad, m_curFontHeight, m_curFontHeight),
                    m_iconDelete);
            return;
        case RequestsTableModel::COPY:
            painter->drawImage(
                    QRect(option.rect.x() + leftPad, option.rect.y() + topPad, m_curFontHeight, m_curFontHeight),
                    m_iconCopy);
            return;
        case RequestsTableModel::SEARCH:
            painter->drawImage(
                    QRect(option.rect.x() + leftPad, option.rect.y() + topPad, m_curFontHeight, m_curFontHeight),
                    m_iconWebSearch);
        default:
            QItemDelegate::paint(painter, option, index);
    }
}

ItemDelegateRequests::ItemDelegateRequests(QObject *parent) : QItemDelegate(parent) {
    resizeIconsForFont(m_settings.font());
}
