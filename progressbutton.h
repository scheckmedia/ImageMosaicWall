#ifndef PROGRESSBUTTON_H
#define PROGRESSBUTTON_H

#include <QColor>
#include <QObject>
#include <QPushButton>

enum ProgressButtonType {
  PROGRESS_BOTTOM_LEFT_TO_RIGHT,
  PROGRESS_TOP_LEFT_TO_RIGHT,
  PROGRESS_LEFT_BOTTOM_TO_TOP,
  PROGRESS_RIGHT_BOTTOM_TO_TOP
};

class ProgressButton : public QPushButton {
  Q_OBJECT

  Q_PROPERTY(QColor progressColor READ progressColor WRITE setProgressColor
                 DESIGNABLE true)
  Q_PROPERTY(int progressLineWidth READ progressLineWidth WRITE
                 setProgressLineWidth DESIGNABLE true)

public slots:
  void updateProgress(int);
  void increment();

public:
  explicit ProgressButton(QWidget *parent);
  ~ProgressButton();

  QColor progressColor() const;
  void paintEvent(QPaintEvent *) override;
  void setProgressColor(QColor c);
  void setProgressLineWidth(int progressLineWidth);
  void setRange(int minimum, int maximum);
  void reset();
  int currentValue() const;
  int progressLineWidth() const;
  int minValue() const;
  int maxValue() const;

private:
  int m_currentValue;
  int m_minValue;
  int m_maxValue;
  int m_progressLineWidth;
  ProgressButtonType m_progressType;
  QColor m_progressColor;
};

#endif // PROGRESSBUTTON_H
