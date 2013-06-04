/****************************************************************************
**
** Copyright (C) 2013 Digia Plc
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

//  W A R N I N G
//  -------------
//
// This file is not part of the QtCommercial Chart API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef BOXWHISKERS_H
#define BOXWHISKERS_H

#include "boxwhiskersdata_p.h"
#include "qchartglobal.h"
#include "abstractdomain_p.h"
#include <QBoxSet>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsLayoutItem>
#include <QPainterPath>

QTCOMMERCIALCHART_BEGIN_NAMESPACE

class QBarSet;

class BoxWhiskers : public QGraphicsObject
{
    Q_OBJECT

public:
    BoxWhiskers(QBoxSet *set, AbstractDomain *domain, QGraphicsObject *parent);
    ~BoxWhiskers();

    void setBrush(const QBrush &brush);
    void setPen(const QPen &pen);
    void setLayout(const BoxWhiskersData &data);
    void setDomainSize(const QSizeF &size);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    void updateGeometry();
protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;
    void setGeometry(const QRectF &rect);

private:
    void updateBoundingRect();

Q_SIGNALS:
    void clicked(QBoxSet *boxset);
    void hovered(bool status, QBoxSet *boxset);

private:
    friend class BoxPlotChartItem;
    friend class BoxPlotAnimation;

    QBoxSet *m_boxSet;
    AbstractDomain *m_domain;
    QPainterPath    m_boxPath;
    QRectF m_boundingRect;
    bool m_hovering;
    bool m_validData;
    QBrush  m_brush;
    QPen m_pen;
    BoxWhiskersData m_data;
    QSizeF m_domainSize;
};

QTCOMMERCIALCHART_END_NAMESPACE

#endif // BOXWHISKERS_H