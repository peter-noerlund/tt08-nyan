#include "monitor.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_4_Compatibility>
#include <QApplication>
#include <QPaintEvent>
#include <QRect>

namespace tt08
{

Monitor::Monitor(QWidget* parent)
    : QOpenGLWidget(parent)
{
}

void Monitor::initializeGL()
{
    context()->functions()->glClearColor(0, 0, 0, 1);
    QOpenGLWidget::initializeGL();
}

void Monitor::paintGL()
{
    context()->versionFunctions<QOpenGLFunctions_4_4_Compatibility>()->glDrawPixels(m_width, m_height, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, m_pixels.data());
    QOpenGLWidget::paintGL();
}

void Monitor::resizeGL(int width, int height)
{
    m_pixels.resize(width * height);
    m_width = width;
    m_height = height;

    QOpenGLWidget::resizeGL(width, height);
}

void Monitor::redraw()
{
    qApp->postEvent(this, new QPaintEvent(QRect(0, 0, m_width, m_height)));
}

} // namespace tt08