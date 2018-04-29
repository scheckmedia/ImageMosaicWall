
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QString>
#include <QDir>
#include <QDirIterator>
#include <QFutureWatcher>

using namespace QtConcurrent;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    iv = std::shared_ptr<ImageViewer>(new ImageViewer());
    ui->wImageContainer->layout()->addWidget(iv.get());
    qDebug() << ui->wImageContainer->layout()->objectName();

    qRegisterMetaType<GridPoint>("GridPoint");
    connect(ui->sbRows, SIGNAL(valueChanged(QString)), this, SLOT(onGridPropsValueChanged()));
    connect(ui->sbCols, SIGNAL(valueChanged(QString)), this, SLOT(onGridPropsValueChanged()));
    connect(ui->btnSetResolution, SIGNAL(clicked(bool)), this, SLOT(onOutputResolutionChanged()));
    connect(&m_imageProcessing, &ImageProcessing::mosaicGenerated, iv.get(), &ImageViewer::setLoadingMosaicAt);
    connect(this, &MainWindow::mosaicCalculationFinished, iv.get(), &ImageViewer::setMosaicImages);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnLoad_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Image"),  QDir::homePath(), tr("ImageFiles (*.png *.jpg *bmp)"));

    if (filename.isEmpty())
        return;


    m_baseImage = QImage(filename);
    QString imageInfo = QString("%1 %2x%3").arg(tr("Image Resolution")).arg(m_baseImage.width()).arg(m_baseImage.height());
    ui->lblImageInfo->setText(imageInfo);

    ui->sbWidth->setValue(m_baseImage.width());
    ui->sbHeight->setValue(m_baseImage.height());

    QSize gridSize = QSize(ui->sbCols->value(), ui->sbRows->value());

    iv->setImage(m_baseImage);
    iv->setGrid(gridSize);

    QtConcurrent::run([=]() {
        m_imageProcessing.processGrid(m_baseImage, gridSize);
        ui->btnGenerate->setEnabled(m_imageProcessing.isReady());
    });
}

void MainWindow::onGridPropsValueChanged()
{
    if(m_baseImage.width() == 0 || m_baseImage.height() == 0)
        return;

    QSize gridSize(ui->sbCols->value(), ui->sbRows->value());
    iv->setGrid(gridSize);
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
    iv->setImage(m_baseImage);
    onGridPropsValueChanged();
}

void MainWindow::onMosaicCreationFinished()
{
    qDebug() << " some shit is done";
    emit mosaicCalculationFinished(m_mappedImages);
    delete m_mosaicGeneration;
}

void MainWindow::on_pushButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Load Images Folder"), QDir::homePath());

    QStringList filter = QStringList() << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp";
    QDirIterator iterator(path, filter, QDir::Files, QDirIterator::Subdirectories);

    QList<QString> imageList;
    while(iterator.hasNext())
    {
        iterator.next();
        imageList.append(iterator.filePath());
    }

    ui->lblImagesFolder->setText(QString("%1 Files selected").arg(imageList.size()));
    QtConcurrent::run([=]() {
        m_imageProcessing.processMosaicImages(imageList);
        ui->btnGenerate->setEnabled(m_imageProcessing.isReady());
    });

}


void MainWindow::on_btnGenerate_clicked()
{
    m_mappedImages.clear();
    m_mosaicGeneration = QThread::create([&]{
        QSize gridSize = QSize(ui->sbCols->value(), ui->sbRows->value());

        m_imageProcessing.moveToThread(this->thread());
        bool success = m_imageProcessing.generateImage(m_baseImage.size(), gridSize, &m_mappedImages);

        if(!success)
            return;

        qDebug() << "result generate image: " << success << " dstSize: " << m_mappedImages.size() ;
        //emit iv.get()->setMosaicImages(m_mappedImages);
    });
    connect(m_mosaicGeneration, SIGNAL(finished()), this, SLOT(onMosaicCreationFinished()));
    m_mosaicGeneration->start();
}
