#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QWheelEvent>
#include <QGraphicsView>
#include <QGraphicsItemGroup>

namespace Ui {
class ImageViewer;
}

class ImageViewer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget *parent = 0);
    ~ImageViewer();

    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void setImage(QImage img);
    void setGrid(QSize gridResolution);

private:
    Ui::ImageViewer *ui;
    QImage m_image;
    QPoint m_moveOffset;
    QGraphicsItemGroup* m_grid = nullptr;

};

#endif // IMAGEVIEWER_H
