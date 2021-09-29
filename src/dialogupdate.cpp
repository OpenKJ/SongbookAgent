#include "dialogupdate.h"
#include "ui_dialogupdate.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QProgressDialog>
#include <QDirIterator>

DialogUpdate::DialogUpdate(OKJSongbookAPI &sbApi, QWidget *parent) :
    sbApi(sbApi),
    QDialog(parent),
    ui(new Ui::DialogUpdate)
{
    ui->setupUi(this);
    headerLabels.append("Artist");
    headerLabels.append("Title");
    ui->tableWidgetPreview->setHorizontalHeaderLabels(headerLabels);

    ui->spinBoxColArtist->setValue(settings.csvImporterConfig().artistCol);
    ui->spinBoxColTitle->setValue(settings.csvImporterConfig().titleCol);
    ui->spinBoxSecArtist->setValue(settings.fileImporterConfig().artistCol);
    ui->spinBoxSecTitle->setValue(settings.fileImporterConfig().titleCol);
    ui->lineEditSepFile->setText(settings.fileImporterConfig().separator);
    ui->lineEditDirPath->setText(settings.fileImporterConfig().path);
    ui->lineEditCsvPath->setText(settings.csvImporterConfig().path);
    ui->checkBoxConvertUnderscore->setChecked(settings.csvImporterConfig().convertUnderscore);
    ui->checkBoxConvertUnderscoreFiles->setChecked(settings.fileImporterConfig().convertUnderscore);

    connect(ui->spinBoxColArtist, SIGNAL(valueChanged(int)), this, SLOT(saveState()));
    connect(ui->spinBoxColTitle, SIGNAL(valueChanged(int)), this, SLOT(saveState()));
    connect(ui->spinBoxSecArtist, SIGNAL(valueChanged(int)), this, SLOT(saveState()));
    connect(ui->spinBoxSecTitle, SIGNAL(valueChanged(int)), this, SLOT(saveState()));
    connect(ui->lineEditSepFile, SIGNAL(textChanged(QString)), this, SLOT(saveState()));
    connect(ui->lineEditCsvPath, SIGNAL(textChanged(QString)), this, SLOT(saveState()));
    connect(ui->lineEditDirPath, SIGNAL(textChanged(QString)), this, SLOT(saveState()));
    connect(ui->checkBoxConvertUnderscore, SIGNAL(clicked(bool)), this, SLOT(saveState()));
    connect(ui->checkBoxConvertUnderscoreFiles, SIGNAL(clicked(bool)), this, SLOT(saveState()));
    connect(ui->btnBrowse, &QPushButton::clicked, this, &DialogUpdate::btnBrowseClicked);
    connect(ui->btnLoadCsv, &QPushButton::clicked, this, &DialogUpdate::btnLoadCsvClicked);
    connect(ui->btnSend, &QPushButton::clicked, this, &DialogUpdate::btnSendClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DialogUpdate::close);
    connect(ui->btnScanFiles, &QPushButton::clicked, this, &DialogUpdate::btnScanFilesClicked);
    connect(ui->btnBrowseDirs, &QPushButton::clicked, this, &DialogUpdate::btnBrowseDirsClicked);
}

DialogUpdate::~DialogUpdate() = default;

void DialogUpdate::btnBrowseClicked()
{
    QString defaultFilePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString saveFilePath = QFileDialog::getOpenFileName(this,tr("Select CSV to import"), defaultFilePath, tr("(*.csv)"));
    if (saveFilePath != "")
    {
        ui->lineEditCsvPath->setText(saveFilePath);
    }
}

void DialogUpdate::btnLoadCsvClicked()
{
    ui->tableWidgetPreview->clear();
    ui->tableWidgetPreview->setRowCount(0);
    ui->tableWidgetPreview->setHorizontalHeaderLabels(headerLabels);
    songs.clear();
    QString path = ui->lineEditCsvPath->text();
    if (path == "")
        return;
    QFile csvFile(path);
    if (!csvFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(this, "Error opening file", csvFile.errorString());
        return;
    }
    else
    {
        ui->tableWidgetPreview->setHorizontalHeaderLabels(headerLabels);
        auto *progressDialog = new QProgressDialog(this);
        progressDialog->setCancelButton(nullptr);
        progressDialog->setMinimum(0);
        progressDialog->setMaximum(0);
        progressDialog->setValue(0);
        progressDialog->setLabelText("Processing CSV data...");
        progressDialog->show();
        QApplication::processEvents();
        int ctr = 0;
        while(!csvFile.atEnd())
        {
            QString artist;
            QString title;
            int artistCol = ui->spinBoxColArtist->value() - 1;
            int titleCol  = ui->spinBoxColTitle->value() - 1;
            QString line = csvFile.readLine();
            if (ui->checkBoxConvertUnderscore->isChecked())
                line.replace("_", " ");
            QStringList parts = parseCsvString(line);
            if (artistCol < parts.size())
                artist = parts.at(artistCol);
            if (titleCol < parts.size())
                title = parts.at(titleCol);
            OkjsSong song(artist, title);
            if (songs.contains(song))
                continue;
            songs.insert(song);
            int row = ui->tableWidgetPreview->rowCount();
            ui->tableWidgetPreview->setRowCount(row + 1);
            auto *newItem = new QTableWidgetItem(artist);
            ui->tableWidgetPreview->setItem(row, 0, newItem);
            newItem = new QTableWidgetItem(title);
            ui->tableWidgetPreview->setItem(row, 1, newItem);
            ui->tableWidgetPreview->setHorizontalHeaderLabels(headerLabels);
            if (ctr % 100 == 0)
                QApplication::processEvents();
            ctr++;
        }
        csvFile.close();
        progressDialog->hide();
        delete progressDialog;
    }

}

QStringList DialogUpdate::parseCsvString(const QString& string)
{
    enum State {Normal, Quote} state = Normal;
    QStringList fields;
    QString value;

    for (int i = 0; i < string.size(); i++)
    {
        QChar current = string.at(i);

        // Normal state
        if (state == Normal)
        {
            // Comma
            if (current == ',')
            {
                // Save field
                fields.append(value);
                value.clear();
            }

            // Double-quote
            else if (current == '"')
                state = Quote;

            // Other character
            else
                value += current;
        }

        // In-quote state
        else if (state == Quote)
        {
            // Another double-quote
            if (current == '"')
            {
                if (i+1 < string.size())
                {
                    QChar next = string.at(i+1);

                    // A double double-quote?
                    if (next == '"')
                    {
                        value += '"';
                        i++;
                    }
                    else
                        state = Normal;
                }
            }

            // Other character
            else
                value += current;
        }
    }
    if (!value.isEmpty())
        fields.append(value);

    return fields;
}

QStringList DialogUpdate::findKaraokeFiles(const QString& directory)
{
    QStringList files;
    QDir dir(directory);
    QDirIterator iterator(dir.absolutePath(), QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        iterator.next();
        if (!iterator.fileInfo().isDir()) {
            QString fn = iterator.filePath();
            if (fn.endsWith(".zip",Qt::CaseInsensitive) || fn.endsWith(".cdg", Qt::CaseInsensitive) || fn.endsWith(".mkv", Qt::CaseInsensitive) || fn.endsWith(".avi", Qt::CaseInsensitive) || fn.endsWith(".wmv", Qt::CaseInsensitive) || fn.endsWith(".mp4", Qt::CaseInsensitive) || fn.endsWith(".mpg", Qt::CaseInsensitive) || fn.endsWith(".mpeg", Qt::CaseInsensitive))
                files.append(fn);
        }
    }
    return files;
}

void DialogUpdate::saveState()
{
    importerConfig fConfig;
    importerConfig cConfig;
    fConfig.artistCol = ui->spinBoxSecArtist->value();
    fConfig.titleCol  = ui->spinBoxSecTitle->value();
    fConfig.separator = ui->lineEditSepFile->text();
    fConfig.path      = ui->lineEditDirPath->text();
    fConfig.convertUnderscore = ui->checkBoxConvertUnderscoreFiles->isChecked();

    cConfig.artistCol = ui->spinBoxColArtist->value();
    cConfig.titleCol  = ui->spinBoxColTitle->value();
    cConfig.path      = ui->lineEditCsvPath->text();
    cConfig.convertUnderscore = ui->checkBoxConvertUnderscore->isChecked();

    settings.saveFileImporterConfig(fConfig);
    settings.saveCsvImporterConfig(cConfig);
}

void DialogUpdate::btnSendClicked()
{
    auto *progressDialog = new QProgressDialog(this);
    progressDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    progressDialog->setCancelButton(nullptr);
    progressDialog->setMinimum(0);
    progressDialog->setMaximum(20);
    progressDialog->setValue(0);
    progressDialog->setLabelText("Updating request server song database");
    progressDialog->show();
    QApplication::processEvents();
    connect(&sbApi, SIGNAL(remoteSongDbUpdateNumDocs(int)), progressDialog, SLOT(setMaximum(int)));
    connect(&sbApi, SIGNAL(remoteSongDbUpdateProgress(int)), progressDialog, SLOT(setValue(int)));
    sbApi.updateSongDb(songs);
    progressDialog->close();
}

void DialogUpdate::btnScanFilesClicked()
{
    ui->tableWidgetPreview->clear();
    ui->tableWidgetPreview->setRowCount(0);
    ui->tableWidgetPreview->setHorizontalHeaderLabels(headerLabels);
    songs.clear();

    QString dir = ui->lineEditDirPath->text();
    QStringList files = findKaraokeFiles(dir);

    auto *progressDialog = new QProgressDialog(this);
    progressDialog->setCancelButton(nullptr);
    progressDialog->setMinimum(0);
    progressDialog->setMaximum(files.size());
    progressDialog->setValue(0);
    progressDialog->setLabelText("Processing files...");
    progressDialog->show();
    QApplication::processEvents();
    for (int i=0; i < files.size(); i++)
    {
        QString artist;
        QString title;
        int artistSec = ui->spinBoxSecArtist->value() - 1;
        int titleSec  = ui->spinBoxSecTitle->value() - 1;
        QString basename = QFileInfo(files.at(i)).completeBaseName();
        if (ui->checkBoxConvertUnderscoreFiles->isChecked())
            basename.replace("_", " ");
        QStringList parts = basename.split(ui->lineEditSepFile->text());
        if (artistSec < parts.size())
            artist = parts.at(artistSec);
        if (titleSec < parts.size())
            title = parts.at(titleSec);
        OkjsSong song(artist, title);
        if (songs.contains(song))
            continue;
        songs.insert(song);
        int row = ui->tableWidgetPreview->rowCount();
        ui->tableWidgetPreview->setRowCount(row + 1);
        auto *newItem = new QTableWidgetItem(artist);
        ui->tableWidgetPreview->setItem(row, 0, newItem);
        newItem = new QTableWidgetItem(title);
        ui->tableWidgetPreview->setItem(row, 1, newItem);
        ui->tableWidgetPreview->setHorizontalHeaderLabels(headerLabels);
        progressDialog->setValue(i);
        if (i % 100 == 0)
            QApplication::processEvents();
    }
    progressDialog->close();
    delete progressDialog;
}

void DialogUpdate::btnBrowseDirsClicked()
{
    QString fileName = QFileDialog::getExistingDirectory(this);
    if (fileName != "")
    {
        ui->lineEditDirPath->setText(fileName);
    }
}
