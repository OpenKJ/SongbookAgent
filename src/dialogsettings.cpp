#include "dialogsettings.h"
#include "ui_dialogsettings.h"
#include <QFontDialog>
#include <QMessageBox>

DialogSettings::DialogSettings(OKJSongbookAPI *sbapi, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings)
{
    sbApi = sbapi;
    ui->setupUi(this);
    loadFromSettings();
    connect(ui->btnTest, SIGNAL(clicked(bool)), this, SLOT(testApiKey()));
    connect(sbapi, SIGNAL(entitledSystemCountChanged(int)), this, (SLOT(entitledSystemCountChanged(int))));

#ifdef Q_OS_MACX
    // Disable/hide this setting since it doesn't work on Mac anyway
    ui->checkBoxPopup->hide();
#endif
}

DialogSettings::~DialogSettings()
{
    delete ui;
}

void DialogSettings::loadFromSettings()
{
    font = settings.font();
    QString fontStr = font.family() + " " + QString::number(font.pointSize()) + "pt";
    if (font.bold())
        fontStr += " Bold";
    if (font.italic())
        fontStr += " Italic";
    ui->labelFont->setText(fontStr);
    ui->lineEditApiKey->setText(settings.apiKey());
    ui->checkBoxPopup->setChecked(settings.popup());
}

void DialogSettings::on_btnFont_clicked()
{
    bool ok;
    QFont selFont = QFontDialog::getFont(&ok, settings.font(), this, "Select application font");
    if (ok)
    {
        font = selFont;
        QString fontStr = font.family() + " " + QString::number(font.pointSize()) + "pt";
        if (font.bold())
            fontStr += " Bold";
        if (font.italic())
            fontStr += " Italic";
        ui->labelFont->setText(fontStr);
    }
}

void DialogSettings::on_buttonBox_accepted()
{
    settings.setApiKey(ui->lineEditApiKey->text());
    settings.setPopup(ui->checkBoxPopup->isChecked());
    settings.setFont(font);
    hide();
}

void DialogSettings::on_buttonBox_rejected()
{
    loadFromSettings();
    hide();
}

void DialogSettings::testApiKey()
{
    bool result = sbApi->testApiKey(ui->lineEditApiKey->text());
    qWarning() << "Test returned: " << result;
    if (result)
    {
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setText("The test was successful!");
        msgBox->exec();
        delete msgBox;
    }
    else
    {
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setText("Test failed!  Incorrect API key or connection error");
        msgBox->exec();
        delete msgBox;
    }
}

void DialogSettings::entitledSystemCountChanged(int count)
{
    ui->spinBoxSystemId->setMaximum(count);
    if (settings.systemId() <= count)
        ui->spinBoxSystemId->setValue(settings.systemId());
}

void DialogSettings::on_spinBoxSystemId_valueChanged(int arg1)
{
    settings.setSystemId(arg1);
}
