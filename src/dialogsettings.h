#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>
#include <memory>
#include "settings.h"
#include "okjsongbookapi.h"

namespace Ui {
    class DialogSettings;
}

class DialogSettings : public QDialog {
Q_OBJECT

public:
    explicit DialogSettings(OKJSongbookAPI &sbApi, QWidget *parent = nullptr);
    ~DialogSettings() override;

private:
    std::unique_ptr<Ui::DialogSettings> m_ui;
    Settings m_settings;
    QFont m_font;
    OKJSongbookAPI &m_sbApi;
    void loadFromSettings();

signals:
    void fontChanged(const QFont &font);

private slots:
    void buttonBoxAccepted();
    void btnFontClicked();
    void buttonBoxRejected();
    void testApiKey();
    void entitledSystemCountChanged(int count);
};

#endif // DIALOGSETTINGS_H
