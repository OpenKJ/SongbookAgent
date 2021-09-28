#include "settings.h"
#include <QApplication>

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
