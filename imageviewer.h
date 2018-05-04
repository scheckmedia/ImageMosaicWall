#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include "imageprocessing.h"
#include <QWidget>
#include <QWheelEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItemGroup>

namespace Ui {
class ImageViewer;
}

class ImageViewer : public QGraphicsView
{
    Q_OBJECT

signals:
    void imageDropped(QString path);
    void folderDropped(QString path);

public slots:
    void setLoadingMosaicAt(const GridPoint);

public:
    explicit ImageViewer(QWidget *parent = 0);
    ~ImageViewer();

    void clearPreview();
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void setImage(const QImage& img);
    void setImage(const QImage& img, QSize resolution);
    void setPreview(const QImage& img);
    void setGrid(QSize gridResolution);
    void setMosaicLoadingDone();

private:
    void fitToScene(const QImage &);

protected:
    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;
    void dragMoveEvent(QDragMoveEvent *) override;


private:
    Ui::ImageViewer *ui;
    QImage m_baseImage;
    QPoint m_moveOffset;
    QSize m_gridResolution;
    QGraphicsPixmapItem* m_image = nullptr;
    QGraphicsPixmapItem* m_preview = nullptr;
    QGraphicsItemGroup* m_grid = nullptr;
    QGraphicsItemGroup* m_mosaicLoading = nullptr;
    QMutex m_lockLoading;

};

#endif // IMAGEVIEWER_H
