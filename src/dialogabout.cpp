#include "dialogabout.h"
#include "ui_dialogabout.h"
#include "version.h"

DialogAbout::DialogAbout(QWidget *parent) :
        QDialog(parent),
        m_ui(new Ui::DialogAbout) {
    m_ui->setupUi(this);
    m_ui->labelVersion->setText("Version: " + QString(VERSION_STRING));
}

DialogAbout::~DialogAbout() = default;

