/*
* TableHistory 1.x
* Author and copyright (C): Massimo Nardello, Modena (Italy) 2026.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version. You can read the version 3
* of the Licence in http://www.gnu.org/licenses/gpl-3.0.txt
* or in the file Licence.txt included in the files of the
* source code of this software.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* This program has been created with the support of Google Gemini.
*
*/

#include "ganttwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QtMath>
#include <utility>

GanttWidget::GanttWidget(QWidget *parent)
    : QWidget(parent) {
    setMinimumWidth(600);
    setMouseTracking(true);
}

void GanttWidget::setHeaderHeight(int height) {
    m_headerHeight = height;
    updateDimensions();
}

void GanttWidget::setRowHeight(int height) {
    m_rowHeight = height;
    updateDimensions();
}

void GanttWidget::setItems(const QList<HistoryItem> &items) {
    m_items = items;
    calculateYearRange();
    updateDimensions();
    update();
}

void GanttWidget::setSelectedIndex(int index) {
    if (m_selectedIndex != index) {
        m_selectedIndex = index;
        update();
    }
}

void GanttWidget::scrollToItem(int index) {
    if (index < 0 || index >= m_items.size()) return;

    const auto &item = m_items.at(index);
    double x1 = yearToX(item.startYear, width());
    double x2;
    if (item.endYear) {
      x2 = yearToX(item.endYear + 1, width());
    } else {
      x2 = yearToX(item.startYear + 1, width());
    }

    QWidget *p = parentWidget();
    while (p) {
        if (auto *scrollArea = qobject_cast<QScrollArea*>(p)) {
            QScrollBar *hBar = scrollArea->horizontalScrollBar();
            int visibleLeft = hBar->value();
            int visibleWidth = scrollArea->viewport()->width();
            int visibleRight = visibleLeft + visibleWidth;

            int itemLeft = qRound(x1);
            int itemRight = qRound(x2);
            int itemWidth = itemRight - itemLeft;

            const int margin = 40;

            if (itemLeft < visibleLeft + margin || itemRight > visibleRight - margin) {
                int targetX = itemLeft - margin;
                if (itemWidth < visibleWidth - (2 * margin)) {
                    targetX = itemLeft - (visibleWidth - itemWidth) / 2;
                }
                hBar->setValue(targetX);
            }
            break;
        }
        p = p->parentWidget();
    }
}

void GanttWidget::zoomIn() {
    setZoomFactor(m_zoomFactor * 1.25);
}

void GanttWidget::zoomOut() {
    setZoomFactor(m_zoomFactor / 1.25);
}

void GanttWidget::resetZoom() {
    setZoomFactor(1.0);
}

void GanttWidget::setZoomFactor(double factor) {
    double clamped = qBound(0.2, factor, 20.0);
    if (!qFuzzyCompare(m_zoomFactor, clamped)) {
        m_zoomFactor = clamped;
        updateDimensions();
        emit zoomFactorChanged(m_zoomFactor);
        if (m_selectedIndex >= 0) {
            scrollToItem(m_selectedIndex);
        }
    }
}

void GanttWidget::updateDimensions() {
    int baseWidth = 1100;
    if (parentWidget() && parentWidget()->width() > 0) {
        baseWidth = qMax(parentWidget()->width() + 300, 4500); // Adds extra width beyond the viewport for labels
    }
    int targetWidth = qRound(baseWidth * m_zoomFactor);
    setMinimumWidth(qMax(700, targetWidth));

    int totalHeight = m_headerHeight + (m_items.size() * m_rowHeight);
    setFixedHeight(qMax(200, totalHeight));

    updateGeometry();
    update();
}

void GanttWidget::calculateYearRange() {
    if (m_items.isEmpty()) {
        m_minYear = 1800;
        m_maxYear = 2026;
        return;
    }

    int minY = m_items.first().startYear;
    int maxY = m_items.first().endYear;

    for (const auto &item : std::as_const(m_items)) {
        if (item.startYear < minY) minY = item.startYear;
        if (item.endYear > maxY)   maxY = item.endYear;
    }

    if (minY >= maxY) maxY = minY + 10;

    int span = maxY - minY;
    int pad = qMax(2, span / 10);
    m_minYear = minY - pad;
    m_maxYear = maxY + pad;
}

double GanttWidget::yearToX(double year, double width) const {
    double leftMargin = 20.0;
    double rightMargin = 200.0; // Increased from 30.0 to reserve space for item labels
    double usableWidth = width - leftMargin - rightMargin;
    if (m_maxYear <= m_minYear) return leftMargin;
    return leftMargin + ((year - m_minYear) / static_cast<double>(m_maxYear - m_minYear)) * usableWidth;
}

QSize GanttWidget::sizeHint() const {
    int totalHeight = m_headerHeight + (m_items.size() * m_rowHeight);
    return QSize(minimumWidth(), qMax(200, totalHeight));
}

void GanttWidget::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        const double delta = event->angleDelta().y();
        if (delta > 0) zoomIn();
        else if (delta < 0) zoomOut();
        event->accept();
    } else {
        QWidget::wheelEvent(event);
    }
}

void GanttWidget::changeEvent(QEvent *event) {
    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::ApplicationPaletteChange) {
        update(); // Redraw canvas on theme toggle (Dark / Light mode)
    }
    QWidget::changeEvent(event);
}

void GanttWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    m_renderedRects.clear();

    int w = width();
    int h = height();

    // Determine current vertical scroll offset from the parent QScrollArea viewport
    int verticalOffset = 0;
    QWidget *p = parentWidget();
    while (p) {
        if (auto *scrollArea = qobject_cast<QScrollArea*>(p)) {
            verticalOffset = scrollArea->verticalScrollBar()->value();
            break;
        }
        p = p->parentWidget();
    }

    // Theme Detection
    bool isDark = palette().color(QPalette::Window).lightness() < 128;

    QColor bgColor           = isDark ? QColor(30, 30, 34)   : QColor(250, 250, 252);
    QColor gridLineColor     = isDark ? QColor(60, 60, 68)   : QColor(230, 230, 235);
    QColor headerBgColor     = isDark ? QColor(38, 38, 42)   : QColor(240, 240, 245); // Background for pinned header
    QColor headerTextColor   = isDark ? QColor(170, 170, 180): QColor(100, 100, 110);
    QColor headerDividerColor= isDark ? QColor(80, 80, 90)   : QColor(180, 180, 190);
    QColor rowDividerColor   = isDark ? QColor(48, 48, 54)   : QColor(235, 235, 240);
    QColor itemLabelColor    = isDark ? QColor(230, 230, 235): QColor(30, 30, 30);

    // Canvas Background for the whole widget
    painter.fillRect(rect(), bgColor);

    // Render Bars and vertical grid lines first (so they scroll normally)
    double barHeight = 20.0;
    double barVerticalOffset = (m_rowHeight - barHeight) / 2.0;

    int yearSpan = m_maxYear - m_minYear;
    double effectiveSpan = yearSpan / m_zoomFactor;

    int step = 1;
    if (effectiveSpan > 300)      step = 50;
    else if (effectiveSpan > 150) step = 20;
    else if (effectiveSpan > 60)  step = 10;
    else if (effectiveSpan > 25)  step = 5;
    else if (effectiveSpan > 10)  step = 2;
    else                          step = 1;

    // Draw vertical grid lines across the full height
    int startLabelYear = (m_minYear / step) * step;
    for (int yr = startLabelYear; yr <= m_maxYear; yr += step) {
        if (yr < m_minYear) continue;
        double x = yearToX(yr, w);

        painter.setPen(QPen(gridLineColor, 1, Qt::DashLine));
        painter.drawLine(QPointF(x, m_headerHeight), QPointF(x, h));
    }

    // Render Rows & Bars
    for (int i = 0; i < m_items.size(); ++i) {
        const auto &item = m_items.at(i);
        double rowTopY = m_headerHeight + (i * m_rowHeight);

        painter.setPen(QPen(rowDividerColor, 1));
        painter.drawLine(QPointF(0, rowTopY + m_rowHeight), QPointF(w, rowTopY + m_rowHeight));

        double barTopY = rowTopY + barVerticalOffset;
        double x1 = yearToX(item.startYear, w);
        double x2 = yearToX(item.endYear + 1, w);
        double barWidth = qMax(x2 - x1, 8.0);

        QRectF rectF(x1, barTopY, barWidth, barHeight);
        m_renderedRects.append({i, rectF});

        QColor color;
        switch (item.kind) {
        case ItemKind::Event:    color = isDark ? QColor(80, 140, 235) : QColor(64, 128, 223); break;
        case ItemKind::Document: color = isDark ? QColor(52, 180, 115) : QColor(46, 160, 100); break;
        case ItemKind::Person:   color = isDark ? QColor(240, 145, 50) : QColor(225, 130, 40); break;
        }

        bool isSelected = (i == m_selectedIndex);

        QBrush fillBrush(color);
        if (item.startUncertain || item.endUncertain) {
            fillBrush = QBrush(color, Qt::BDiagPattern);
        }

        painter.setBrush(fillBrush);
        QPen outlinePen(isSelected ? (isDark ? QColor(255, 90, 90) : QColor(200, 30, 30))
                                   : color.darker(130), isSelected ? 2.5 : 1.0);
        painter.setPen(outlinePen);
        painter.drawRoundedRect(rectF, 4, 4);

        if (isSelected) {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(isDark ? QColor(255, 100, 100, 220) : QColor(255, 60, 60, 180), 2.0));
            painter.drawRoundedRect(rectF.adjusted(-3, -3, 3, 3), 6, 6);
        }

        painter.setPen(itemLabelColor);
        QFont font = painter.font();
        font.setBold(isSelected);
        font.setPointSize(11);
        painter.setFont(font);

        QString label;
        if (item.endYear) {
            label = QString("%1 (%2-%3)")
            .arg(item.name,
                 item.startUncertain ? QString("~%1").arg(item.startYear) : QString::number(item.startYear),
                 item.endUncertain ? QString("~%1").arg(item.endYear) : QString::number(item.endYear));
        } else {
            label = QString("%1 (%2)")
            .arg(item.name,
                 item.startUncertain ? QString("~%1").arg(item.startYear) : QString::number(item.startYear));
        }

        QRectF textRect(rectF.right() + 8, barTopY, w - rectF.right() - 10, barHeight);
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, label);
    }

    // Pinned header (Stays fixed at the top of the visible viewport)
    double pinnedHeaderY = verticalOffset;
    QRectF headerBgRect(0, pinnedHeaderY, w, m_headerHeight);
    painter.fillRect(headerBgRect, headerBgColor);

    QFont headerFont = painter.font();
    headerFont.setPointSize(11);
    headerFont.setBold(true);
    painter.setFont(headerFont);

    for (int yr = startLabelYear; yr <= m_maxYear; yr += step) {
        if (yr < m_minYear) continue;
        double x = yearToX(yr, w);

        painter.setPen(headerTextColor);
        painter.drawText(QRectF(x - 30, pinnedHeaderY + (m_headerHeight - 18) / 2.0, 60, 18), Qt::AlignCenter, QString::number(yr));
    }

    // Header bottom divider line pinned to viewport
    painter.setPen(QPen(headerDividerColor, 1.5));
    painter.drawLine(QPointF(0, pinnedHeaderY + m_headerHeight), QPointF(w, pinnedHeaderY + m_headerHeight));
}

void GanttWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QPointF pos = event->position();
        for (const auto &ir : std::as_const(m_renderedRects)) {
            if (ir.rect.contains(pos)) {
                setSelectedIndex(ir.index);
                emit itemSelected(ir.index);
                return;
            }
        }
    }
    QWidget::mousePressEvent(event);
}
