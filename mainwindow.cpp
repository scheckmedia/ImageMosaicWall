
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QString>
#include <QDir>
#include <QDirIterator>
#include <QFutureWatcher>

using namespace QtConcurrent;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_loadingSequence(":/icons/assets/loader.gif")
{
    ui->setupUi(this);        

    QFile file(":/styles/default.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    setStyleSheet(styleSheet);

    ui->wImageContainer->layout()->addWidget(&m_imageView);        

    qRegisterMetaType<GridPoint>("GridPoint");
    connect(ui->sbRows, SIGNAL(valueChanged(QString)), this, SLOT(onGridPropsValueChanged()));
    connect(ui->sbCols, SIGNAL(valueChanged(QString)), this, SLOT(onGridPropsValueChanged()));
    connect(ui->btnSetResolution, SIGNAL(clicked(bool)), this, SLOT(onOutputResolutionChanged()));
    connect(&m_imageProcessing, &ImageProcessing::mosaicGenerated, &m_imageView, &ImageViewer::setLoadingMosaicAt);    
    connect(&m_imageView, &ImageViewer::folderDropped, this, &MainWindow::onFolderDropped);
    connect(&m_imageView, &ImageViewer::imageDropped, this, &MainWindow::onImageDropped);
}

MainWindow::~MainWindow()
{
    delete ui;
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

    if(!m_baseImage.isNull())
        ui->btnSetResolution->setEnabled(true);
}

void MainWindow::enableEnableUi(bool enabled)
{
    ui->btnGenerate->setEnabled(enabled);
    ui->btnLoad->setEnabled(enabled);
    ui->btnSave->setEnabled(enabled);
    ui->btnSetResolution->setEnabled(enabled);
    ui->btnSetImageFolder->setEnabled(enabled);

    ui->sbCols->setEnabled(enabled);
    ui->sbRows->setEnabled(enabled);
    ui->sbHeight->setEnabled(enabled);
    ui->sbWidth->setEnabled(enabled);
    ui->sbHistory->setEnabled(enabled);
}

void MainWindow::loadImage(QString &filename)
{
    if (filename.isEmpty())
        return;

    m_baseImage = QImage(filename);

    if(m_baseImage.isNull())
        return;

    setLoadingState(*ui->btnLoad, true);
    updateStatus();

    ui->sbWidth->setValue(m_baseImage.width());
    ui->sbHeight->setValue(m_baseImage.height());

    QSize gridSize = QSize(ui->sbCols->value(), ui->sbRows->value());

    m_imageView.setImage(m_baseImage);
    m_imageView.setGrid(gridSize);

    QtConcurrent::run([=]() {
        m_imageProcessing.processGrid(m_baseImage, gridSize);
        ui->btnGenerate->setEnabled(m_imageProcessing.isReady());
        setLoadingState(*ui->btnLoad, false);
    });
}

void MainWindow::loadImageFolder(QString &path)
{
    setLoadingState(*ui->btnSetImageFolder, true);
    QStringList filter = QStringList() << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp";
    QDirIterator iterator(path, filter, QDir::Files, QDirIterator::Subdirectories);

    QList<QString> imageList;
    while(iterator.hasNext())
    {
        iterator.next();
        imageList.append(iterator.filePath());
    }

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    QFuture<void> f = QtConcurrent::run([=]() {
        m_imageProcessing.processMosaicImages(imageList);
        ui->btnGenerate->setEnabled(m_imageProcessing.isReady());
    });

    connect(watcher, &QFutureWatcher<void>::finished, [=]() {
       updateStatus();
       setLoadingState(*ui->btnSetImageFolder, false);
       watcher->deleteLater();
    });
    watcher->setFuture(f);

}

void MainWindow::setLoadingState(QPushButton &btn, bool isLoading)
{
    if(isLoading)
    {
        m_loadingSequence.start();
        m_activeLoadingButtons.insert(btn.objectName(), btn.icon());
        connect(&m_loadingSequence, &QMovie::frameChanged, [&](int){
            btn.setIcon(m_loadingSequence.currentPixmap());
        });
    }
    else {
        //disconnect
        disconnect(&m_loadingSequence, &QMovie::frameChanged, 0, 0);
        btn.setIcon(m_activeLoadingButtons.value(btn.objectName()));
        m_loadingSequence.stop();
    }
}




void MainWindow::on_btnLoad_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Image"),  QDir::homePath(), tr("ImageFiles (*.png *.jpg *bmp)"));
    loadImage(filename);
}

void MainWindow::onGridPropsValueChanged()
{
    if(m_baseImage.width() == 0 || m_baseImage.height() == 0)
        return;

    QSize gridSize(ui->sbCols->value(), ui->sbRows->value());
    m_imageView.setGrid(gridSize);
    QtConcurrent::run([=]() {
        m_imageProcessing.processGrid(m_baseImage, gridSize);
        ui->btnGenerate->setEnabled(m_imageProcessing.isReady());
    });
}

void MainWindow::onOutputResolutionChanged()
{
    QSize res = QSize(ui->sbWidth->value(), ui->sbHeight->value());

    if(res.width() <= 0 || res.height() <= 0 || m_baseImage.isNull())
        return;

    m_baseImage = m_baseImage.scaled(res);
    m_imageView.setImage(m_baseImage);
    onGridPropsValueChanged();
    ui->btnSave->setEnabled(false);
}

void MainWindow::onMosaicCreationFinished()
{    
    m_imageView.setMosaicLoadingDone();
    m_imageView.setPreview(m_imageProcessing.getOutputImage());
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


void MainWindow::on_btnGenerate_clicked()
{
    enableEnableUi(false);
    m_imageView.clearPreview();
    m_mosaicGeneration = QThread::create([&]{
        QSize gridSize = QSize(ui->sbCols->value(), ui->sbRows->value());

        m_imageProcessing.moveToThread(this->thread());
        bool success = m_imageProcessing.generateImage(m_baseImage.size(), gridSize, ui->sbHistory->value());

        if(!success)
            return;

        enableEnableUi(true);

    });
    connect(m_mosaicGeneration, SIGNAL(finished()), this, SLOT(onMosaicCreationFinished()));
    m_mosaicGeneration->start();
}

void MainWindow::on_btnSave_clicked()
{
    QString defaultFileExtension = "PNG (*.png)";
    QString filename = QFileDialog::getSaveFileName(this, "Save Mosaic Image",
                                 QDir::homePath(),
                                 tr("JPEG (*.jpg);;PNG (*.png)"), &defaultFileExtension);
    m_imageProcessing.getOutputImage().save(filename);
}

void MainWindow::on_btnSetImageFolder_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Load Images Folder"), QDir::homePath());
    loadImageFolder(path);
}
