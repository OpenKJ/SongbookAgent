#ifndef DIALOGUPDATE_H
#define DIALOGUPDATE_H

#include <memory>
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
    explicit DialogUpdate(OKJSongbookAPI &sbApi, QWidget *parent = nullptr);
    ~DialogUpdate() override;

private slots:
    void btnBrowseClicked();
    void btnLoadCsvClicked();
    void btnSendClicked();
    void btnScanFilesClicked();
    void btnBrowseDirsClicked();
    void saveState();


private:
    std::unique_ptr<Ui::DialogUpdate> ui;
    QStringList headerLabels;
    static QStringList parseCsvString(const QString& string);
    OKJSongbookAPI &sbApi;
    OkjsSongs songs;
    static QStringList findKaraokeFiles(const QString& directory);
    Settings settings;
};

#endif // DIALOGUPDATE_H
