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

#ifndef GANTTWIDGET_H
#define GANTTWIDGET_H

#include <QWidget>
#include <QList>
#include <QRectF>
#include <QEvent>
#include "historyitem.h"

class GanttWidget : public QWidget {
    Q_OBJECT

public:
    explicit GanttWidget(QWidget *parent = nullptr);

    void setItems(const QList<HistoryItem> &items);
    void setSelectedIndex(int index);
    int selectedIndex() const { return m_selectedIndex; }

    double zoomFactor() const { return m_zoomFactor; }

    void setRowHeight(int height);
    void setHeaderHeight(int height);
    int rowHeight() const { return m_rowHeight; }
    int headerHeight() const { return m_headerHeight; }

    void scrollToItem(int index);

public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void setZoomFactor(double factor);

signals:
    void itemSelected(int index);
    void zoomFactorChanged(double factor);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void changeEvent(QEvent *event) override;
    QSize sizeHint() const override;

private:
    QList<HistoryItem> m_items;
    int m_selectedIndex = -1;
    double m_zoomFactor = 1.0;

    int m_headerHeight = 28;
    int m_rowHeight = 36;

    struct ItemRect {
        int index;
        QRectF rect;
    };
    QList<ItemRect> m_renderedRects;

    int m_minYear = 1800;
    int m_maxYear = 2026;

    void calculateYearRange();
    double yearToX(double year, double width) const;
    void updateDimensions();
};

#endif // GANTTWIDGET_H
