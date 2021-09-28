#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>
#include "settings.h"
#include "okjsongbookapi.h"

namespace Ui {
class DialogSettings;
}

class DialogSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSettings(OKJSongbookAPI *sbapi, QWidget *parent = 0);
    ~DialogSettings();

private slots:
    void on_buttonBox_accepted();
    void on_btnFont_clicked();
    void on_buttonBox_rejected();
    void testApiKey();
    void entitledSystemCountChanged(int count);


    void on_spinBoxSystemId_valueChanged(int arg1);

private:
    Ui::DialogSettings *ui;
    Settings settings;
    QFont font;
    void loadFromSettings();
    OKJSongbookAPI *sbApi;

};

#endif // DIALOGSETTINGS_H
