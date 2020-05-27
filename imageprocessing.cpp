#include "imageprocessing.h"
#include <QDebug>
#include <QImage>
#include <QImageReader>
#include <exiv2/exiv2.hpp>

ImageProcessing::ImageProcessing(QObject *parent) : QObject(parent) {}

ImageProcessing::~ImageProcessing() {}

void ImageProcessing::processMosaicImages(const QList<QString> &imageList) {
  m_imageMeanMap.clear();
  calculateImageMeanMap(imageList);
}

bool ImageProcessing::generateImage(QSize outputSize, QSize gridSize,
                                    int history) {
  if (!isReady())
    return false;

  m_outputSize = outputSize;
  m_historySize = history;

  int gx = m_outputSize.width() / (gridSize.width() - 1);
  int gy = m_outputSize.height() / (gridSize.height() - 1);

  QSize gridCellSize(gx, gy);

  int maxThreads = QThreadPool::globalInstance()->maxThreadCount();
  int numCells = gridSize.width() * gridSize.height();
  int itemsPerThread = numCells / maxThreads;
  QThreadPool pool;

  m_outputImage =
      std::unique_ptr<QImage>(new QImage(outputSize, QImage::Format_ARGB32));

  int numCellsPerThread = itemsPerThread;
  for (int t = 0; t < maxThreads; ++t) {
    if (t == (maxThreads - 1)) {
      numCellsPerThread += numCells % maxThreads;
    }

    QtConcurrent::run(&pool, this, &ImageProcessing::calculateMosaicPositions,
                      gridCellSize, gridSize, t * numCellsPerThread,
                      numCellsPerThread);
  }

  pool.waitForDone();
  return true;
}

void ImageProcessing::calculateGridCellsMean(const QImage &baseImage,
                                             const QSize &gridCellSize,
                                             const QSize &gridSize,
                                             std::vector<QColor> &colorMap,
                                             int pos, int numCells) {
  int nx = 0, ny = 0, g = pos, gx = 0, gy = 0;
  int numberOfPixel = gridCellSize.width() * gridCellSize.height();

  for (; g < pos + numCells; ++g) {
    int red = 0, green = 0, blue = 0;
    gx = (g % gridSize.width()) * gridCellSize.width();
    gy = g / gridSize.width() * gridCellSize.height();

    for (int y = 0; y < gridCellSize.height(); ++y) {
      ny = y + gy;
      if (ny >= baseImage.height() || ny < 0)
        continue;

      for (int x = 0; x < gridCellSize.width(); ++x) {
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

void ImageProcessing::calculateImageMeanMap(const QList<QString> &imageList) {
  QMutex mutex;
  std::function<void(const QString)> scale = [&](const QString imageFileName) {
    QImage image = extractThumbnail(imageFileName, QSize(128, 128));
    if (image.isNull())
      image = QImage(imageFileName);

    int meanR = 0;
    int meanG = 0;
    int meanB = 0;

    // QColor color(image.scaled(QSize(1, 1), Qt::IgnoreAspectRatio,
    // Qt::FastTransformation).pixel(0,0));
    for (int y = 0; y < image.height(); ++y) {
      for (int x = 0; x < image.width(); ++x) {
        QColor c = image.pixelColor(QPoint(x, y));
        meanR += c.red();
        meanG += c.green();
        meanB += c.blue();
      }
    }

    meanR /= (image.width() * image.height());
    meanG /= (image.width() * image.height());
    meanB /= (image.width() * image.height());

    QMutexLocker lock(&mutex);
    m_imageMeanMap.insert(imageFileName, QColor(meanR, meanG, meanB));
    emit imageProcessed(imageFileName);
  };

  QtConcurrent::map(imageList, scale).waitForFinished();
}

void ImageProcessing::calculateMosaicPositions(const QSize cellSize,
                                               const QSize gridSize,
                                               const int pos,
                                               const int length) {
  QVector<QString> history;
  int g = pos;
  m_gridMapCache.clear();

  for (; g < pos + length; ++g) {
    history.clear();
    double bestDistance = INT_MAX;
    QString imagePath;
    QColor cellMean = m_gridColorMap.at(g);
    ColorLab cellMeanLab = toLab(cellMean);

    GridPoint p;
    p.setX(g % gridSize.width());
    p.setY(g / gridSize.width());

    for (int kx = -m_historySize; kx <= m_historySize; ++kx) {
      for (int ky = -m_historySize; ky <= m_historySize; ++ky) {
        GridPoint pDelta(p.x() + kx, p.y() + ky);

        if (pDelta.x() < 0 || pDelta.x() > gridSize.width() - 1)
          continue;

        if (pDelta.y() < 0 || pDelta.y() > gridSize.height() - 1)
          continue;

        if (m_gridMapCache.contains(pDelta))
          history.push_back(m_gridMapCache.value(pDelta));
      }
    }

    for (auto &image : m_imageMeanMap.keys()) {
      QColor c = m_imageMeanMap.value(image);
      ColorLab imageMean = toLab(c);

      double dist = calculateDistance(cellMeanLab, imageMean);

      if (dist < bestDistance && history.contains(image) == false) {
        imagePath = image;
        bestDistance = dist;
      }
    }

    QImage image = extractThumbnail(imagePath, cellSize);
    if (image.isNull())
      image = QImage(imagePath);

    QImage cellImage =
        image
            .scaled(cellSize + QSize(2, 2), Qt::KeepAspectRatioByExpanding,
                    Qt::SmoothTransformation)
            .copy(QRect(QPoint(0, 0), cellSize + QSize(2, 2)));

    QMutexLocker lock(&m_lockMean);
    m_gridMapCache.insert(p, imagePath);
    lock.unlock();
    emit mosaicGenerated(p);

    double gx = m_outputImage.get()->width() / (double)gridSize.width();
    double gy = m_outputImage.get()->height() / (double)gridSize.height();
    int nx = 0;
    int ny = 0;

    for (int y = 0; y < cellImage.height(); ++y) {
      ny = round(p.y() * gy + y);
      if (ny < 0 || ny >= m_outputImage->height())
        continue;

      for (int x = 0; x < cellImage.width(); ++x) {
        nx = round(p.x() * gx + x);
        if (nx < 0 || nx >= m_outputImage->width())
          continue;

        auto const pixel = cellImage.pixel(x, y);
        m_outputImage.get()->setPixel(nx, ny, pixel);
      }
    }
  }
}

double ImageProcessing::calculateDistance(QColor rhs, QColor lhs) const {
  return abs(rhs.redF() - lhs.redF()) + abs(rhs.greenF() - lhs.greenF()) +
         abs(rhs.blueF() - lhs.blueF());
}

double ImageProcessing::calculateDistance(ColorLab rhs, ColorLab lhs) const {
  return abs(rhs.L - lhs.L) + abs(rhs.a - lhs.a) + abs(rhs.b - lhs.b);
}

QMap<QString, QColor> ImageProcessing::getImageMeanMap() const {
  return m_imageMeanMap;
}

const QImage &ImageProcessing::getOutputImage() const {
  return *m_outputImage.get();
}

void ImageProcessing::processGrid(const QImage &baseImage, QSize gridSize) {
  int maxThreads = QThreadPool::globalInstance()->maxThreadCount();
  int numCells = gridSize.width() * gridSize.height();
  int itemsPerThread = numCells / maxThreads;
  QThreadPool pool;

  m_gridColorMap.resize(numCells);

  int gx = baseImage.width() / (gridSize.width() - 1);
  int gy = baseImage.height() / (gridSize.height() - 1);

  QSize gridCellSize(gx, gy);

  int numCellsPerThread = itemsPerThread;
  for (int t = 0; t < maxThreads; ++t) {
    if (t == (maxThreads - 1)) {
      numCellsPerThread += numCells % maxThreads;
    }

    std::function<void()> meanFnc = [=]() {
      this->calculateGridCellsMean(baseImage, gridCellSize, gridSize,
                                   m_gridColorMap, t * itemsPerThread,
                                   numCellsPerThread);
    };

    QtConcurrent::run(&pool, meanFnc);
  }

  pool.waitForDone();
}

std::vector<QColor> ImageProcessing::getGridColorMap() const {
  return m_gridColorMap;
}

bool ImageProcessing::isReady() {
  return !(m_gridColorMap.size() == 0 || m_imageMeanMap.size() == 0);
}

QImage extractThumbnail(const QString &filename, QSize minSize) {
  Exiv2::Image::AutoPtr image =
      Exiv2::ImageFactory::open(filename.toStdString());
  image->readMetadata();

  Exiv2::PreviewManager loader(*image);
  Exiv2::PreviewPropertiesList list = loader.getPreviewProperties();

  if (list.empty())
    return QImage();

  QSize bestSize(-1, -1);

  Exiv2::PreviewProperties previewProperty;
  for (const auto &previewItem : list) {
    if (previewItem.width_ < minSize.width() ||
        previewItem.height_ < minSize.height())
      continue;

    if (previewItem.width_ < bestSize.width() ||
        previewItem.height_ < bestSize.height()) {
      previewProperty = previewItem;
      bestSize.setWidth(previewItem.width_);
      bestSize.setHeight(previewItem.height_);
    }
  }

  Exiv2::PreviewImage preview = loader.getPreviewImage(previewProperty);
  return QImage::fromData(preview.pData(), preview.size());
}
