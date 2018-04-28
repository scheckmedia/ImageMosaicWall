#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QMap>
#include <QColor>

class ImageProcessing : public QObject
{

   Q_OBJECT

private:
   void calculateGridCellsMean(const QImage&, const QSize&, const QSize&, std::vector<QColor>&, int, int);
   void calculateImageMeanMap(const QList<QString>&);
   void mapImageForMean(const QSize, const QSize, QMap<QPoint,QImage>*, const int, const int);
   double calculateDistance(QColor rhs, QColor lhs) const;

private:
   std::vector<QColor> m_gridColorMap;
   QMap<QString, QColor> m_imageMeanMap;

public:
    explicit ImageProcessing(QObject *parent = 0);
    ~ImageProcessing();

    void processGrid(const QImage&, QSize);
    void processMosaicImages(const QList<QString>&);
    bool generateImage(QSize, QSize, QMap<QPoint, QImage>*);

public:
    std::vector<QColor> getGridColorMap() const;
    bool isReady();
};

#endif // IMAGEPROCESSOR_H
