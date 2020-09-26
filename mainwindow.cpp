
#include "mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QString>
#include "asyncdirectoryscanner.h"
#include "progressbutton.h"
#include "ui_mainwindow.h"
#include "i18nutils.h"

using namespace QtConcurrent;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    s_initI18nResource();
    s_loadI18n(QLatin1String("ImageMosaicWall"));

    QFile file(":/styles/default.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    setStyleSheet(styleSheet);

    ui->btnCancel->hide();
    ui->wImageContainer->layout()->addWidget(&m_imageView);

    qRegisterMetaType<GridPoint>("GridPoint");
    connect(ui->sbRows, SIGNAL(valueChanged(QString)), this, SLOT(onGridPropsValueChanged()));
    connect(ui->sbCols, SIGNAL(valueChanged(QString)), this, SLOT(onGridPropsValueChanged()));
    connect(ui->btnSetResolution, SIGNAL(clicked(bool)), this, SLOT(onOutputResolutionChanged()));

    connect(ui->btnCancel, &QPushButton::clicked, [&]() {
        qDebug() << "Cancel!";
        m_imageProcessing.processCanceled(true);
    });

    connect(&m_imageProcessing, &ImageProcessing::mosaicGenerated, &m_imageView, &ImageViewer::setLoadingMosaicAt);
    connect(&m_imageProcessing, &ImageProcessing::imageProcessed, this, &MainWindow::onImageLoadPorgress);

    connect(&m_imageProcessing, &ImageProcessing::cellProcessed, this, &MainWindow::onImageCellProgress);

    connect(&m_imageView, &ImageViewer::folderDropped, this, &MainWindow::onFolderDropped);
    connect(&m_imageView, &ImageViewer::imageDropped, this, &MainWindow::onImageDropped);
    connect(ui->btnLockRatio, &QPushButton::clicked, [=]() {
        m_lockedResolution.setWidth(ui->sbWidth->value());
        m_lockedResolution.setHeight(ui->sbHeight->value());

        if (ui->btnLockRatio->isChecked())
            ui->btnLockRatio->setIcon(QIcon(":/icons/assets/locked.png"));
        else
            ui->btnLockRatio->setIcon(QIcon(":/icons/assets/opened.png"));
    });
    connect(ui->sbWidth, QOverload<int>::of(&QSpinBox::valueChanged), [=]() {
        if (ui->btnLockRatio->isChecked())
        {
            ui->sbHeight->blockSignals(true);
            scaleLockedImageSize(true);
            ui->sbHeight->blockSignals(false);
        }
    });
    connect(ui->sbHeight, QOverload<int>::of(&QSpinBox::valueChanged), [=]() {
        if (ui->btnLockRatio->isChecked())
        {
            ui->sbWidth->blockSignals(true);
            scaleLockedImageSize(false);
            ui->sbWidth->blockSignals(false);
        }
    });

    connect(ui->slHistory, QOverload<int>::of(&QSlider::valueChanged), [=]() {
        QString val;
        switch (ui->slHistory->value())
        {
            case 0:
                val = tr("disabled");
                break;
            case 1:
                val = tr("very low");
                break;
            case 2:
                val = tr("low");
                break;
            case 3:
                val = tr("medium");
                break;
            case 4:
                val = tr("high");
                break;
            case 5:
                val = tr("very high");
                break;
            default:
                break;
        }

        ui->lblVariation->setText(val);
    });
}

MainWindow::~MainWindow()
{
    delete ui;

    s_cleanupI18nResource();
}

void MainWindow::updateStatus()
{
    QString imageInfo = QString("<b style=\"text-indent: 100px\">%1</b> %2x%3")
                            .arg(tr("Image Resolution:"))
                            .arg(m_baseImage.width())
                            .arg(m_baseImage.height());

    QString folderInfo = QString("<b style=\"text-indent: 100px\">%1</b> %2")
                             .arg(tr("Images in Folder:"))
                             .arg(m_imageProcessing.getImageMeanMap().size());

    ui->lblStatus->setText(QString("%1<br/>%2").arg(imageInfo).arg(folderInfo));

    if (!m_baseImage.isNull())
        ui->btnSetResolution->setEnabled(true);

    ui->btnCancel->hide();
}

void MainWindow::toggleUI(bool enabled)
{
    auto out = m_imageProcessing.getOutputImage();
    ui->btnGenerate->setEnabled(enabled && m_imageProcessing.isReady());
    ui->btnSave->setEnabled(enabled && out != nullptr);

    ui->btnLoad->setEnabled(enabled);
    ui->btnSetResolution->setEnabled(enabled);
    ui->btnSetImageFolder->setEnabled(enabled);

    ui->sbCols->setEnabled(enabled);
    ui->sbRows->setEnabled(enabled);
    ui->sbHeight->setEnabled(enabled);
    ui->sbWidth->setEnabled(enabled);
    ui->slHistory->setEnabled(enabled);
}

void MainWindow::loadImage(QString &filename)
{
    if (filename.isEmpty())
        return;

    m_baseImage = QImage(filename);

    if (m_baseImage.isNull())
        return;

    m_baseImagePath = filename;

    if (ui->btnLockRatio->isChecked())
        m_lockedResolution = m_baseImage.size();

    ui->sbWidth->setValue(m_baseImage.width());
    ui->sbHeight->setValue(m_baseImage.height());

    QSize gridSize = QSize(ui->sbCols->value(), ui->sbRows->value());

    ui->btnLoad->setRange(0, gridSize.width() * gridSize.height());
    ui->btnCancel->show();
    m_imageView.setImage(m_baseImage);
    m_imageView.setGrid(gridSize);

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    QFuture<void> f = QtConcurrent::run([=]() { m_imageProcessing.processGrid(m_baseImage, gridSize); });

    connect(watcher, &QFutureWatcher<void>::finished, [=]() {
        ui->btnGenerate->setEnabled(m_imageProcessing.isReady());
        watcher->deleteLater();
        updateStatus();
        toggleUI(true);
    });

    toggleUI(false);
    m_imageProcessing.processCanceled(false);
    watcher->setFuture(f);
}

void MainWindow::loadImageFolder(QString &path)
{
    if (path.isEmpty())
        return;

    QStringList filter = QStringList() << "*.png"
                                       << "*.jpg"
                                       << "*.jpeg"
                                       << "*.bmp";

    auto scanner = new AsyncDirectoryScanner(
        path, filter, QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot, QDirIterator::Subdirectories);

    connect(scanner, &AsyncDirectoryScanner::fileScanned, this, [=](const QString file) {
        auto message = tr("%1 images scanned").arg(scanner->numFilesScanned());
        ui->lblStatus->setText(message);
    });

    connect(scanner, &AsyncDirectoryScanner::finished, this, [=]() {
        qDebug() << "finished scanning";
        auto imageList = scanner->scannedFiles();

        ui->btnSetImageFolder->setRange(0, imageList.size());
        ui->btnCancel->show();

        QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
        QFuture<void> f = QtConcurrent::run([=]() {
            m_imageProcessing.processMosaicImages(imageList);
            if (!m_imageProcessing.skipBackgroundProcesses())
                ui->btnGenerate->setEnabled(m_imageProcessing.isReady());
        });

        connect(watcher, &QFutureWatcher<void>::finished, [=]() {
            watcher->deleteLater();
            updateStatus();
            toggleUI(true);
        });

        toggleUI(false);
        m_imageProcessing.processCanceled(false);
        watcher->setFuture(f);
        scanner->deleteLater();
    });

    scanner->start();
}

void MainWindow::scaleLockedImageSize(bool senderIsWidth)
{
    double n = qMax(m_lockedResolution.width(), m_lockedResolution.height());
    double d = qMin(m_lockedResolution.width(), m_lockedResolution.height());
    double ratio = n / d;

    if (ui->sbWidth->value() >= ui->sbHeight->value())
    {
        if (senderIsWidth)
            ui->sbHeight->setValue(ui->sbWidth->value() / ratio);
        else
            ui->sbWidth->setValue(ui->sbHeight->value() * ratio);
    }
    else
    {
        if (senderIsWidth)
            ui->sbHeight->setValue(ui->sbWidth->value() * ratio);
        else
            ui->sbWidth->setValue(ui->sbHeight->value() / ratio);
    }
}

void MainWindow::on_btnLoad_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image"),
                                                    m_currentFolder.isEmpty() ? QDir::homePath() : m_currentFolder,
                                                    tr("ImageFiles (*.png *.jpg *bmp)"));
    loadImage(filename);
}

void MainWindow::onGridPropsValueChanged()
{
    if (m_baseImage.width() == 0 || m_baseImage.height() == 0)
        return;

    QSize gridSize(ui->sbCols->value(), ui->sbRows->value());
    m_imageView.setGrid(gridSize);
    ui->btnLoad->setRange(0, gridSize.width() * gridSize.height());

    QtConcurrent::run([=]() {
        m_imageProcessing.processGrid(m_baseImage, gridSize);
        ui->btnGenerate->setEnabled(m_imageProcessing.isReady());
    });
}

void MainWindow::onOutputResolutionChanged()
{
    QSize res = QSize(ui->sbWidth->value(), ui->sbHeight->value());

    if (res.width() <= 0 || res.height() <= 0 || m_baseImage.isNull())
        return;

    m_baseImage = m_baseImage.scaled(res);
    m_imageView.setImage(m_baseImage);
    onGridPropsValueChanged();
    ui->btnSave->setEnabled(false);
}

void MainWindow::onMosaicCreationFinished()
{
    m_imageView.setMosaicLoadingDone();
    m_imageView.setPreview(*m_imageProcessing.getOutputImage());
    ui->btnSave->setEnabled(true);
    delete m_mosaicGeneration;
}

void MainWindow::onImageDropped(QString image)
{
    loadImage(image);
}

void MainWindow::onFolderDropped(QString folder)
{
    loadImageFolder(folder);
}

void MainWindow::onImageCellProgress()
{
    ui->btnLoad->increment();

    auto current = ui->btnLoad->currentValue();
    auto max = ui->btnLoad->maxValue();
    auto message = tr("%1 of %2 cells processed").arg(current).arg(max);

    ui->lblStatus->setText(message);
}

void MainWindow::onImageLoadPorgress()
{
    ui->btnSetImageFolder->increment();

    auto current = ui->btnSetImageFolder->currentValue();
    auto max = ui->btnSetImageFolder->maxValue();
    auto message = tr("%1 of %2 images processed").arg(current).arg(max);

    ui->lblStatus->setText(message);
}

void MainWindow::on_btnGenerate_clicked()
{
    toggleUI(false);
    m_imageProcessing.processCanceled(false);
    m_imageView.clearPreview();
    ui->btnCancel->show();
    m_mosaicGeneration = QThread::create([&] {
        QSize gridSize = QSize(ui->sbCols->value(), ui->sbRows->value());

        m_imageProcessing.moveToThread(this->thread());
        bool success = m_imageProcessing.generateImage(m_baseImage.size(), gridSize, m_variations[ui->slHistory->value()]);

        if (!success)
            return;

        toggleUI(true);
        ui->btnCancel->hide();
    });
    connect(m_mosaicGeneration, SIGNAL(finished()), this, SLOT(onMosaicCreationFinished()));
    m_mosaicGeneration->start();
}

void MainWindow::on_btnSave_clicked()
{
    QString defaultFileExtension = "PNG (*.png)";
    QString filename = QFileDialog::getSaveFileName(this,
                                                    "Save Mosaic Image",
                                                    m_baseImagePath.isEmpty() ? QDir::homePath() : m_baseImagePath,
                                                    tr("JPEG (*.jpg);;PNG (*.png)"),
                                                    &defaultFileExtension);
    m_imageProcessing.getOutputImage()->save(filename);
}

void MainWindow::on_btnSetImageFolder_clicked()
{
    QString path = QFileDialog::getExistingDirectory(
        this, tr("Load Images Folder"), m_currentFolder.isEmpty() ? QDir::homePath() : m_currentFolder);
    loadImageFolder(path);
}

void MainWindow::setCurrentFolder(const QString &folder)
{
    m_currentFolder = folder;
}
