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

public slots:    
    void on_btnLoad_clicked();
    void onGridPropsValueChanged(const QString &arg1);    

private slots:
    void on_pushButton_clicked();
    void on_btnGenerate_clicked();

private:
    Ui::MainWindow *ui;
    QImage m_baseImage;
    ImageProcessing m_imageProcessing;
    std::shared_ptr<ImageViewer> iv;
    QFutureWatcher<QRgb> *m_imageScaling;
};

#endif // MAINWINDOW_H
