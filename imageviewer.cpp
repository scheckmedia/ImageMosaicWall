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
}

void ImageViewer::setImage(QImage img)
{
    m_image = img;
    QGraphicsScene *scene = new QGraphicsScene(this);
    setScene(scene);
    QSize s = this->parentWidget()->size();
    scene->addPixmap(QPixmap::fromImage(m_image));    
    show();

    float sx = s.width() / (float)m_image.width();
    float sy = s.height() / (float)m_image.height();
    float ratio = qMin(sx, sy);
    scale(m_image.width() * ratio / m_image.width(), m_image.height() * ratio / m_image.height());
}

void ImageViewer::setGrid(QSize gridResolution)
{
    int gx = m_image.width() / gridResolution.width();
    int gy = m_image.height() / gridResolution.height();
    qDebug() << gx << " x " << gy;

    if(m_grid != nullptr)
    {
        scene()->removeItem(m_grid);
        for(QGraphicsItem* item : m_grid->childItems())
        {
            delete item;
        }

        delete m_grid;
        qDebug() << " delete group" << (m_grid == nullptr);
    }

    m_grid = new QGraphicsItemGroup();

    for(int x = 0; x < m_image.width(); x += gx)
    {
        QGraphicsLineItem* s = new QGraphicsLineItem(QLine(QPoint(x, 0), QPoint(x, m_image.height())));
        m_grid->addToGroup(s);
    }

    for(int y = 0; y < m_image.height(); y += gy)
    {
        QGraphicsLineItem* s = new QGraphicsLineItem(QLine(QPoint(0, y), QPoint(m_image.width(), y)));
        m_grid->addToGroup(s);
    }

    scene()->addItem(m_grid);
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

