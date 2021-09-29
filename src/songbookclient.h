#ifndef SONGBOOKCLIENT_H
#define SONGBOOKCLIENT_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QTimer>
#include "dialogsettings.h"
#include "requeststablemodel.h"
#include "okjsongbookapi.h"
#include "settings.h"
#include "dialogupdate.h"
#include "dialogabout.h"
#include <QMenu>
#include <memory>

namespace Ui {
class SongbookClient;
}

class SongbookClient : public QMainWindow
{
    Q_OBJECT

public:
    explicit SongbookClient(QWidget *parent = nullptr);
    ~SongbookClient() override;

private:
    std::unique_ptr<Ui::SongbookClient> ui;
    QSystemTrayIcon m_icon{QIcon(QPixmap(":/resources/org.openkj.SongbookAgent.svg"))};
    OKJSongbookAPI m_sbApi{this};
    RequestsTableModel m_reqModel{m_sbApi, this};
    ItemDelegateRequests m_reqDelegate{this};
    Settings m_settings;
    QTimer m_blinkTimer;
    QMenu m_iconMenu{this};
    QAction m_showAction{"Show"};
    QAction m_hideAction{"Hide"};
    QAction m_exitAction{"Exit"};
    bool m_blinked{false};

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void closeProgram();
    void newRequestsReceived();
    void venuesChanged(const OkjsVenues& venues);
    void autoSizeCols();
    void synchronized(QTime updateTime);
    void requestClicked(const QModelIndex &index);
    void venueIndexChanged(int index);
    void blinkTimerTimeout();
    void showAlert(const QString& title, const QString& message);
    static void launchDocs();
    void newVersionAvailable(const QString &curVersion, const QString &availVersion, const QString &branch, const QString &os, const QString &url);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

};

#endif // SONGBOOKCLIENT_H
