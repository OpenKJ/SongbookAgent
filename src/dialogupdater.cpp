#include "dialogupdater.h"
#include "ui_dialogupdater.h"

#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QProcess>
#include <QStandardPaths>

DialogUpdater::DialogUpdater(QString curVersion, QString availVersion, QString branch, QString os, QString url, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogUpdater)
{
//    dlgProgress = new QProgressDialog(this);
//    dlgProgress->setCancelButton(0);
//    dlgProgress->setLabelText("Downloading update");
//    dlgProgress->setWindowTitle("Downloading");
//    dlgProgress->hide();
    ui->setupUi(this);
    this->url = url;
    this->os = os;
    if (os == "lin")
    {
        ui->pushButtonUpdate->hide();
        ui->labelUpdateMsg->show();
    }
    else
        ui->labelUpdateMsg->hide();
    ui->labelAvailVersion->setText(availVersion);
    ui->labelCurVersion->setText(curVersion);
    ui->labelBranch->setText(branch);
}

DialogUpdater::~DialogUpdater()
{
    delete ui;
}

void DialogUpdater::on_pushButtonUpdate_clicked()
{
    dlgProgress = new QProgressDialog(this);
    dlgProgress->setCancelButton(0);
    dlgProgress->setLabelText("Downloading update");
    dlgProgress->setWindowTitle("Downloading");
    QString destDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QUrl aUrl(url);
    QString destPath = destDir + "/" + aUrl.fileName();
    QNetworkAccessManager m_NetworkMngr;
    QNetworkReply *reply= m_NetworkMngr.get(QNetworkRequest(url));
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    dlgProgress->show();
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress(qint64,qint64)));
    loop.exec();
    QFile file(destPath);
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    delete reply;
    file.close();
    dlgProgress->hide();
    QMessageBox mBox(this);
    mBox.setWindowTitle("Updating");
    mBox.setText("The program will now exit and launch the installer");
    mBox.exec();
    if (os == "win32" || os == "win64")
    {
        QString setupFile = "\"" + destPath + "\"";
        qWarning() << "Running setup file: " << setupFile;
        QProcess::startDetached(setupFile);
//        process.setProgram(setupFile);
//        process.setWorkingDirectory(destDir);
//        process.setStandardOutputFile(QProcess::nullDevice());
//        process.setStandardErrorFile(QProcess::nullDevice());
//        qint64 pid;
//        process.start();
//        process.waitForFinished();
//        qWarning() << process.error() << " - " << process.errorString();
//        qWarning() << process.readAllStandardOutput();
//        qWarning() << process.readAllStandardError();
        QApplication::quit();
    }
    else if (os == "mac")
    {
        QStringList args;
        args << destPath;
        QProcess::startDetached("open", args);
        QApplication::quit();
    }

}

void DialogUpdater::onDownloadProgress(qint64 cur, qint64 tot)
{
    if (os == "lin")
        return;
    dlgProgress->setMaximum(tot);
    dlgProgress->setValue(cur);
}
