#include "imageprocessing.h"
#include <algorithm>
#include <cstdlib>
#include <QDebug>
#include <QImage>
#include <QImageReader>
#include <exiv2/exiv2.hpp>
#include <exiv2/exv_conf.h>
#include <exiv2/version.hpp>

#ifndef EXIV2_TEST_VERSION
#    define EXIV2_TEST_VERSION(major,minor,patch) \
         ( EXIV2_VERSION >= EXIV2_MAKE_VERSION(major,minor,patch) )
#endif

#if EXIV2_TEST_VERSION(0,27,99)
#   define AutoPtr UniquePtr
#   define AnyError Error
#   define kerErrorMessage ErrorCode::kerErrorMessage
#endif

template<class RandomIt>
void my_random_shuffle(RandomIt first, RandomIt last)
{
    typedef typename std::iterator_traits<RandomIt>::difference_type diff_t;

    for (diff_t i = last - first - 1; i > 0; --i)
    {
        using std::swap;
        swap(first[i], first[std::rand() % (i + 1)]);
    }
}

ImageProcessing::ImageProcessing(QObject *parent)
    : QObject(parent)
    , m_skipBackgroundProcess(false)
    , m_imageMeanMap()
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
}

ImageProcessing::~ImageProcessing()
{
    m_skipBackgroundProcess = true;
}

void ImageProcessing::processMosaicImages(const QList<QString> &imageList)
{
    m_imageMeanMap.clear();
    calculateImageMeanMap(imageList);
}

bool ImageProcessing::generateImage(QSize outputSize, QSize gridSize, int history)
{
    if (!isReady())
        return false;

    m_outputSize = outputSize;
    m_historySize = history;

    int maxThreads = QThreadPool::globalInstance()->maxThreadCount();
    int numCells = gridSize.width() * gridSize.height();
    int itemsPerThread = numCells / maxThreads;
    QThreadPool pool;

    m_outputImage = std::unique_ptr<QImage>(new QImage(outputSize, QImage::Format_ARGB32));

    int numCellsPerThread = itemsPerThread;
    for (int t = 0; t < maxThreads; ++t)
    {
        if (t == (maxThreads - 1))
        {
            numCellsPerThread += numCells % maxThreads;
        }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        QtConcurrent::run(
            &pool, &ImageProcessing::calculateMosaicPositions, this, gridSize, t * itemsPerThread, numCellsPerThread);

#else

        QtConcurrent::run(
            &pool, this, &ImageProcessing::calculateMosaicPositions, gridSize, t * itemsPerThread, numCellsPerThread);

#endif

    }

    pool.waitForDone();
    return true;
}

void ImageProcessing::calculateGridCellsMean(const QImage &baseImage,
                                             const QSize &gridSize,
                                             std::vector<QColor> &colorMap,
                                             int pos,
                                             int numCells)
{
    int nx = 0, ny = 0, g = pos;
    double gx = ceil(baseImage.width() / static_cast<float>(gridSize.width()));
    double gy = ceil(baseImage.height() / static_cast<float>(gridSize.height()));

    QSize gridCellSize(floor(gx), floor(gy));

    int numberOfPixel = gridCellSize.width() * gridCellSize.height();

    for (; g < pos + numCells; ++g)
    {
        int red = 0, green = 0, blue = 0;
        gx = (g % gridSize.width()) * gridCellSize.width();
        gy = g / gridSize.width() * gridCellSize.height();

        for (int y = 0; y < gridCellSize.height(); ++y)
        {
            ny = y + gy;
            if (ny >= baseImage.height() || ny < 0)
                continue;

            for (int x = 0; x < gridCellSize.width(); ++x)
            {
                nx = x + gx;

                if (nx < 0 || nx >= baseImage.width())
                    continue;

                QColor c = baseImage.pixelColor(nx, ny);
                red += c.red();
                green += c.green();
                blue += c.blue();
            }
        }

        red /= numberOfPixel;
        green /= numberOfPixel;
        blue /= numberOfPixel;

        colorMap.at(g) = QColor(red, green, blue);
        emit cellProcessed(g);
    }
}

void ImageProcessing::calculateImageMeanMap(const QList<QString> imageList)
{
    std::function<void(const QString)> scale = [&](const QString imageFileName) {
        if (m_skipBackgroundProcess)
            return;

        QImage image = extractThumbnail(imageFileName, QSize(128, 128));
        if (image.isNull())
            image = QImage(imageFileName).scaled(QSize(128, 128), Qt::IgnoreAspectRatio, Qt::FastTransformation);

        if (image.isNull())
            return;

        uint64_t meanR = 0;
        uint64_t meanG = 0;
        uint64_t meanB = 0;

        // QColor color(image.scaled(QSize(1, 1), Qt::IgnoreAspectRatio,
        // Qt::FastTransformation).pixel(0,0));
        for (int y = 0; y < image.height(); ++y)
        {
            for (int x = 0; x < image.width(); ++x)
            {
                QColor c = image.pixelColor(QPoint(x, y));
                meanR += c.red();
                meanG += c.green();
                meanB += c.blue();
            }
        }

        auto dim = image.width() * image.height();
        meanR /= dim;
        meanG /= dim;
        meanB /= dim;

        if (m_skipBackgroundProcess)
            return;
        {
            QMutexLocker lock(&m_lockMean);
            auto color = QColor(meanR, meanG, meanB);
            m_imageMeanMap.insert(imageFileName, color);
        }
        emit imageProcessed(imageFileName);
    };

    QtConcurrent::map(imageList, scale).waitForFinished();
}

void ImageProcessing::calculateMosaicPositions(const QSize gridSize, const int startPos, const int length)
{
    QVector<QString> history;
    m_gridMapCache.clear();

    double gx = ceil(m_outputSize.width() / static_cast<float>(gridSize.width()));
    double gy = ceil(m_outputSize.height() / static_cast<float>(gridSize.height()));
    QSize cellSize(gx, gy);

    std::vector<uint32_t> positions;
    for (uint32_t i = startPos; i < startPos + length; ++i)
        positions.push_back(i);

    my_random_shuffle(positions.begin(), positions.end());
    for (auto pos : positions)
    {
        if (m_skipBackgroundProcess)
        {
            qDebug() << "skip because of cancel";
            return;
        }

        history.clear();
        double bestDistance = INT_MAX;
        QString imagePath;
        QColor cellMean = m_gridColorMap.at(pos);
        ColorLab cellMeanLab = toLab(cellMean);

        GridPoint p;
        p.setX(pos % gridSize.width());
        p.setY(pos / gridSize.width());

        for (int kx = -m_historySize; kx <= m_historySize; ++kx)
        {
            for (int ky = -m_historySize; ky <= m_historySize; ++ky)
            {
                GridPoint pDelta(p.x() + kx, p.y() + ky);

                if (pDelta.x() < 0 || pDelta.x() > gridSize.width() - 1)
                    continue;

                if (pDelta.y() < 0 || pDelta.y() > gridSize.height() - 1)
                    continue;

                if (m_gridMapCache.contains(pDelta))
                    history.push_back(m_gridMapCache.value(pDelta));
            }
        }

        // brute force knn should be replaced by kd-tree
        // this is the bottleneck with ~30 ms per cell
        for (auto &image : m_imageMeanMap.keys())
        {
            QColor c = m_imageMeanMap.value(image);
            ColorLab imageMean = toLab(c);

            double dist = calculateDistance(cellMeanLab, imageMean);

            if (dist < bestDistance && history.contains(image) == false)
            {
                imagePath = image;
                bestDistance = dist;
            }
        }

        QImage image = extractThumbnail(imagePath, cellSize);
        if (image.isNull())
            image = QImage(imagePath);

        QImage cellImage = image.scaled(cellSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)
                               .copy(QRect(QPoint(0, 0), cellSize));

        QMutexLocker lock(&m_lockMean);
        m_gridMapCache.insert(p, imagePath);
        lock.unlock();
        emit mosaicGenerated(p);

        int nx = 0;
        int ny = 0;

        for (int y = 0; y < cellSize.height(); ++y)
        {
            ny = p.y() * gy + y;
            if (ny < 0 || ny >= m_outputImage->height())
                continue;

            for (int x = 0; x < cellSize.width(); ++x)
            {
                nx = p.x() * gx + x;
                if (nx < 0 || nx >= m_outputImage->width())
                    continue;

                auto const pixel = cellImage.pixel(x, y);
                m_outputImage.get()->setPixel(nx, ny, pixel);
            }
        }
    }
}

double ImageProcessing::calculateDistance(QColor rhs, QColor lhs) const
{
    return abs(rhs.redF() - lhs.redF()) + abs(rhs.greenF() - lhs.greenF()) + abs(rhs.blueF() - lhs.blueF());
}

double ImageProcessing::calculateDistance(ColorLab rhs, ColorLab lhs) const
{
    return abs(rhs.L - lhs.L) + abs(rhs.a - lhs.a) + abs(rhs.b - lhs.b);
}

QMap<QString, QColor> ImageProcessing::getImageMeanMap() const
{
    return m_imageMeanMap;
}

bool ImageProcessing::skipBackgroundProcesses() const
{
    return m_skipBackgroundProcess;
}

void ImageProcessing::processCanceled(bool canceled)
{
    m_skipBackgroundProcess = canceled;
}

const QImage *ImageProcessing::getOutputImage() const
{
    return m_outputImage.get();
}

void ImageProcessing::processGrid(const QImage &baseImage, QSize gridSize)
{
    int maxThreads = QThreadPool::globalInstance()->maxThreadCount();
    int numCells = gridSize.width() * gridSize.height();
    int itemsPerThread = numCells / maxThreads;
    QThreadPool pool;

    m_gridColorMap.resize(numCells);

    int numCellsPerThread = itemsPerThread;
    for (int t = 0; t < maxThreads; ++t)
    {
        if (t == (maxThreads - 1))
        {
            numCellsPerThread += numCells % maxThreads;
        }

        std::function<void()> meanFnc = [=]() {
            this->calculateGridCellsMean(baseImage, gridSize, m_gridColorMap, t * itemsPerThread, numCellsPerThread);
        };

        QtConcurrent::run(&pool, meanFnc);
    }

    pool.waitForDone();
}

std::vector<QColor> ImageProcessing::getGridColorMap() const
{
    return m_gridColorMap;
}

bool ImageProcessing::isReady()
{
    return !(m_gridColorMap.size() == 0 || m_imageMeanMap.size() == 0);
}

QImage extractThumbnail(const QString filename, QSize minSize)
{
    try
    {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filename.toStdString());

        if (image.get() == nullptr || !image.get()->good())
            return QImage();

        image->readMetadata();

        Exiv2::PreviewManager loader(*image);
        Exiv2::PreviewPropertiesList list = loader.getPreviewProperties();

        if (list.empty())
            return QImage();

        QSize bestSize(-1, -1);

        Exiv2::PreviewProperties previewProperty;
        for (const auto &previewItem : list)
        {
            if (previewItem.width_ < minSize.width() || previewItem.height_ < minSize.height())
                continue;

            if (previewItem.width_ < bestSize.width() || previewItem.height_ < bestSize.height())
            {
                previewProperty = previewItem;
                bestSize.setWidth(previewItem.width_);
                bestSize.setHeight(previewItem.height_);
            }
        }

        Exiv2::PreviewImage preview = loader.getPreviewImage(previewProperty);
        return QImage::fromData(preview.pData(), preview.size());
    }
    catch (Exiv2::Error &err)
    {
        qDebug() << "Error: " << err.what();
        return QImage();
    }
}
