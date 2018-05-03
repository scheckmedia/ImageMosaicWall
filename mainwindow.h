#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QMap>
#include "imageviewer.h"
#include "imageprocessing.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void updateStatus();
    void enableDisableUi(bool enabled);
    void loadImage(QString&);
    void loadImageFolder(QString&);

signals:
    void mosaicCalculationFinished(QMap<GridPoint, QImage>&);


public slots:    
    void on_btnLoad_clicked();
    void onGridPropsValueChanged();
    void onOutputResolutionChanged();
    void onMosaicCreationFinished();
    void onImageDropped(QString image);
    void onFolderDropped(QString folder);

private slots:
    void on_btnGenerate_clicked();
    void on_btnSave_clicked();
    void on_btnSetImageFolder_clicked();

private:
    Ui::MainWindow *ui;
    QImage m_baseImage;
    ImageProcessing m_imageProcessing;
    ImageViewer m_imageView;
    QFutureWatcher<QRgb> *m_imageScaling;
    QThread *m_mosaicGeneration;
    QMap<GridPoint, QImage> m_mappedImages;
};

#endif // MAINWINDOW_H
