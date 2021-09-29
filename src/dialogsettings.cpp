#include "dialogsettings.h"
#include "ui_dialogsettings.h"
#include <QFontDialog>
#include <QMessageBox>

DialogSettings::DialogSettings(OKJSongbookAPI &sbApi, QWidget *parent) :
        m_sbApi(sbApi), QDialog(parent), m_ui(new Ui::DialogSettings) {
    m_ui->setupUi(this);
    loadFromSettings();
    connect(m_ui->btnTest, SIGNAL(clicked(bool)), this, SLOT(testApiKey()));
    connect(&sbApi, &OKJSongbookAPI::entitledSystemCountChanged, this, &DialogSettings::entitledSystemCountChanged);
    connect(m_ui->btnFont, &QPushButton::clicked, this, &DialogSettings::btnFontClicked);
    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &DialogSettings::buttonBoxAccepted);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &DialogSettings::buttonBoxRejected);
    connect(m_ui->spinBoxSystemId, qOverload<int>(&QSpinBox::valueChanged), &m_settings, &Settings::setSystemId);
    m_sbApi.getEntitledSystemCount();
#ifdef Q_OS_MACX
    // Disable/hide this setting since it doesn't work on Mac anyway
    ui->checkBoxPopup->hide();
#endif
}

void DialogSettings::loadFromSettings() {
    m_font = m_settings.font();
    QString fontStr = m_font.family() + " " + QString::number(m_font.pointSize()) + "pt";
    if (m_font.bold())
        fontStr += " Bold";
    if (m_font.italic())
        fontStr += " Italic";
    m_ui->labelFont->setText(fontStr);
    m_ui->lineEditApiKey->setText(m_settings.apiKey());
    m_ui->checkBoxPopup->setChecked(m_settings.popup());
}

void DialogSettings::btnFontClicked() {
    bool ok;
    QFont selFont = QFontDialog::getFont(&ok, m_settings.font(), this, "Select application font");
    if (ok) {
        m_font = selFont;
        QString fontStr = m_font.family() + " " + QString::number(m_font.pointSize()) + "pt";
        if (m_font.bold())
            fontStr += " Bold";
        if (m_font.italic())
            fontStr += " Italic";
        m_ui->labelFont->setText(fontStr);
    }
}

void DialogSettings::buttonBoxAccepted() {
    m_settings.setApiKey(m_ui->lineEditApiKey->text());
    m_settings.setPopup(m_ui->checkBoxPopup->isChecked());
    m_settings.setFont(m_font);
    m_sbApi.getEntitledSystemCount();
    emit fontChanged(m_font);
    hide();
}

void DialogSettings::buttonBoxRejected() {
    loadFromSettings();
    hide();
}

void DialogSettings::testApiKey() {
    bool result = m_sbApi.testApiKey(m_ui->lineEditApiKey->text());
    qWarning() << "Test returned: " << result;
    if (result) {
        auto *msgBox = new QMessageBox(this);
        msgBox->setText("The test was successful!");
        msgBox->exec();
        m_settings.setApiKey(m_ui->lineEditApiKey->text());
        m_sbApi.getEntitledSystemCount();
        delete msgBox;
    } else {
        auto *msgBox = new QMessageBox(this);
        msgBox->setText("Test failed!  Incorrect API key or connection error");
        msgBox->exec();
        delete msgBox;
    }
}

void DialogSettings::entitledSystemCountChanged(int count) {
    m_ui->spinBoxSystemId->setMaximum(count);
    if (m_settings.systemId() <= count)
        m_ui->spinBoxSystemId->setValue(m_settings.systemId());
}

DialogSettings::~DialogSettings() = default;
