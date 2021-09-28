#ifndef DIALOGUPDATER_H
#define DIALOGUPDATER_H

#include <QDialog>
#include <QProgressDialog>


namespace Ui {
class DialogUpdater;
}

class DialogUpdater : public QDialog
{
    Q_OBJECT

public:
    explicit DialogUpdater(QString curVersion, QString availVersion, QString branch, QString os, QString url, QWidget *parent = nullptr);
    ~DialogUpdater();

private slots:
    void on_pushButtonUpdate_clicked();
    void onDownloadProgress(qint64 cur, qint64 tot);

private:
    Ui::DialogUpdater *ui;
    QString url;
    QString os;
    QProgressDialog *dlgProgress;
};

#endif // DIALOGUPDATER_H
