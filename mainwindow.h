#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "imageprocessing.h"
#include "imageviewer.h"
#include <QFutureWatcher>
#include <QImage>
#include <QMainWindow>
#include <QMap>
#include <QMovie>
#include <QPushButton>
#include <QtConcurrent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  void setCurrentFolder(const QString &folder);

private:
  void updateStatus();
  void toggleUI(bool enabled);
  void loadImage(QString &);
  void loadImageFolder(QString &);
  void scaleLockedImageSize(bool senderIsWidth);

signals:
  void mosaicCalculationFinished(QMap<GridPoint, QImage> &);

public slots:
  void on_btnLoad_clicked();
  void onGridPropsValueChanged();
  void onOutputResolutionChanged();
  void onMosaicCreationFinished();
  void onImageDropped(QString image);
  void onFolderDropped(QString folder);
  void onImageCellProgress();
  void onImageLoadPorgress();

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
  QSize m_lockedResolution;

  const ushort m_variations[6] = {0, 1, 3, 9, 11, 15};
  QString m_currentFolder;
  QString m_baseImagePath;
};

#endif // MAINWINDOW_H
