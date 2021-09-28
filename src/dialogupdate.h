#ifndef DIALOGUPDATE_H
#define DIALOGUPDATE_H

#include <QDialog>
#include "okjsongbookapi.h"
#include "settings.h"

namespace Ui {
class DialogUpdate;
}

class DialogUpdate : public QDialog
{
    Q_OBJECT

public:
    explicit DialogUpdate(OKJSongbookAPI *sbapi, QWidget *parent = 0);
    ~DialogUpdate();

private slots:
    void on_btnBrowse_clicked();
    void on_btnLoadCsv_clicked();
    void on_btnSend_clicked();
    void on_buttonBox_rejected();
    void on_btnScanFiles_clicked();

    void on_btnBrowseDirs_clicked();
    void saveState();


private:
    Ui::DialogUpdate *ui;
    QStringList headerLabels;
    QStringList parseCsvString(QString string);
    OKJSongbookAPI *sbApi;
    OkjsSongs songs;
    QStringList findKaroakeFiles(QString directory);
    Settings settings;
};

#endif // DIALOGUPDATE_H
