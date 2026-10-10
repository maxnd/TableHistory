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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextList>
#include <QKeyEvent>
#include <QLabel>
#include <QSplitter>
#include <QCloseEvent>
#include <QTextCharFormat>
#include <QSortFilterProxyModel>
#include "historytablemodel.h"
#include "ganttwidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void fileNew();
    void fileOpen();
    void fileSave();
    void fileSaveAs();
    void exportDiagram();

    void itemAdd();
    void itemEdit();
    void itemDelete();
    void itemCopy();
    void itemPaste();

    void updateZoomLabel(double factor);
    void onTableSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onGanttItemSelected(int index);

    void zoomInInterface();
    void zoomOutInterface();
    void resetInterfaceZoom();

private:
    QAction *m_actionCopy = nullptr;
    QAction *m_actionSelectAll = nullptr;
    QAction *m_actionFind = nullptr;
    QAction *m_actionFindNext = nullptr;
    QShortcut *m_findNextShortcut = nullptr;
    QString m_lastSearchQuery;
    int m_lastSearchIndex = -1;

    void setupUi();
    void createMenus();
    void loadCsvFile(const QString &filePath);
    bool saveCsvFile(const QString &filePath, bool showConfirmation = true);
    void readSettings();
    void saveSettings();
    void updateGanttItems();
    void findItem();
    void findNextItem();
    void performSearch(const QString &query, int startIndex, bool wrap);

    HistoryTableModel *m_model = nullptr;
    QSortFilterProxyModel *m_proxyModel = nullptr;
    QTableView *m_tableView = nullptr;
    QTextBrowser *m_notesMemo = nullptr;

    GanttWidget *m_ganttWidget = nullptr;
    QLabel *m_zoomLabel = nullptr;
    QSplitter *m_mainSplitter = nullptr;
    QSplitter *m_leftSplitter = nullptr;

    QString m_currentFilePath;
    int m_baseFontSize = 10;
};

#endif // MAINWINDOW_H
