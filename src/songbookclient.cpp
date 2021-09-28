#include "songbookclient.h"
#include "ui_songbookclient.h"
#include "dialogabout.h"

#include <QClipboard>
#include <QDesktopServices>
#include <QMessageBox>
#include <qevent.h>
#include "dialogupdater.h"

SongbookClient::SongbookClient(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SongbookClient)
{
    ui->setupUi(this);
    ui->btnRefresh->setIcon(QIcon(QPixmap(":/resources/refresh.svg")));
    m_sbApi.versionCheck();
    m_icon.show();
    m_dlgSettings = new DialogSettings(&m_sbApi, this);
    m_dlgUpdate = new DialogUpdate(&m_sbApi, this);
    m_dlgAbout = new DialogAbout(this);
    ui->tableView->setItemDelegate(&m_reqDelegate);
    ui->tableView->setModel(&m_reqModel);
    m_blinkTimer.start(500);
    m_iconMenu.addAction(&m_showAction);
    m_iconMenu.addAction(&m_hideAction);
    m_iconMenu.addAction(&m_exitAction);
#ifndef Q_OS_MACX
    m_icon.setContextMenu(&m_iconMenu);
#endif
#ifdef Q_OS_WIN
    QWindowsWindowFunctions::setWindowActivationBehavior(QWindowsWindowFunctions::AlwaysActivateWindow);
#endif
    connect(&m_showAction, &QAction::triggered, this, &SongbookClient::show);
    connect(&m_hideAction, &QAction::triggered, this, &SongbookClient::hide);
    connect(&m_exitAction, &QAction::triggered, this, &SongbookClient::closeProgram);
    connect(ui->actionE_xit, &QAction::triggered, this, &SongbookClient::closeProgram);
    connect(ui->actionSettings, &QAction::triggered, m_dlgSettings, &DialogSettings::show);
    connect(ui->actionUpdate_Songbook, &QAction::triggered, m_dlgUpdate, &DialogUpdate::show);
    connect(ui->actionAbout, &QAction::triggered, m_dlgAbout, &DialogAbout::show);
    connect(ui->actionDocumentation, &QAction::triggered, this, &SongbookClient::launchDocs);
    connect(ui->btnHide, &QPushButton::clicked, this, &SongbookClient::hide);
    connect(ui->btnClear, &QPushButton::clicked, &m_sbApi, &OKJSongbookAPI::clearRequests);
    connect(ui->btnRefresh, &QPushButton::clicked, &m_sbApi, &OKJSongbookAPI::refreshRequests);
    connect(ui->tableView, &QTableView::clicked, this, &SongbookClient::requestClicked);
    connect(ui->cbxVenue, qOverload<int>(&QComboBox::currentIndexChanged), this, &SongbookClient::venueIndexChanged);
    connect(ui->checkBoxAccepting, &QCheckBox::clicked, &m_sbApi, &OKJSongbookAPI::setAccepting);
    connect(&m_icon, &QSystemTrayIcon::activated, this, &SongbookClient::iconActivated);
    connect(&m_sbApi, &OKJSongbookAPI::venuesChanged, this, &SongbookClient::venuesChanged);
    connect(&m_sbApi, &OKJSongbookAPI::synchronized, this, &SongbookClient::synchronized);
    connect(&m_sbApi, &OKJSongbookAPI::alertReceived, this, &SongbookClient::showAlert);
    connect(&m_sbApi, &OKJSongbookAPI::newVersionAvailable, this, &SongbookClient::newVersionAvailable);
    connect(&m_sbApi, &OKJSongbookAPI::newRequestsReceived, this, &SongbookClient::newRequestsReceived);
    connect(&m_blinkTimer, &QTimer::timeout, this, &SongbookClient::blinkTimerTimeout);
    connect(m_dlgSettings, &DialogSettings::fontChanged, &m_reqDelegate, &ItemDelegateRequests::resizeIconsForFont);
    connect(m_dlgSettings, &DialogSettings::fontChanged, &m_reqModel, &RequestsTableModel::fontChanged);

    QApplication::processEvents();
    QTimer::singleShot(250, [&] () {
        // We delay some stuff to give the UI time to draw and be displayed
        QApplication::setFont(m_settings.font());
        m_settings.restoreWindowState(m_dlgAbout);
        m_settings.restoreWindowState(m_dlgSettings);
        m_settings.restoreWindowState(m_dlgUpdate);
        m_settings.restoreWindowState(this);
        if (!m_settings.restoreColumnWidths(ui->tableView))
            autoSizeCols();
        connect(ui->tableView->horizontalHeader(), &QHeaderView::geometriesChanged, [&] () {
            m_settings.saveColumnWidths(ui->tableView);
        });
    });
}

SongbookClient::~SongbookClient()
{
    m_settings.saveWindowState(m_dlgAbout);
    m_settings.saveWindowState(m_dlgSettings);
    m_settings.saveWindowState(m_dlgUpdate);
    m_settings.saveWindowState(this);
    m_dlgAbout->deleteLater();
    m_dlgSettings->deleteLater();
    m_dlgUpdate->deleteLater();
    delete ui;
}

void SongbookClient::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
#ifndef Q_OS_MACX
    qWarning() << "iconActivated fired";
    if (reason != QSystemTrayIcon::Context)
        this->setVisible(!this->isVisible());
  //  if (reason == QSystemTrayIcon::Context)
#endif
}

void SongbookClient::closeProgram()
{
    m_icon.hide();
    this->close();
}

void SongbookClient::venuesChanged(const OkjsVenues& venues)
{
    int venue = m_settings.lastVenue();
    ui->cbxVenue->clear();
    int selItem = 0;
    for (int i=0; i < venues.size(); i++)
    {
        ui->cbxVenue->addItem(venues.at(i).name, venues.at(i).venueId);
        if (venues.at(i).venueId == venue)
        {
            selItem = i;
        }
    }
    ui->cbxVenue->setCurrentIndex(selItem);
    m_settings.setLastVenue(ui->cbxVenue->itemData(selItem).toInt());
    ui->checkBoxAccepting->setChecked(m_sbApi.getAccepting());
}

void SongbookClient::autoSizeCols()
{
    ui->tableView->horizontalHeader()->setSectionResizeMode(RequestsTableModel::SINGER, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(RequestsTableModel::KEY, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(RequestsTableModel::TIMESTAMP, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(RequestsTableModel::COPY, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(RequestsTableModel::DELETE, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(RequestsTableModel::SEARCH, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(RequestsTableModel::ARTIST, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(RequestsTableModel::TITLE, QHeaderView::Stretch);
    QApplication::processEvents();
    ui->tableView->horizontalHeader()->setSectionResizeMode(RequestsTableModel::SINGER, QHeaderView::Interactive);
    ui->tableView->horizontalHeader()->setSectionResizeMode(RequestsTableModel::KEY, QHeaderView::Interactive);
    ui->tableView->horizontalHeader()->setSectionResizeMode(RequestsTableModel::TIMESTAMP, QHeaderView::Interactive);
    ui->tableView->horizontalHeader()->setSectionResizeMode(RequestsTableModel::COPY, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(RequestsTableModel::DELETE, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(RequestsTableModel::SEARCH, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(RequestsTableModel::ARTIST, QHeaderView::Interactive);
    ui->tableView->horizontalHeader()->setSectionResizeMode(RequestsTableModel::TITLE, QHeaderView::Interactive);
}

void SongbookClient::synchronized(QTime updateTime)
{
    ui->labelLastUpdate->setText(updateTime.toString("hh:mm:ss AP"));
}


void SongbookClient::closeEvent(QCloseEvent *event)
{
#ifndef Q_OS_MACX
    if (m_icon.isVisible()) {
        hide();
        event->ignore();
    }
#else
    closeProgram();
#endif

}


void SongbookClient::resizeEvent(QResizeEvent *event)
{
    autoSizeCols();
    QMainWindow::resizeEvent(event);
    m_settings.saveWindowState(this);
}

void SongbookClient::requestClicked(const QModelIndex &index)
{
    if (index.column() == RequestsTableModel::DELETE)
    {
        m_sbApi.removeRequest(index.data(Qt::UserRole).toInt());
    }
    if (index.column() == RequestsTableModel::COPY)
    {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(m_reqModel.requests().at(index.row()).artist() + " " + m_reqModel.requests().at(index.row()).title());
    }
    if (index.column() == RequestsTableModel::SEARCH)
    {
        QDesktopServices::openUrl(QUrl("https://db.openkj.org/?type=All&searchstr=" + QUrl::toPercentEncoding(m_reqModel.requests().at(index.row()).artist() + " " + m_reqModel.requests().at(index.row()).title())));
    }
}

void SongbookClient::venueIndexChanged(int index)
{
    int venue = ui->cbxVenue->itemData(index).toInt();
    m_settings.setLastVenue(venue);
    m_sbApi.refreshRequests();
    ui->checkBoxAccepting->setChecked(m_sbApi.getAccepting());
    qWarning() << "Set venue_id to " << venue;
    qWarning() << "Settings now reporting venue as " << m_settings.lastVenue();
}

void SongbookClient::blinkTimerTimeout()
{
    if ((m_sbApi.numRequests() > 0) && !m_blinked)
    {
        m_icon.setIcon(QIcon(QPixmap(":/resources/org.openkj.SongbookAgent-blink.svg")));
        m_blinked = true;
    }
    else if (m_blinked)
    {
        m_icon.setIcon(QIcon(QPixmap(":/resources/org.openkj.SongbookAgent.svg")));
        m_blinked = false;
    }
}

void SongbookClient::showAlert(const QString& title, const QString& message)
{
    show();
    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
   // msgBox.setInformativeText(file);
    msgBox.exec();
}

void SongbookClient::launchDocs()
{
    QDesktopServices::openUrl(QUrl("https://docs.okjsongbook.com"));
}

void SongbookClient::newVersionAvailable(const QString &curVersion, const QString &availVersion, const QString &branch, const QString &os, const QString &url)
{
    DialogUpdater dlgUpdater(curVersion,availVersion,branch,os,url,this);
    dlgUpdater.exec();

}

void SongbookClient::newRequestsReceived() {
    if (m_settings.popup()) {
        show();
        raise();
        activateWindow();
    }
}
