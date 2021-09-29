#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QFont>
#include <QWidget>
#include <QTableView>

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
    void setApiKey(const QString& key);
    void setPopup(bool popup);
    void setFont(const QFont& font);
    void setLastVenue(int venueId);
    void saveFileImporterConfig(const importerConfig& config);
    void saveCsvImporterConfig(const importerConfig& config);
    void setSystemId(int id);
    bool restoreColumnWidths(QTableView *tableView);
    void saveColumnWidths(QTableView *tableView);

    void restoreWindowState(QWidget *window);
    void saveWindowState(QWidget *window);
};

#endif // SETTINGS_H
