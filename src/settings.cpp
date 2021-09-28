#include "settings.h"
#include <QApplication>
#include <QHeaderView>

Settings::Settings(QObject *parent) : QObject(parent)
{
    settings = new QSettings();
}

QString Settings::apiKey()
{
    return settings->value("apiKey", QString()).toString();
}

bool Settings::popup()
{
    return settings->value("popup", false).toBool();
}

QFont Settings::font()
{
    QFont font;
    font.fromString(settings->value("font", QApplication::font().toString()).toString());
    return font;
}

int Settings::lastVenue()
{
    return settings->value("lastVenue", -1).toInt();
}

importerConfig Settings::fileImporterConfig()
{
    importerConfig config;
    config.artistCol = settings->value("fimportArtistCol", 1).toInt();
    config.titleCol  = settings->value("fimportTitleCol", 2).toInt();
    config.separator = settings->value("fimportSeparator", " - ").toString();
    config.path      = settings->value("fimportPath", QString()).toString();
    config.convertUnderscore = settings->value("fimportConvertUnderscore", false).toBool();
    return config;
}

importerConfig Settings::csvImporterConfig()
{
    importerConfig config;
    config.artistCol = settings->value("cimportArtistCol", 1).toInt();
    config.titleCol  = settings->value("cimportTitleCol", 2).toInt();
    config.separator = settings->value("cimportSeparator", " - ").toString();
    config.path      = settings->value("cimportPath", QString()).toString();
    config.convertUnderscore = settings->value("cimportConvertUnderscore", false).toBool();
    return config;
}

int Settings::systemId()
{
    return settings->value("systemId", 1).toInt();
}

void Settings::setApiKey(QString key)
{
    settings->setValue("apiKey", key);
    emit apiKeyChanged(key);
}

void Settings::setPopup(bool popup)
{
    settings->setValue("popup", popup);
}

void Settings::setFont(QFont font)
{
    settings->setValue("font", font.toString());
    QApplication::setFont(font, "QWidget");
}

void Settings::setLastVenue(int venueId)
{
    settings->setValue("lastVenue", venueId);
}

void Settings::saveFileImporterConfig(importerConfig config)
{
    settings->setValue("fimportArtistCol", config.artistCol);
    settings->setValue("fimportTitleCol", config.titleCol);
    settings->setValue("fimportSeparator", config.separator);
    settings->setValue("fimportPath", config.path);
    settings->setValue("fimportConvertUnderscore", config.convertUnderscore);
}

void Settings::saveCsvImporterConfig(importerConfig config)
{
    settings->setValue("cimportArtistCol", config.artistCol);
    settings->setValue("cimportTitleCol", config.titleCol);
    settings->setValue("cimportSeparator", config.separator);
    settings->setValue("cimportPath", config.path);
    settings->setValue("cimportConvertUnderscore", config.convertUnderscore);
}

void Settings::setSystemId(int id)
{
    return settings->setValue("systemId", id);
}

void Settings::saveWindowState(QWidget *window)
{
    if (!window->isVisible())
        return;
    settings->beginGroup(window->objectName());
    settings->setValue("geometry", window->saveGeometry());
    settings->endGroup();
}

void Settings::restoreWindowState(QWidget *window)
{
    settings->beginGroup(window->objectName());
    if (settings->contains("geometry"))
    {
        window->restoreGeometry(settings->value("geometry").toByteArray());
    }
    else if (settings->contains("size") && settings->contains("pos"))
    {
        window->resize(settings->value("size", QSize(640, 480)).toSize());
        window->move(settings->value("pos", QPoint(100, 100)).toPoint());
    }
    settings->endGroup();
}

void Settings::saveColumnWidths(QTableView *tableView)
{
    settings->beginGroup(tableView->objectName());
    for (int i=0; i < tableView->horizontalHeader()->count(); i++)
    {
        settings->beginGroup(QString::number(i));
        settings->setValue("size", tableView->horizontalHeader()->sectionSize(i));
        settings->setValue("hidden", tableView->horizontalHeader()->isSectionHidden(i));
        settings->endGroup();
    }
    settings->endGroup();
}

bool Settings::restoreColumnWidths(QTableView *tableView)
{
    if (!settings->childGroups().contains(tableView->objectName()))
        return false;
    settings->beginGroup(tableView->objectName());
    QStringList headers = settings->childGroups();
    for (int i=0; i < headers.size(); i++)
    {
        settings->beginGroup(headers.at(i));
        int section = headers.at(i).toInt();
        bool hidden = settings->value("hidden", false).toBool();
        int size = settings->value("size", 0).toInt();
        tableView->horizontalHeader()->resizeSection(section, size);
        tableView->horizontalHeader()->setSectionHidden(section, hidden);
        settings->endGroup();
    }
    settings->endGroup();
    return true;
}