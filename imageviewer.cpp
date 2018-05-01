#include "imageviewer.h"
#include "ui_imageviewer.h"
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QScrollBar>
#include <qmath.h>

ImageViewer::ImageViewer(QWidget *parent) :
    QGraphicsView(parent),    
    ui(new Ui::ImageViewer)
{
    ui->setupUi(this);
    QGraphicsScene *scene = new QGraphicsScene(this);
    setScene(scene);

    m_imageItem = new QGraphicsPixmapItem();
    m_grid = new QGraphicsItemGroup(m_imageItem);
    m_mosaicImages = new QGraphicsItemGroup(m_imageItem);
    m_mosaicLoading = new QGraphicsItemGroup(m_imageItem);
    scene->addItem(m_imageItem);

    m_mosaicLoading->setZValue(10);
}

ImageViewer::~ImageViewer()
{
    delete ui;
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    const QPointF p0scene = mapToScene(event->pos());

    qreal factor = qPow(1.2, event->delta() / 240.0);
    scale(factor, factor);

    const QPointF p1mouse = mapFromScene(p0scene);
    const QPointF move = p1mouse - event->pos(); // The move
    horizontalScrollBar()->setValue(move.x() + horizontalScrollBar()->value());
    verticalScrollBar()->setValue(move.y() + verticalScrollBar()->value());

    double f = transform().m11();
    qDebug() << transform() << " m1: " << f << scene()->width() * f;
    qDebug() << "zoom: " << (f * (double)m_image.size().width()) << (f * (double)m_image.size().height() );
}

void ImageViewer::setImage(QImage img)
{
    setImage(img, img.size());
}

void ImageViewer::setImage(QImage img, QSize resolution)
{
    for(QGraphicsItem* item : m_mosaicImages->childItems())
    {
        delete item;
    }


    qDebug() << sceneRect();
    resetMatrix();

    m_image = img;
    QSize s = parentWidget()->size();
    m_imageItem->setPixmap(QPixmap::fromImage(m_image));
    setSceneRect(0.0, 0.0, m_image.width(), m_image.height());

    float sx = s.width() / (float)m_image.width();
    float sy = s.height() / (float)m_image.height();
    float ratio = qMin(sx, sy);
    scale(m_image.width() * ratio / m_image.width(), m_image.height() * ratio / m_image.height());
    show();
}

void ImageViewer::setGrid(QSize gridResolution)
{
    for(QGraphicsItem* item : m_mosaicImages->childItems())
    {
        delete item;
    }

    m_gridResolution = gridResolution;
    double gx = m_image.width() / (double)gridResolution.width();
    double gy = m_image.height() / (double)gridResolution.height();
    qDebug() << gx << " x " << gy;

    if(m_grid != nullptr)
    {
        for(QGraphicsItem* item : m_grid->childItems())
        {
            delete item;
        }
    }


    for(int x = 0; x < gridResolution.width(); ++x)
    {        
        QGraphicsLineItem* s = new QGraphicsLineItem(QLineF(QPointF(x * gx, 0), QPointF(x * gx, m_image.height())));
        m_grid->addToGroup(s);
    }

    for(int y = 0; y < gridResolution.height(); y++)
    {
        QGraphicsLineItem* s = new QGraphicsLineItem(QLine(QPoint(0, y * gy), QPoint(m_image.width(), y * gy)));
        m_grid->addToGroup(s);
    }
}

void ImageViewer::setMosaicImages(QMap<GridPoint, QImage> &images)
{        

    qDebug() << "in setMosiac...";
    setMosaicLoadingDone();
    if(m_mosaicImages != nullptr)
    {
        for(QGraphicsItem* item : m_mosaicImages->childItems())
        {
            delete item;
        }
    }

    qDebug() << " m_mosaicImages: " << m_mosaicImages;


    double gx = m_image.width() / (double)m_gridResolution.width();
    double gy = m_image.height() / (double)m_gridResolution.height();
    QSizeF cellSize(gx, gy);

    for(auto &p : images.keys())
    {
        QImage img = images.value(p);
        QGraphicsItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(img));
        item->setPos(QPointF(p.x() * cellSize.width(), p.y() * cellSize.height()));
        //item->setOpacity(0.8);

        m_mosaicImages->addToGroup(item);
    }

    m_grid->setZValue( 1 );
}

void ImageViewer::setLoadingMosaicAt(const GridPoint p)
{    
    double gx = m_image.width() / (double)m_gridResolution.width();
    double gy = m_image.height() / (double)m_gridResolution.height();
    QSizeF cellSize(gx, gy);


    QImage background(cellSize.width(), cellSize.height(), QImage::Format::Format_ARGB32);
    background.fill(QColor(128,0,0));
    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(background));
    item->setPos(QPointF(p.x() * cellSize.width(), p.y() * cellSize.height()));
    item->setOpacity(0.8);

    m_mosaicLoading->addToGroup(item);
}

void ImageViewer::setMosaicLoadingDone()
{
    qDebug() << "set moasic done";
    for(QGraphicsItem *p : m_mosaicLoading->childItems())
    {
        delete p;
    }
}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    m_moveOffset = event->pos();
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        float acceleration = 1.5;
        QPoint off = m_moveOffset - event->pos();
        m_moveOffset = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + off.x() * acceleration);
        verticalScrollBar()->setValue(verticalScrollBar()->value() + off.y() * acceleration);
    }
}

