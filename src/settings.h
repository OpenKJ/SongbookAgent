#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QFont>

class importerConfig
{
public:
    int artistCol;
    int titleCol;
    bool convertUnderscore;
    QString path;
    QString separator;
};

class Settings : public QObject
{
    Q_OBJECT
private:
    QSettings *settings;
public:
    explicit Settings(QObject *parent = nullptr);
    QString apiKey();
    bool popup();
    QFont font();
    int lastVenue();
    importerConfig fileImporterConfig();
    importerConfig csvImporterConfig();
    int systemId();

signals:
    void apiKeyChanged(QString);

public slots:
    void setApiKey(QString key);
    void setPopup(bool popup);
    void setFont(QFont font);
    void setLastVenue(int venueId);
    void saveFileImporterConfig(importerConfig config);
    void saveCsvImporterConfig(importerConfig config);
    void setSystemId(int id);

};

#endif // SETTINGS_H
