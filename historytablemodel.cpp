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

#include "historytablemodel.h"
#include "historyitem.h"

HistoryTableModel::HistoryTableModel(QObject *parent)
    : QAbstractTableModel(parent) {}

int HistoryTableModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_items.size();
}

int HistoryTableModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return ColumnCount;
}

QVariant HistoryTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_items.size())
        return QVariant();

    const auto &item = m_items[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColKind:
            return HistoryItem::kindToString(item.kind);
        case ColName:
            return item.name;
        case ColStartYear:
            if (item.startYear == 0) return QVariant(QString(" ")); // Show space if 0
            return item.startYear;
        case ColStartUncertain:
            return QVariant(); // Checkbox
        case ColEndYear:
            if (item.endYear == 0) return QVariant(QString(" ")); // Show space if 0
            return item.endYear;
        case ColEndUncertain:
            return QVariant();   // Checkbox
        case ColPlace:
            return item.place;
        case ColNotes:
            return item.notes;
        default:
            break;
        }
    } else if (role == Qt::CheckStateRole) {
        if (index.column() == ColStartUncertain) {
            return item.startUncertain ? Qt::Checked : Qt::Unchecked;
        } else if (index.column() == ColEndUncertain) {
            return item.endUncertain ? Qt::Checked : Qt::Unchecked;
        }
    } else if (role == Qt::TextAlignmentRole) {
        if (index.column() == ColStartUncertain || index.column() == ColEndUncertain) {
            return Qt::AlignCenter;
        }
    }

    return QVariant();
}

Qt::ItemFlags HistoryTableModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) return Qt::NoItemFlags;

    Qt::ItemFlags f = QAbstractTableModel::flags(index);

    // For checkbox columns, allow them to be displayed as checkable,
    // but disable general text editing if applicable
    if (index.column() == ColStartUncertain || index.column() == ColEndUncertain) {
        return (f | Qt::ItemIsUserCheckable) & ~Qt::ItemIsEditable;
    }

    return f;
}

bool HistoryTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() >= m_items.size()) return false;

    if (role == Qt::CheckStateRole) {
        // Make StartUncertain and EndUncertain read-only
        if (index.column() == ColStartUncertain || index.column() == ColEndUncertain) {
            return false; // Rejects user clicks from altering the checkbox state
        }
    }

    // Handle normal data changes for other columns/roles here...
    return QAbstractTableModel::setData(index, value, role);
}

QVariant HistoryTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case ColKind: return "Kind";
        case ColName: return "Name";
        case ColStartYear: return "Beginning";
        case ColStartUncertain: return "?";
        case ColEndYear: return "End";
        case ColEndUncertain: return "?";
        case ColPlace: return "Place";
        case ColNotes: return "Notes";
        default: break;
        }
    } else if (orientation == Qt::Vertical) {
        return QString::number(section + 1);
    }

    return QVariant();
}

void HistoryTableModel::setItems(const QList<HistoryItem> &items) {
    beginResetModel();
    m_items = items;
    endResetModel();
}

void HistoryTableModel::addItem(const HistoryItem &item) {
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append(item);
    endInsertRows();
}

void HistoryTableModel::updateItem(int row, const HistoryItem &item) {
    if (row < 0 || row >= m_items.size()) return;
    m_items[row] = item;
    emit dataChanged(index(row, 0), index(row, ColumnCount - 1));
}

void HistoryTableModel::removeItem(int row) {
    if (row < 0 || row >= m_items.size()) return;
    beginRemoveRows(QModelIndex(), row, row);
    m_items.removeAt(row);
    endRemoveRows();
}
