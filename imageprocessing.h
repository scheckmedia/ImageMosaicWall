#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QColor>
#include <QMap>
#include <QMetaType>
#include <QObject>
#include <QPainter>
#include <QPoint>
#include <QtConcurrent>
#include <math.h>
#include <memory>

struct ColorXYZ {
  double X, Y, Z;
};

struct ColorLab {
  double L, a, b;
};

struct GridPoint : public QPoint {
  GridPoint() : QPoint(){};
  GridPoint(int x, int y) : QPoint(x, y){};
  bool operator<(const GridPoint &rhs) const {
    if (this->x() == rhs.x())
      return this->y() < rhs.y();
    else
      return this->x() < rhs.x();
  }

  bool operator==(const GridPoint &other) const {
    return this->x() == other.x() && this->y() == other.y();
  }
};

Q_DECLARE_METATYPE(GridPoint)

class ImageProcessing : public QObject {

  Q_OBJECT

public:
  explicit ImageProcessing(QObject *parent = 0);
  ~ImageProcessing();

  void processGrid(const QImage &, QSize);
  void processMosaicImages(const QList<QString> &);
  bool generateImage(QSize, QSize, int);
  bool isReady();

signals:
  void mosaicGenerated(const GridPoint);
  void cellProcessed(const int cellIndex);
  void imageProcessed(const QString image);

private:
  void calculateGridCellsMean(const QImage &, const QSize &, const QSize &,
                              std::vector<QColor> &, int, int);
  void calculateImageMeanMap(const QList<QString> &);
  void calculateMosaicPositions(const QSize, const QSize, const int, const int);
  double calculateDistance(QColor rhs, QColor lhs) const;
  double calculateDistance(ColorLab rhs, ColorLab lhs) const;

public:
  std::vector<QColor> getGridColorMap() const;
  const QImage &getOutputImage() const;
  QMap<QString, QColor> getImageMeanMap() const;

  bool getImportFolderCanceled() const;
  void processCanceled(bool canceled);

private:
  std::vector<QColor> m_gridColorMap;
  QMap<QString, QColor> m_imageMeanMap;
  QMap<GridPoint, QString> m_gridMapCache;
  QMutex m_lockMean;
  QSize m_outputSize;
  std::unique_ptr<QImage> m_outputImage;
  std::atomic<bool> m_importFolderCanceled;
  int m_historySize;
};

/**
 * @brief Converts q QColor (RGB) to XYZ
 * @see http://www.easyrgb.com/en/math.php
 * @param color RGB Color
 * @return XYZ Color
 */
inline ColorXYZ toXYZ(QColor &color) {
  double r = color.redF();
  double g = color.greenF();
  double b = color.blueF();

  if (r > 0.04045)
    r = pow(((r + 0.055) / 1.055), 2.4);
  else
    r /= 12.92;

  if (g > 0.04045)
    g = pow(((g + 0.055) / 1.055), 2.4);
  else
    g /= 12.92;

  if (b > 0.04045)
    b = pow(((b + 0.055) / 1.055), 2.4);
  else
    b /= 12.92;

  r *= 100.0;
  g *= 100.0;
  b *= 100.0;

  ColorXYZ xyz;

  xyz.X = r * 0.4124 + g * 0.3576 + b * 0.1805;
  xyz.Y = r * 0.2126 + g * 0.7152 + b * 0.0722;
  xyz.Z = r * 0.0193 + g * 0.1192 + b * 0.9505;

  return xyz;
}

/**
 * @brief Converts QColor (RGB) to L*ab
 * The reference white point is D65
 * @see http://www.easyrgb.com/en/math.php
 * @param color RGB Color
 * @return L*ab Color
 */
inline ColorLab toLab(QColor &color) {
  // d65 white point

  ColorXYZ xyz = toXYZ(color);

  double x = xyz.X / 95.047;
  double y = xyz.Y / 100.0;
  double z = xyz.Z / 108.883;

  if (x > 0.008856)
    x = pow(x, 1.0 / 3.0);
  else
    x = (7.787 * x) + (16.0 / 116.0);

  if (y > 0.008856)
    y = pow(y, 1.0 / 3.0);
  else
    y = (7.787 * y) + (16.0 / 116.0);

  if (z > 0.008856)
    z = pow(z, 1.0 / 3.0);
  else
    z = (7.787 * z) + (16.0 / 116.0);

  ColorLab lab;
  lab.L = (116.0 * y) - 16.0;
  lab.a = 500.0 * (x - y);
  lab.b = 200.0 * (x - z);

  return lab;
}

QImage extractThumbnail(const QString &, QSize);

#endif // IMAGEPROCESSOR_H
