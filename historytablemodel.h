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

#ifndef HISTORYTABLEMODEL_H
#define HISTORYTABLEMODEL_H

#include <QAbstractTableModel>
#include "historyitem.h"

class HistoryTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        ColKind = 0,
        ColName,
        ColStartYear,
        ColStartUncertain,
        ColEndYear,
        ColEndUncertain,
        ColPlace,
        ColNotes,
        ColumnCount
    };

    explicit HistoryTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void setItems(const QList<HistoryItem> &items);
    QList<HistoryItem> items() const { return m_items; }
    HistoryItem getItem(int row) const { return m_items.value(row); }

    void addItem(const HistoryItem &item);
    void updateItem(int row, const HistoryItem &item);
    void removeItem(int row);

private:
    QList<HistoryItem> m_items;
};

#endif // HISTORYTABLEMODEL_H
