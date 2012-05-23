/****************************************************************************
 **
 ** Copyright (C) 2012 Digia Plc
 ** All rights reserved.
 ** For any questions to Digia, please use contact form at http://qt.digia.com
 **
 ** This file is part of the Qt Commercial Charts Add-on.
 **
 ** $QT_BEGIN_LICENSE$
 ** Licensees holding valid Qt Commercial licenses may use this file in
 ** accordance with the Qt Commercial License Agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and Digia.
 **
 ** If you have questions regarding the use of this file, please use
 ** contact form at http://qt.digia.com
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#include "xychart_p.h"
#include "qxyseries.h"
#include "qxyseries_p.h"
#include "chartpresenter_p.h"
#include "chartanimator_p.h"
#include <QPainter>
#include <QAbstractItemModel>
#include "qxymodelmapper.h"

QTCOMMERCIALCHART_BEGIN_NAMESPACE

//TODO: optimize : remove points which are not visible

XYChart::XYChart(QXYSeries *series, ChartPresenter *presenter):Chart(presenter),
m_minX(0),
m_maxX(0),
m_minY(0),
m_maxY(0),
m_series(series),
m_animation(0)
{
    QObject::connect(series->d_func(),SIGNAL(pointReplaced(int)),this,SLOT(handlePointReplaced(int)));
    QObject::connect(series->d_func(),SIGNAL(pointAdded(int)),this,SLOT(handlePointAdded(int)));
    QObject::connect(series->d_func(),SIGNAL(pointsAdded(int, int)),this,SLOT(handlePointsAdded(int, int)));
    QObject::connect(series->d_func(),SIGNAL(pointRemoved(int)),this,SLOT(handlePointRemoved(int)));
    QObject::connect(series->d_func(),SIGNAL(pointsRemoved(int, int)),this,SLOT(handlePointsRemoved(int, int)));
    QObject::connect(series->d_func(),SIGNAL(reinitialized()),this,SLOT(handleReinitialized()));
    QObject::connect(this,SIGNAL(clicked(QPointF)),series,SIGNAL(clicked(QPointF)));
}

void XYChart::setGeometryPoints(QVector<QPointF>& points)
{
    m_points = points;
}

void XYChart::setClipRect(const QRectF &rect)
{
    m_clipRect = rect;
}

void XYChart::setAnimation(XYAnimation* animation)
{
    m_animation=animation;
}

QPointF XYChart::calculateGeometryPoint(const QPointF &point) const
{
    const qreal deltaX = m_size.width()/(m_maxX-m_minX);
    const qreal deltaY = m_size.height()/(m_maxY-m_minY);
    qreal x = (point.x() - m_minX)* deltaX;
    qreal y = (point.y() - m_minY)*-deltaY + m_size.height();
    return QPointF(x,y);
}

QPointF XYChart::calculateGeometryPoint(int index) const
{
    const qreal deltaX = m_size.width()/(m_maxX-m_minX);
    const qreal deltaY = m_size.height()/(m_maxY-m_minY);
    const QList<QPointF>& vector = m_series->points();
    qreal x = (vector[index].x() - m_minX)* deltaX;
    qreal y = (vector[index].y() - m_minY)*-deltaY + m_size.height();
    return QPointF(x,y);
}

QVector<QPointF> XYChart::calculateGeometryPoints() const
{
    const qreal deltaX = m_size.width()/(m_maxX-m_minX);
    const qreal deltaY = m_size.height()/(m_maxY-m_minY);

    QVector<QPointF> result;
    result.resize(m_series->count());
    const QList<QPointF>& vector = m_series->points();
    for (int i = 0; i < m_series->count(); ++i) {
        qreal x = (vector[i].x() - m_minX)* deltaX;
        qreal y = (vector[i].y() - m_minY)*-deltaY + m_size.height();
        result[i].setX(x);
        result[i].setY(y);
    }
    return result;
}

QPointF XYChart::calculateDomainPoint(const QPointF &point) const
{
    const qreal deltaX = m_size.width()/(m_maxX-m_minX);
    const qreal deltaY = m_size.height()/(m_maxY-m_minY);
    qreal x = point.x()/deltaX +m_minX;
    qreal y = (point.y()-m_size.height())/(-deltaY)+ m_minY;
    return QPointF(x,y);
}

void XYChart::updateChart(QVector<QPointF> &oldPoints, QVector<QPointF> &newPoints,int index)
{
    if (m_animation) {
        m_animation->setValues(oldPoints, newPoints, index);
        animator()->startAnimation(m_animation);
    }
    else {
        setGeometryPoints(newPoints);
        updateGeometry();
    }
}

//handlers

void XYChart::handlePointAdded(int index)
{
    if (m_series->model() == 0) {
        Q_ASSERT(index<m_series->count());
        Q_ASSERT(index>=0);
    }

    QVector<QPointF> points = m_points;
    QPointF point;
    point = calculateGeometryPoint(index);
    points.insert(index, point);

    if(m_animation) {
        m_animation->setAnimationType(XYAnimation::LineDrawAnimation);
    }

   updateChart(m_points,points,index);
}

void XYChart::handlePointsAdded(int start, int end)
{
    if (m_series->model() == 0) {
        for (int i = start; i <= end; i++)
        handlePointAdded(i);
    }
    else {
        int mapFirst = m_series->modelMapper()->first();
        int mapCount = m_series->modelMapper()->count();
        if (mapCount != -1 && start >= mapFirst + mapCount) {
            return;
        }
        else {
            int addedCount = end - start + 1;
            if (mapCount != -1 && addedCount > mapCount)
            addedCount = mapCount;
            int first = qMax(start, mapFirst); // get the index of the first item that will be added
            int last = qMin(first + addedCount - 1, mapCount + mapFirst - 1);// get the index of the last item that will be added
            for (int i = first; i <= last; i++) {
                handlePointAdded(i - mapFirst);
            }
            // the map is limited therefore the items that are now outside the map
            // need to be removed from the drawn points
            if (mapCount != -1 && m_points.size() > mapCount)
            for (int i = m_points.size() - 1; i >= mapCount; i--)
            handlePointRemoved(i);
        }
    }
}

void XYChart::handlePointRemoved(int index)
{
    if (m_series->model() == 0) {
        Q_ASSERT(index<m_series->count() + 1);
        Q_ASSERT(index>=0);
    }
    QVector<QPointF> points = m_points;
    points.remove(index);

    if(m_animation) {
        m_animation->setAnimationType(XYAnimation::LineDrawAnimation);
    }

    updateChart(m_points,points,index);
}

void XYChart::handlePointsRemoved(int start, int end)
{
    Q_UNUSED(start)
    Q_UNUSED(end)
    if (m_series->model() == 0) {
        for (int i = end; i >= start; i--)
        handlePointRemoved(i);
    }
    else {
        // series uses model as a data source
        int mapFirst = m_series->modelMapper()->first();
        int mapCount = m_series->modelMapper()->count();
        int removedCount = end - start + 1;
        if (mapCount != -1 && start >= mapFirst + mapCount) {
            return;
        }
        else {
            int toRemove = qMin(m_points.size(), removedCount); // first find how many items can actually be removed
            int first = qMax(start, mapFirst);// get the index of the first item that will be removed.
            int last = qMin(first + toRemove - 1, m_points.size() + mapFirst - 1);// get the index of the last item that will be removed.
            if (last - first == 0) {
                for (int i = last; i >= first; i--) {
                    handlePointRemoved(i - mapFirst);

                }
            }
            else {
                QVector<QPointF> points = m_points;
                for (int i = last; i >= first; i--)
                points.remove(i - mapFirst);
                setGeometryPoints(points);
                updateGeometry();
            }
            if (mapCount != -1) {
                int itemsAvailable; // check how many are available to be added
                if (m_series->modelMapper()->orientation() == Qt::Vertical)
                itemsAvailable = m_series->model()->rowCount() - mapFirst - m_points.size();
                else
                itemsAvailable = m_series->model()->columnCount() - mapFirst - m_points.size();
                int toBeAdded = qMin(itemsAvailable, mapCount - m_points.size());// add not more items than there is space left to be filled.
                int currentSize = m_points.size();
                if (toBeAdded > 0)
                for (int i = m_points.size(); i < currentSize + toBeAdded; i++) {
                    handlePointAdded(i);
                }
            }
        }
    }

}

void XYChart::handlePointReplaced(int index)
{
    Q_ASSERT(index<m_series->count());
    Q_ASSERT(index>=0);
    QPointF point = calculateGeometryPoint(index);
    QVector<QPointF> points = m_points;
    points.replace(index,point);

    if(m_animation) {
        m_animation->setAnimationType(XYAnimation::MoveDownAnimation);
    }

    updateChart(m_points,points,index);
}

void XYChart::handleReinitialized()
{
    QVector<QPointF> points = calculateGeometryPoints();

    if(m_animation) {
        m_animation->setAnimationType(XYAnimation::LineDrawAnimation);
    }

    updateChart(m_points,points);
}

void XYChart::handleDomainChanged(qreal minX, qreal maxX, qreal minY, qreal maxY)
{
    m_minX=minX;
    m_maxX=maxX;
    m_minY=minY;
    m_maxY=maxY;
    if (isEmpty()) return;
    QVector<QPointF> points = calculateGeometryPoints();

    if(m_animation) {
        m_animation->setAnimationType(XYAnimation::MoveDownAnimation);
    }
    updateChart(m_points,points);
}

void XYChart::handleGeometryChanged(const QRectF &rect)
{
    Q_ASSERT(rect.isValid());
    m_size=rect.size();
    m_clipRect=rect.translated(-rect.topLeft());
    m_origin=rect.topLeft();

    if (isEmpty()) return;
    QVector<QPointF> points = calculateGeometryPoints();
    if(m_animation) {
          m_animation->setAnimationType(XYAnimation::LineDrawAnimation);
      }
    updateChart(m_points,points);
}

bool XYChart::isEmpty()
{
    return !m_clipRect.isValid() || qFuzzyIsNull(m_maxX - m_minX) || qFuzzyIsNull(m_maxY - m_minY) || m_series->points().isEmpty();
}

#include "moc_xychart_p.cpp"

QTCOMMERCIALCHART_END_NAMESPACE
