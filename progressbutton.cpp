#include "progressbutton.h"
#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QCoreApplication>

ProgressButton::ProgressButton(QWidget *parent)
    : QPushButton(parent)    
    , m_currentValue(0.0)
    , m_minValue(0.0)
    , m_maxValue(100.0)    
    , m_progressType(PROGRESS_BOTTOM_LEFT_TO_RIGHT)
    , m_progressColor(QColor(255, 0, 0))
{
    setObjectName("ProgressButton");
}

ProgressButton::~ProgressButton()
{}



void ProgressButton::paintEvent(QPaintEvent *e)
{
    QPushButton::paintEvent(e);
    ensurePolished();
    QPainter p(this);    

    double scale = m_currentValue / (double)(m_maxValue - m_minValue);    
    QRect rect = e->rect();
    QRect r;

    if(scale >= 1)
    {
        p.end();
        return;
    }

    switch (m_progressType) {
        case PROGRESS_BOTTOM_LEFT_TO_RIGHT:
            r = QRect(0, rect.height() - progressLineWidth(), rect.width() * scale, progressLineWidth());
            break;
        case PROGRESS_TOP_LEFT_TO_RIGHT:
            r = QRect(0, 0, rect.width() * scale, progressLineWidth());
            break;
        case PROGRESS_LEFT_BOTTOM_TO_TOP:
            r = QRect(0, rect.height() - rect.height() * scale, progressLineWidth(), rect.height());
            break;
        case PROGRESS_RIGHT_BOTTOM_TO_TOP:
            r = QRect(rect.width() - progressLineWidth(), rect.height() - rect.height() * scale, rect.width(), rect.height());
            break;
        default:
            break;
    }

    p.fillRect(r, progressColor());
    p.end();

}

QColor ProgressButton::progressColor() const
{
    return m_progressColor;
}

void ProgressButton::setProgressColor(QColor c)
{
    m_progressColor = c;
}

int ProgressButton::currentValue() const
{
    return m_currentValue;
}

int ProgressButton::progressLineWidth() const
{
    return m_progressLineWidth;
}

void ProgressButton::setProgressLineWidth(int progressLineWidth)
{
    m_progressLineWidth = progressLineWidth;
}

void ProgressButton::setRange(int minimum, int maximum)
{
    Q_ASSERT(minimum < maximum);
    m_minValue = minimum;
    m_maxValue = maximum;
    reset();
}

void ProgressButton::reset()
{
    m_currentValue = m_minValue;
    update();
}

void ProgressButton::updateProgress(int val)
{    
    m_currentValue = val;
    qDebug() << "min: " << m_minValue << " max: " << m_maxValue << " current: " << m_currentValue;
    update();
}

void ProgressButton::increment()
{
    updateProgress(m_currentValue + 1);
}
