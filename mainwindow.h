#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QMovie>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QMap>
#include <QPushButton>
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

    void setCurrentFolder(const QString& folder);

private:
    void updateStatus();
    void enableEnableUi(bool enabled);
    void loadImage(QString&);
    void loadImageFolder(QString&);
    void setLoadingState(QPushButton &btn, bool isLoading);
    void scaleLockedImageSize(bool senderIsWidth);

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
    QMap<QString, QIcon> m_activeLoadingButtons;
    QMovie m_loadingSequence;
    QSize m_lockedResolution;
    QString m_currentFolder;
    QString m_baseImagePath;
};

#endif // MAINWINDOW_H
