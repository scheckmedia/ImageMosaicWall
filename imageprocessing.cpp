#include "imageprocessing.h"
#include <QtConcurrent>
#include <QDebug>
#include <QImage>

ImageProcessing::ImageProcessing(QObject *parent)
    : QObject(parent)
{

}

ImageProcessing::~ImageProcessing()
{

}

void ImageProcessing::processMosaicImages(const QList<QString> &imageList)
{
    m_imageMeanMap.clear();
    calculateImageMeanMap(imageList);
}

bool ImageProcessing::generateImage(QSize outputSize, QSize gridSize, QMap<QPoint,QImage>* dst)
{
    if(!isReady())
        return false;

    QSize gridCellSize(outputSize.width() / gridSize.width(), outputSize.height() / gridSize.height());
    int maxThreads = QThreadPool::globalInstance()->maxThreadCount();
    int numCells = gridSize.width() * gridSize.height();
    int itemsPerThread = numCells / maxThreads;
    QThreadPool pool;


    int numCellsPerThread = itemsPerThread;
    for(int t = 0; t < maxThreads; ++t)
    {
        if ((numCells % maxThreads) > 0 && t == (maxThreads - 1))
        {
            numCellsPerThread += numCells % maxThreads;
        }

        QtConcurrent::run(&pool, this, &ImageProcessing::mapImageForMean, gridCellSize, gridSize, dst, t * numCells, numCells);
    }

    pool.waitForDone();

    return true;
}


void ImageProcessing::calculateGridCellsMean(const QImage &baseImage, const QSize &gridCellSize, const QSize &gridSize, std::vector<QColor> &colorMap, int pos, int numCells)
{
    int nx = 0, ny = 0, g = pos, gx = 0, gy = 0;
    int numberOfPixel = gridCellSize.width() * gridCellSize.height();

    for(; g < pos + numCells; ++g)
    {
        int red = 0, green = 0, blue = 0;
        gx = (g % gridSize.width()) * gridCellSize.width();
        gy = g / gridSize.width() * gridCellSize.height();

        for(int y = 0; y < gridCellSize.height(); ++y)
        {
            for(int x = 0; x < gridCellSize.width(); ++x)
            {
                nx = x + gx;
                ny = y + gy;

                if(nx < 0 || nx >= baseImage.width() || ny >= baseImage.height() || ny < 0)
                    continue;

                QColor c = baseImage.pixelColor(nx, ny);
                red   += c.red();
                green += c.green();
                blue  += c.blue();
            }
        }

        red /= numberOfPixel;
        green /= numberOfPixel;
        blue /= numberOfPixel;

        colorMap.at(g) = QColor(red, green, blue);
    }
}

void ImageProcessing::calculateImageMeanMap(const QList<QString> &imageList)
{
    QMutex mutex;
    std::function<void(const QString)> scale = [&](const QString imageFileName) {
        QImage image(imageFileName);
        QColor color(image.scaled(QSize(1, 1), Qt::IgnoreAspectRatio, Qt::FastTransformation).pixel(0,0));

        QMutexLocker lock(&mutex);
        m_imageMeanMap.insert(imageFileName, color);
    };


    QtConcurrent::map(imageList, scale).waitForFinished();
}

void ImageProcessing::mapImageForMean(const QSize cellSize, const QSize gridSize, QMap<QPoint,QImage>*, const int pos, const int length)
{
    int g = pos, x = 0, y = 0;
    for(; g < pos + length; ++g)
    {
        x = (g % gridSize.width()) * cellSize.width();
        y = g / gridSize.width() * cellSize.height();

        int dist = -1;
        for(auto &image : m_imageMeanMap.keys())
        {
            QColor c = m_imageMeanMap.value(c);
        }
    }
}

double ImageProcessing::calculateDistance(QColor rhs, QColor lhs) const
{

}

void ImageProcessing::processGrid(const QImage &baseImage, QSize gridSize)
{
    int maxThreads = QThreadPool::globalInstance()->maxThreadCount();
    int numCells = gridSize.width() * gridSize.height();
    int itemsPerThread = numCells / maxThreads;
    QThreadPool pool;

    m_gridColorMap.resize(numCells);

    int gx = baseImage.width() / gridSize.width();
    int gy = baseImage.height() / gridSize.height();

    QSize gridCellSize(gx, gy);

    int numCellsPerThread = itemsPerThread;
    for(int t = 0; t < maxThreads; ++t)
    {
        if ((numCells % maxThreads) > 0 && t == (maxThreads - 1))
        {
            numCellsPerThread += numCells % maxThreads;
        }

        std::function<void()>
                meanFnc = [=](){
            this->calculateGridCellsMean(baseImage, gridCellSize, gridSize, m_gridColorMap, t * numCells, numCells);
        };

        QtConcurrent::run(&pool, meanFnc);
    }

    pool.waitForDone();


    /*
     * DEBUG Code
    qDebug() << "Size: " << m_gridColorMap.size();
    QString filename = "/Users/tobi/Desktop/test.bmp";
    QImage testImage(gridSize, QImage::Format_ARGB32);

    for(int y = 0; y < gridSize.height(); ++y)
    {
        for(int x = 0; x < gridSize.width(); ++x)
        {
            QColor c = colorMap->at(y * gridSize.width() + x);
            testImage.setPixelColor(x, y, c);
        }
    }

    qDebug() << "save state: " << testImage.save(filename);

    QImage test2(baseImage);
    test2.scaled(gridSize,  Qt::IgnoreAspectRatio, Qt::SmoothTransformation).save("/Users/tobi/Desktop/test2.bmp");
    */
}

std::vector<QColor> ImageProcessing::getGridColorMap() const
{
    return m_gridColorMap;
}

bool ImageProcessing::isReady()
{
    return !(m_gridColorMap.size() == 0 || m_imageMeanMap.size() == 0);
}
