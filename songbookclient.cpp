#include "songbookclient.h"
#include "ui_songbookclient.h"
#include "dialogabout.h"

#include <QClipboard>
#include <QDesktopServices>
#include <QMessageBox>
#include <qevent.h>

SongbookClient::SongbookClient(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SongbookClient)
{
    ui->setupUi(this);
    sbApi = new OKJSongbookAPI(this);
    icon = new QSystemTrayIcon(this);
    icon->setIcon(QIcon(QPixmap(":/resources/AppIcon.png")));
    icon->show();
    dlgSettings = new DialogSettings(sbApi,this);
    dlgUpdate = new DialogUpdate(sbApi, this);
    dlgAbout = new DialogAbout(this);
    reqModel = new RequestsTableModel(sbApi, this);
    ui->tableView->setModel(reqModel);
    blinkTimer = new QTimer(this);
    blinkTimer->start(500);
    showAction = new QAction(tr("Show"), this);
    hideAction = new QAction(tr("Hide"), this);
    exitAction = new QAction(tr("Exit"), this);
    iconMenu = new QMenu(this);
    iconMenu->addAction(showAction);
    iconMenu->addAction(hideAction);
    iconMenu->addAction(exitAction);
#ifndef Q_OS_MACX
    icon->setContextMenu(iconMenu);
#endif
    connect(showAction, SIGNAL(triggered(bool)), this, SLOT(show()));
    connect(hideAction, SIGNAL(triggered(bool)), this, SLOT(hide()));
    connect(exitAction, SIGNAL(triggered(bool)), this, SLOT(closeProgram()));
    connect(ui->actionE_xit, SIGNAL(triggered(bool)), this, SLOT(closeProgram()));
    connect(ui->actionSettings, SIGNAL(triggered(bool)), dlgSettings, SLOT(show()));
    connect(ui->actionUpdate_Songbook, SIGNAL(triggered(bool)), dlgUpdate, SLOT(show()));
    connect(ui->actionAbout, SIGNAL(triggered()), dlgAbout, SLOT(show()));
    connect(ui->actionDocumentation, SIGNAL(triggered()), this, SLOT(launchDocs()));
    connect(icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    connect(sbApi, SIGNAL(venuesChanged(OkjsVenues)), this, SLOT(venuesChanged(OkjsVenues)));
    connect(sbApi, SIGNAL(requestsChanged(OkjsRequests)), this, SLOT(requestsChanged(OkjsRequests)));
    connect(sbApi, SIGNAL(synchronized(QTime)), this, SLOT(synchronized(QTime)));
    connect(blinkTimer, SIGNAL(timeout()), this, SLOT(blinkTimerTimeout()));
    connect(sbApi, SIGNAL(alertReceived(QString, QString)), this, SLOT(showAlert(QString, QString)));
    oneShot = new QTimer(this);
    oneShot->setSingleShot(true);
    connect(oneShot, SIGNAL(timeout()), this, SLOT(autoSizeCols()));
    oneShot->start(250);

}

SongbookClient::~SongbookClient()
{
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

void SongbookClient::on_btnHide_clicked()
{
    this->hide();
}

void SongbookClient::closeProgram()
{
    icon->hide();
    this->close();
}

void SongbookClient::on_btnRefresh_clicked()
{
    sbApi->refreshRequests();
}

void SongbookClient::requestsChanged(OkjsRequests requests)
{
    static int lastCount = 0;
    if (lastCount == 0)
    {
        oneShot->start(150);
    }
    if (settings.popup())
        show();
    lastCount = requests.size();
}

void SongbookClient::venuesChanged(OkjsVenues venues)
{
    int venue = settings.lastVenue();
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
    settings.setLastVenue(ui->cbxVenue->itemData(selItem).toInt());
    ui->checkBoxAccepting->setChecked(sbApi->getAccepting());
}

void SongbookClient::autoSizeCols()
{
    int fH = QFontMetrics(settings.font()).height();
    int tsColSize = QFontMetrics(settings.font()).width(" 00/00/00 00:00 pm ");
    int keySize = QFontMetrics(settings.font()).width("_Key_");
    int iconWidth = fH + fH;
    int remainingSpace = ui->tableView->width() - tsColSize - keySize - (iconWidth * 3);
    int singerColSize = (remainingSpace / 3) - 120;
    int artistColSize = (remainingSpace / 3);
    int titleColSize = (remainingSpace / 3) + 115;
    ui->tableView->horizontalHeader()->resizeSection(0, singerColSize);
    ui->tableView->horizontalHeader()->resizeSection(1, artistColSize);
    ui->tableView->horizontalHeader()->resizeSection(2, titleColSize);
    ui->tableView->horizontalHeader()->resizeSection(3, iconWidth);
    ui->tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->resizeSection(4, iconWidth);
    ui->tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);

    ui->tableView->horizontalHeader()->resizeSection(5, tsColSize);
    ui->tableView->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->resizeSection(6, keySize);
    ui->tableView->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->resizeSection(7, iconWidth);
    ui->tableView->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Fixed);
}

void SongbookClient::synchronized(QTime updateTime)
{
    ui->labelLastUpdate->setText(updateTime.toString("hh:mm:ss AP"));
}


void SongbookClient::closeEvent(QCloseEvent *event)
{
#ifndef Q_OS_MACX
    if (icon->isVisible()) {
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
}

void SongbookClient::on_tableView_clicked(const QModelIndex &index)
{
    if (index.column() == RequestsTableModel::DELETE)
    {
        sbApi->removeRequest(index.data(Qt::UserRole).toInt());
    }
    if (index.column() == RequestsTableModel::COPY)
    {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(reqModel->requests().at(index.row()).artist() + " " + reqModel->requests().at(index.row()).title());
    }
    if (index.column() == RequestsTableModel::SEARCH)
    {
        QDesktopServices::openUrl(QUrl("http://db.openkj.org/?type=All&searchstr=" + QUrl::toPercentEncoding(reqModel->requests().at(index.row()).artist() + " " + reqModel->requests().at(index.row()).title())));
    }
}

void SongbookClient::on_btnClear_clicked()
{
    sbApi->clearRequests();
}

void SongbookClient::on_cbxVenue_currentIndexChanged(int index)
{
    int venue = ui->cbxVenue->itemData(index).toInt();
    settings.setLastVenue(venue);
    sbApi->refreshRequests();
    ui->checkBoxAccepting->setChecked(sbApi->getAccepting());
    qWarning() << "Set venue_id to " << venue;
    qWarning() << "Settings now reporting venue as " << settings.lastVenue();
}

void SongbookClient::on_checkBoxAccepting_clicked(bool checked)
{
    sbApi->setAccepting(checked);
}

void SongbookClient::blinkTimerTimeout()
{
    static bool blinked = false;
    if ((sbApi->numRequests() > 0) && !blinked)
    {
        icon->setIcon(QIcon(QPixmap(":/resources/AppIcon-blink.png")));
        blinked = true;
    }
    else if ((sbApi->numRequests() > 0) && blinked)
    {
        icon->setIcon(QIcon(QPixmap(":/resources/AppIcon.png")));
        blinked = false;
    }
    else if ((sbApi->numRequests() == 0) && blinked)
    {
        icon->setIcon(QIcon(QPixmap(":/resources/AppIcon.png")));
        blinked = false;
    }
}

void SongbookClient::showAlert(QString title, QString message)
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
