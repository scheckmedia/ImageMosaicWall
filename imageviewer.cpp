#include "imageviewer.h"
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QImageReader>
#include <QScrollBar>
#include <qmath.h>
#include "ui_imageviewer.h"

ImageViewer::ImageViewer(QWidget *parent)
    : QGraphicsView(parent)
    , ui(new Ui::ImageViewer)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    QGraphicsScene *scene = new QGraphicsScene(this);
    setScene(scene);

    m_image = new QGraphicsPixmapItem();
    m_grid = new QGraphicsItemGroup(m_image);
    m_mosaicLoading = new QGraphicsItemGroup(m_image);
    m_preview = new QGraphicsPixmapItem(m_image);
    scene->addItem(m_image);

    m_mosaicLoading->setZValue(10);

    connect(ui->btnZoomIn, &QPushButton::clicked, this, &ImageViewer::zoomIn);
    connect(ui->btnZoomOut, &QPushButton::clicked, this, &ImageViewer::zoomOut);
    connect(ui->btnHome, &QPushButton::clicked, [=]() { fitToScene(m_baseImage); });
}

ImageViewer::~ImageViewer()
{
    delete ui;
}

void ImageViewer::clearPreview()
{
    m_preview->setVisible(false);
    fitToScene(m_baseImage);
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    const QPointF p0scene = mapToScene(event->position().toPoint());
    qreal factor = qPow(1.2, event->angleDelta().y() / 240.0);

#else

    const QPointF p0scene = mapToScene(event->pos());
    qreal factor = qPow(1.2, event->delta() / 240.0);

#endif

    scale(factor, factor);

    const QPointF p1mouse = mapFromScene(p0scene);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    const QPointF move = p1mouse - event->position().toPoint(); // The move

#else

    const QPointF move = p1mouse - event->pos(); // The move

#endif

    horizontalScrollBar()->setValue(move.x() + horizontalScrollBar()->value());
    verticalScrollBar()->setValue(move.y() + verticalScrollBar()->value());
}

void ImageViewer::fitToScene(const QImage &img)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    resetTransform();

#else

    resetMatrix();

#endif

    QSize s = parentWidget()->size();
    setSceneRect(0.0, 0.0, img.width(), img.height());
    float sx = s.width() / (float)img.width();
    float sy = s.height() / (float)img.height();
    float ratio = qMin(sx, sy);
    scale(img.width() * ratio / img.width(), img.height() * ratio / img.height());
    show();
}

void ImageViewer::setImage(const QImage &img)
{
    setImage(img, img.size());
}

void ImageViewer::setImage(const QImage &img, QSize resolution)
{
    clearPreview();
    m_baseImage = img;
    m_image->setPixmap(QPixmap::fromImage(m_baseImage));

    fitToScene(img);
}

void ImageViewer::setPreview(const QImage &img)
{
    m_preview->setPixmap(QPixmap::fromImage(img));
    m_preview->setVisible(true);
    fitToScene(img);
}

void ImageViewer::setGrid(QSize gridResolution)
{
    clearPreview();
    m_gridResolution = gridResolution;

    double gx = ceil(m_baseImage.width() / static_cast<double>(gridResolution.width()));
    double gy = ceil(m_baseImage.height() / static_cast<double>(gridResolution.height()));

    if (m_grid != nullptr)
    {
        for (QGraphicsItem *item : m_grid->childItems())
        {
            delete item;
        }
    }

    for (int x = 0; x < m_baseImage.width(); x += gx)
    {
        QGraphicsLineItem *s = new QGraphicsLineItem(QLine(QPoint(x, 0), QPoint(x, m_baseImage.height())));
        m_grid->addToGroup(s);
    }

    for (int y = 0; y < m_baseImage.height(); y += gy)
    {
        QGraphicsLineItem *s = new QGraphicsLineItem(QLine(QPoint(0, y), QPoint(m_baseImage.width(), y)));
        m_grid->addToGroup(s);
    }
}

void ImageViewer::setLoadingMosaicAt(const GridPoint p)
{
    double gx = ceil(m_baseImage.width() / static_cast<double>(m_gridResolution.width()));
    double gy = ceil(m_baseImage.height() / static_cast<double>(m_gridResolution.height()));
    QSize cellSize(gx, gy);

    auto target = QPointF(QPoint(p.x() * cellSize.width(), p.y() * cellSize.height()));
    if (target.x() >= m_baseImage.width() || target.y() >= m_baseImage.height())
        return;
    QImage background(cellSize.width(), cellSize.height(), QImage::Format::Format_ARGB32);
    background.fill(QColor(128, 0, 0));
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(QPixmap::fromImage(background));
    item->setPos(target);
    item->setOpacity(0.8);

    m_mosaicLoading->addToGroup(item);
}

void ImageViewer::zoomIn()
{
    double delta = 0.10;
    double t = transform().m11() + delta;
    double zoomLevel = delta * floor(t / delta + 0.5);

    setTransform(QTransform(zoomLevel, 0, 0, 0, zoomLevel, 0, 0, 0, 1));
}

void ImageViewer::zoomOut()
{
    double delta = 0.10;
    double t = transform().m11() - delta;
    double zoomLevel = (delta * floor(t / delta + 0.5));

    if (zoomLevel > 0)
    {
        setTransform(QTransform(zoomLevel, 0, 0, 0, zoomLevel, 0, 0, 0, 1));
    }
}

void ImageViewer::setMosaicLoadingDone()
{
    for (QGraphicsItem *p : m_mosaicLoading->childItems())
    {
        delete p;
    }
}

void ImageViewer::resizeEvent(QResizeEvent *)
{
    QSize s = ui->wControls->size();
    ui->wControls->move(this->width() - s.width() - 10, this->height() - s.height() - 10);
}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    m_moveOffset = event->pos();
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        float acceleration = 1.5;
        QPoint off = m_moveOffset - event->pos();
        m_moveOffset = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + off.x() * acceleration);
        verticalScrollBar()->setValue(verticalScrollBar()->value() + off.y() * acceleration);
    }
}

void ImageViewer::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        fitToScene(m_baseImage);
}

void ImageViewer::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

void ImageViewer::dragMoveEvent(QDragMoveEvent * /*event*/) { }

void ImageViewer::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls())
    {
        QFileInfo info(mimeData->urls().first().path());

        if (info.isDir())
            emit folderDropped(info.absoluteFilePath());
        else if (info.isFile() && QImageReader::imageFormat(info.absoluteFilePath()).isEmpty() == false)
            emit imageDropped(info.absoluteFilePath());
    }
}
