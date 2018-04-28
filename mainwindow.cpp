
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QString>
#include <QDir>
#include <QDirIterator>

using namespace QtConcurrent;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    iv = std::shared_ptr<ImageViewer>(new ImageViewer());
    ui->wImageContainer->layout()->addWidget(iv.get());
    qDebug() << ui->wImageContainer->layout()->objectName();

    connect(ui->sbRows, SIGNAL(valueChanged(QString)), this, SLOT(onGridPropsValueChanged(QString)));
    connect(ui->sbCols, SIGNAL(valueChanged(QString)), this, SLOT(onGridPropsValueChanged(QString)));
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

    QSize gridSize = QSize(ui->sbCols->value(), ui->sbRows->value());

    iv->setImage(m_baseImage);
    iv->setGrid(gridSize);

    QtConcurrent::run([=]() {
        m_imageProcessing.processGrid(m_baseImage, gridSize);
        ui->btnGenerate->setEnabled(m_imageProcessing.isReady());
    });
}

void MainWindow::onGridPropsValueChanged(const QString &arg1)
{
    if(m_baseImage.width() == 0 || m_baseImage.height() == 0)
        return;

    iv->setGrid(QSize(ui->sbCols->value(), ui->sbRows->value()));
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
    QSize gridSize = QSize(ui->sbCols->value(), ui->sbRows->value());
    QMap<QPoint, QImage> dst;
    qDebug() << "result generate image: " << m_imageProcessing.generateImage(m_baseImage.size(), gridSize, &dst);
}
