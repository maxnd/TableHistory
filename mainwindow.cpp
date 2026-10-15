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

#include "mainwindow.h"
#include "itemeditdialog.h"
#include "aboutdialog.h"
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSplitter>
#include <QHeaderView>
#include <QScrollBar>
#include <QScrollArea>
#include <QStatusBar>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QKeySequence>
#include <QTextBlockFormat>
#include <QStyleHints>
#include <QGuiApplication>
#include <QClipboard>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTextDocument>
#include <QShortcut>
#include <QPixmap>
#include <QTextBrowser>
#include <QRegularExpression>
#include <QPdfWriter>
#include <QPainter>
#include <QPageSize>
#include <QPageLayout>
#include <QStandardPaths>

static QStringList parseCsvLine(const QString &line);

// Helper function to convert plain text URLs into clickable HTML anchor tags safely
static QString linkifyNotes(QString text) {
    if (text.isEmpty()) return text;

    // Matches http://, https://, ftp://, or www. URLs that are not already inside an HTML attribute
    QRegularExpression urlRegex(R"((?<!href=["'])(?<!src=["'])\b((https?|ftp)://[^\s<"']+|www\.[^\s<"']+))", QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatchIterator it = urlRegex.globalMatch(text);
    struct MatchInfo {
        qsizetype start;
        qsizetype length;
        QString url;
    };
    QList<MatchInfo> matches;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString url = match.captured(1);
        // Skip HTML standard DTD schema links
        if (url.contains("w3.org", Qt::CaseInsensitive)) {
            continue;
        }
        matches.append({match.capturedStart(1), match.capturedLength(1), url});
    }

    // Process matches from right to left so index positions do not shift during replacement
    for (int idx = matches.size() - 1; idx >= 0; --idx) {
        const auto &m = matches.at(idx);
        QString href = m.url;
        if (href.startsWith("www.", Qt::CaseInsensitive)) {
            href = "https://" + href;
        }
        QString replacement = QString("<a href=\"%1\">%2</a>").arg(href, m.url);
        text.replace(m.start, m.length, replacement);
    }

    return text;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("TableHistory");
    resize(1150, 680);

    m_model = new HistoryTableModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setDynamicSortFilter(true);

    setupUi();
    createMenus();

    readSettings();

    statusBar()->setSizeGripEnabled(true);

    QSettings settings("TableHistory", "TableHistory");
    QString lastPath = settings.value("lastFilePath").toString();

    if (!lastPath.isEmpty() && QFile::exists(lastPath)) {
        loadCsvFile(lastPath);
    }
    if (m_model->rowCount() > 0) {
        m_tableView->selectRow(0);
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_model->rowCount() > 0) {
        if (m_currentFilePath.isEmpty()) {
            fileSaveAs();
            if (m_currentFilePath.isEmpty()) {
                int ret = QMessageBox::warning(
                    this, "Save Data",
                    "Data has not been saved. Do you still want to exit?",
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No
                    );
                if (ret != QMessageBox::Yes) {
                    event->ignore();
                    return;
                }
            }
        } else {
            saveCsvFile(m_currentFilePath, false);
        }
    }

    saveSettings();
    event->accept();
}

void MainWindow::readSettings() {
    QSettings settings("TableHistory", "TableHistory");
    if (settings.contains("geometry")) {
        restoreGeometry(settings.value("geometry").toByteArray());
    }
    if (settings.contains("mainSplitterState")) {
        m_mainSplitter->restoreState(settings.value("mainSplitterState").toByteArray());
    }
    if (settings.contains("leftSplitterState")) {
        m_leftSplitter->restoreState(settings.value("leftSplitterState").toByteArray());
    }
    if (settings.contains("horizontalHeaderState")) {
        m_tableView->horizontalHeader()->restoreState(settings.value("horizontalHeaderState").toByteArray());
    }
}

void MainWindow::saveSettings() {
    QSettings settings("TableHistory", "TableHistory");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("mainSplitterState", m_mainSplitter->saveState());
    settings.setValue("leftSplitterState", m_leftSplitter->saveState());
    settings.setValue("horizontalHeaderState", m_tableView->horizontalHeader()->saveState());
}

void MainWindow::setupUi() {
    bool isDarkMode = false;
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    isDarkMode = (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark);
#else
    isDarkMode = (palette().color(QPalette::Window).lightness() < 128);
#endif
    const int HEADER_HEIGHT = 28;
    const int ROW_HEIGHT = 36;
    const int VERTICAL_HEADER_WIDTH = 50;

    QColor formColor = palette().color(QPalette::Window);
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_leftSplitter = new QSplitter(Qt::Vertical, this);
    QAction *exportAction = new QAction(tr("&Export diagram..."), this);

    // 1. Table View Grid
    m_tableView = new QTableView(this);
    m_tableView->setModel(m_proxyModel);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setFrameShape(QFrame::NoFrame);
    m_tableView->verticalHeader()->hide();
    m_tableView->setShowGrid(false);
    m_tableView->setSortingEnabled(true);
    m_tableView->horizontalHeader()->setSectionsClickable(true);
    #ifdef Q_OS_WIN
      m_tableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    #endif

    QString tableViewStyle = "QTableView {";
    if (isDarkMode) {
        tableViewStyle += QString(" background-color: %1;").arg(formColor.name());
        tableViewStyle += "  selection-background-color: rgba(0, 120, 215, 128);  selection-color: #ffffff;";
    } else {
        tableViewStyle += "  selection-background-color: rgba(0, 120, 215, 60);  selection-color: #000000;";
    }

    tableViewStyle +=
        "  border-right: 1px solid #606060;"
        "  border-left: none;"
        "  border-bottom: none;"
        "}";
#ifdef Q_OS_APPLE
    if (isDarkMode) {
        tableViewStyle += QString("QAbstractScrollArea::viewport { background-color: %1; }").arg(formColor.name());
        tableViewStyle +=
            "QTableCornerButton::section {"
            "  background: transparent;"
            "  border-bottom: 1px solid #606060;"
            "  border-right: 1px solid #606060;"
            "}"
            "QHeaderView::section:vertical {"
            "  border-top: none;"
            "  border-bottom: none;"
            "  border-right: 1px solid #606060;"
            "  border-left: none;"
            "  background: transparent;"
            "}"
            "QHeaderView::section:horizontal {"
            "  border-top: none;"
            "  border-bottom: 1px solid #606060;"
            "  border-right: 0px solid #606060;"
            "  border-left: 1px solid #606060;"
            "  background: transparent;"
            "}";
    } else {
        tableViewStyle +=
            "QTableCornerButton::section {"
            "  background: transparent;"
            "  border-bottom: 1px solid #C0C0C0;"
            "  border-right: 1px solid #C0C0C0;"
            "}"
            "QHeaderView::section:vertical {"
            "  border-top: none;"
            "  border-bottom: none;"
            "  border-right: 1px solid #C0C0C0;"
            "  border-left: none;"
            "  background: transparent;"
            "}"
            "QHeaderView::section:horizontal {"
            "  border-top: none;"
            "  border-bottom: 1px solid #C0C0C0;"
            "  border-right: 0px solid #C0C0C0;"
            "  border-left: 1px solid #C0C0C0;"
            "  background: transparent;"
            "}";
    }
#else
    tableViewStyle +=
        "QTableCornerButton::section {"
        "  background: transparent;"
        "  border-bottom: 1px solid #C0C0C0;"
        "  border-right: 1px solid #C0C0C0;"
        "}"
        "QHeaderView::section:vertical {"
        "  border-top: none;"
        "  border-bottom: none;"
        "  border-right: 1px solid #C0C0C0;"
        "  border-left: none;"
        "  background: transparent;"
        "}"
        "QHeaderView::section:horizontal {"
        "  border-top: none;"
        "  border-bottom: 1px solid #C0C0C0;"
        "  border-right: 0px solid #C0C0C0;"
        "  border-left: 1px solid #C0C0C0;"
        "  background: transparent;"
        "}";
#endif
    m_tableView->setStyleSheet(tableViewStyle);

    m_tableView->horizontalHeader()->setFixedHeight(HEADER_HEIGHT);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    m_tableView->verticalHeader()->setDefaultSectionSize(ROW_HEIGHT);
    m_tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_tableView->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

    m_tableView->setColumnHidden(HistoryTableModel::ColNotes, true);

    m_leftSplitter->addWidget(m_tableView);

    // 2. Notes Section Container (Read-Only QTextBrowser)
    auto *notesContainer = new QWidget(this);
    auto *notesLayout = new QVBoxLayout(notesContainer);
    notesLayout->setContentsMargins(0, 6, 0, 6);

    m_notesMemo = new QTextBrowser(this);
    m_notesMemo->setReadOnly(true);
    QFont memoFont = m_notesMemo->font();
    memoFont.setPointSize(qApp->font().pointSize() + 3);
    memoFont.setFamily("Avenir Next");
    m_notesMemo->setFont(memoFont);
    m_notesMemo->document()->setDefaultFont(memoFont);
    m_notesMemo->setOpenExternalLinks(true);
    m_notesMemo->setMinimumHeight(120);
    if (isDarkMode) {
        m_notesMemo->setStyleSheet("QTextBrowser { background-color: #2b2b2b; color: #ffffff; border: 1px solid #555555; border-radius: 4px; }");
    }
    m_notesMemo->setEnabled(false);

    notesLayout->addWidget(m_notesMemo);

    m_leftSplitter->addWidget(notesContainer);

    m_leftSplitter->setStretchFactor(0, 3);
    m_leftSplitter->setStretchFactor(1, 1);
    m_leftSplitter->setCollapsible(0, false);
    m_leftSplitter->setCollapsible(1, false);

    // --- Right Panel (Gantt Chart + Zoom Bar) ---
    auto *rightContainer = new QWidget(this);
    auto *rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    auto *scrollArea = new QScrollArea(this);
    m_ganttWidget = new GanttWidget(this);
    m_ganttWidget->setHeaderHeight(HEADER_HEIGHT);
    m_ganttWidget->setRowHeight(ROW_HEIGHT);

    scrollArea->setWidget(m_ganttWidget);
    scrollArea->setWidgetResizable(true);

    rightLayout->addWidget(scrollArea, 1);

    auto *zoomBar = new QHBoxLayout();
    zoomBar->setContentsMargins(8, 6, 8, 6);

    auto *zoomOutBtn = new QPushButton(" Zoom - ", this);
    auto *zoomResetBtn = new QPushButton(" Reset (100%) ", this);
    auto *zoomInBtn = new QPushButton(" Zoom + ", this);
    m_zoomLabel = new QLabel("Zoom: 100%", this);

    zoomBar->addWidget(new QLabel("<b>Gantt Zoom:</b>", this));
    zoomBar->addWidget(zoomOutBtn);
    zoomBar->addWidget(zoomResetBtn);
    zoomBar->addWidget(zoomInBtn);
    zoomBar->addWidget(m_zoomLabel);
    zoomBar->addStretch();

    rightLayout->addLayout(zoomBar);

    m_mainSplitter->addWidget(m_leftSplitter);
    m_mainSplitter->addWidget(rightContainer);
    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 1);

    setCentralWidget(m_mainSplitter);

    statusBar()->setSizeGripEnabled(true);
    auto *fileStatusLabel = new QLabel(this);
    fileStatusLabel->setStyleSheet("padding-left: 6px;");
    statusBar()->addWidget(fileStatusLabel);

    // Connections
    connect(m_tableView->verticalScrollBar(), &QScrollBar::valueChanged,
            scrollArea->verticalScrollBar(), &QScrollBar::setValue);
    connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged,
            m_tableView->verticalScrollBar(), &QScrollBar::setValue);

    connect(zoomInBtn, &QPushButton::clicked, m_ganttWidget, &GanttWidget::zoomIn);
    connect(zoomOutBtn, &QPushButton::clicked, m_ganttWidget, &GanttWidget::zoomOut);
    connect(zoomResetBtn, &QPushButton::clicked, m_ganttWidget, &GanttWidget::resetZoom);
    connect(m_ganttWidget, &GanttWidget::zoomFactorChanged, this, &MainWindow::updateZoomLabel);
    connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged,
            m_ganttWidget, qOverload<>(&QWidget::update));
    connect(exportAction, &QAction::triggered, this, &MainWindow::exportDiagram);

    connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onTableSelectionChanged);
    connect(m_ganttWidget, &GanttWidget::itemSelected,
            this, &MainWindow::onGanttItemSelected);

    connect(m_tableView, &QTableView::doubleClicked, this, &MainWindow::itemEdit);
    connect(m_tableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
            this, [this](int, Qt::SortOrder) { updateGanttItems(); });
}

void MainWindow::zoomInInterface() {
    QFont font = qApp->font();
    int newSize = font.pointSize() + 1;
    if (newSize <= 14) {
        font.setPointSize(newSize);
        qApp->setFont(font);
        this->setFont(font);

        m_tableView->setFont(font);
        m_tableView->horizontalHeader()->setFont(font);
        m_tableView->verticalHeader()->setFont(font);
        m_tableView->style()->polish(m_tableView);
    }
}

void MainWindow::zoomOutInterface() {
    QFont font = qApp->font();
    int newSize = font.pointSize() - 1;
    if (newSize >= 8) {
        font.setPointSize(newSize);
        qApp->setFont(font);
        this->setFont(font);

        m_tableView->setFont(font);
        m_tableView->horizontalHeader()->setFont(font);
        m_tableView->verticalHeader()->setFont(font);
        m_tableView->style()->polish(m_tableView);
    }
}

void MainWindow::resetInterfaceZoom() {
    QFont font = qApp->font();
    font.setPointSize(m_baseFontSize);
    qApp->setFont(font);
    this->setFont(font);

    m_tableView->setFont(font);
    m_tableView->horizontalHeader()->setFont(font);
    m_tableView->verticalHeader()->setFont(font);
    m_tableView->style()->polish(m_tableView);
}

void MainWindow::createMenus() {
    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&New", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N), this, &MainWindow::fileNew);
    fileMenu->addAction("&Open...", QKeySequence::Open, this, &MainWindow::fileOpen);
    fileMenu->addAction("&Save", QKeySequence::Save, this, &MainWindow::fileSave);
    fileMenu->addAction("Save &As...", QKeySequence::SaveAs, this, &MainWindow::fileSaveAs);
    fileMenu->addSeparator();

    // --- Recent Files Submenu ---
    m_recentFilesMenu = fileMenu->addMenu(tr("Recent &Files"));
    for (int i = 0; i < MaxRecentFiles; ++i) {
        m_recentFileActions[i] = m_recentFilesMenu->addAction(QString(), this, [this, i]() {
            QAction *action = qobject_cast<QAction*>(sender());
            if (action) {
                QString filePath = action->data().toString();
                if (!filePath.isEmpty() && QFile::exists(filePath)) {
                    loadCsvFile(filePath);
                } else {
                    QMessageBox::warning(this, tr("Open File"), tr("The file no longer exists."));
                    // Remove missing file from recent list
                    QSettings settings("TableHistory", "TableHistory");
                    QStringList files = settings.value("recentFileList").toStringList();
                    files.removeAll(filePath);
                    settings.setValue("recentFileList", files);
                    updateRecentFileActions();
                }
            }
        });
        m_recentFileActions[i]->setVisible(false);
    }
    updateRecentFileActions();

    fileMenu->addSeparator();

    QAction *exportAction = fileMenu->addAction(tr("&Export diagram..."));
    connect(exportAction, &QAction::triggered, this, &MainWindow::exportDiagram);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", QKeySequence::Quit, this, &QWidget::close);

    QMenu *itemsMenu = menuBar()->addMenu("&Items");
    itemsMenu->addAction("&Add...", QKeySequence::New, this, &MainWindow::itemAdd);
    itemsMenu->addAction("&Edit...", QKeySequence(Qt::CTRL | Qt::Key_E), this, &MainWindow::itemEdit);
    itemsMenu->addAction("&Delete", QKeySequence::Delete, this, &MainWindow::itemDelete);
    itemsMenu->addSeparator();
    m_actionFind = itemsMenu->addAction(tr("&Find..."), QKeySequence::Find, this, &MainWindow::findItem);
    m_actionFindNext = itemsMenu->addAction(tr("Find &Next"), QKeySequence(Qt::ALT | Qt::Key_G), this, &MainWindow::findNextItem);
    m_actionFindNext->setEnabled(false);
    m_actionFindPrev = itemsMenu->addAction(tr("Find &Previous"), QKeySequence(Qt::ALT | Qt::Key_D), this, &MainWindow::findPreviousItem);
    m_actionFindPrev->setEnabled(false);
    itemsMenu->addSeparator();
    itemsMenu->addAction(tr("Find and &select..."), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F), this, &MainWindow::selectFoundItems);
    itemsMenu->addSeparator();
    itemsMenu->addAction("&Copy selected", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C), this, &MainWindow::itemCopy);
    itemsMenu->addAction("&Paste from selection", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V), this, &MainWindow::itemPaste);

    QMenu *viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction("Zoom &In Diagram", QKeySequence::ZoomIn, m_ganttWidget, &GanttWidget::zoomIn);
    viewMenu->addAction("Zoom &Out Diagram", QKeySequence::ZoomOut, m_ganttWidget, &GanttWidget::zoomOut);
    viewMenu->addAction("&Reset Zoom Diagram", QKeySequence(Qt::CTRL | Qt::Key_0), m_ganttWidget, &GanttWidget::resetZoom);
    viewMenu->addSeparator();

    QMenu *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About...", this, [this]() {
        AboutDialog aboutDlg(this);
        aboutDlg.exec();
    });
}

void MainWindow::updateZoomLabel(double factor) {
    m_zoomLabel->setText(QString("Zoom: %1%").arg(qRound(factor * 100)));
}

void MainWindow::onTableSelectionChanged(const QItemSelection &, const QItemSelection &) {
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    bool hasSelection = !selected.isEmpty();

    if (m_actionCopy) m_actionCopy->setEnabled(hasSelection);
    if (m_actionSelectAll) m_actionSelectAll->setEnabled(hasSelection);

    if (hasSelection) {
        int proxyRow = selected.first().row();
        QModelIndex sourceIndex = m_proxyModel->mapToSource(selected.first());
        int row = sourceIndex.row();
        const auto &item = m_model->getItem(row);

        m_notesMemo->setEnabled(true);
        // Process notes to automatically linkify plain text URLs while skipping DTD schema links
        m_notesMemo->setHtml(linkifyNotes(item.notes));

        m_ganttWidget->setSelectedIndex(proxyRow);
        m_ganttWidget->scrollToItem(proxyRow);
    } else {
        m_notesMemo->clear();
        m_notesMemo->setEnabled(false);
        m_ganttWidget->setSelectedIndex(-1);
    }
}

void MainWindow::onGanttItemSelected(int index) {
    if (index >= 0 && index < m_proxyModel->rowCount()) {
        m_tableView->selectRow(index);
    }
}

void MainWindow::updateGanttItems() {
    QList<HistoryItem> sortedItems;
    for (int i = 0; i < m_proxyModel->rowCount(); ++i) {
        QModelIndex sourceIdx = m_proxyModel->mapToSource(m_proxyModel->index(i, 0));
        sortedItems.append(m_model->getItem(sourceIdx.row()));
    }
    m_ganttWidget->setItems(sortedItems);
}

void MainWindow::itemAdd() {
    ItemEditDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        HistoryItem item = dialog.getItem();
        m_model->addItem(item);

        QModelIndex sourceIndex = m_model->index(m_model->rowCount() - 1, 0);
        QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
        int sortedRow = proxyIndex.row();

        m_tableView->setCurrentIndex(proxyIndex);
        m_tableView->selectionModel()->select(proxyIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        m_tableView->scrollTo(proxyIndex);

        updateGanttItems();
        m_ganttWidget->setSelectedIndex(sortedRow);
        m_ganttWidget->scrollToItem(sortedRow);

        updateStatusBar();
    }
}

void MainWindow::itemEdit() {
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Edit Item", "Please select an item to edit.");
        return;
    }

    int row = m_proxyModel->mapToSource(selected.first()).row();
    ItemEditDialog dialog(m_model->getItem(row), this);
    if (dialog.exec() == QDialog::Accepted) {
        m_model->updateItem(row, dialog.getItem());
        updateGanttItems();
        onTableSelectionChanged(QItemSelection(), QItemSelection());
    }
}

void MainWindow::itemDelete() {
    QModelIndexList selectedRows = m_tableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        QMessageBox::information(this, "Delete Items", "Please select an item to delete.");
        return;
    }

    QString message = (selectedRows.size() == 1)
                          ? "Are you sure you want to delete this item?"
                          : QString("Are you sure you want to delete these %1 items?").arg(selectedRows.size());

    if (QMessageBox::question(this, "Confirm Delete", message) == QMessageBox::Yes) {
        QList<int> sourceRows;
        for (int i = 0; i < selectedRows.size(); ++i) {
            sourceRows.append(m_proxyModel->mapToSource(selectedRows.at(i)).row());
        }

        std::sort(sourceRows.begin(), sourceRows.end(), std::greater<int>());

        for (int row : sourceRows) {
            m_model->removeItem(row);
        }
        updateGanttItems();
        updateStatusBar();
    }
}

void MainWindow::itemCopy() {
    QModelIndexList selectedRows = m_tableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        return;
    }

    QString clipText = "TableHistoryItems:\n";
    clipText += "Kind,Name,StartYear,StartUncertain,EndYear,EndUncertain,Place,Notes\n";

    QList<int> rows;
    for (int i = 0; i < selectedRows.size(); ++i) {
        rows.append(m_proxyModel->mapToSource(selectedRows.at(i)).row());
    }
    std::sort(rows.begin(), rows.end());

    for (int row : rows) {
        const auto &item = m_model->getItem(row);
        clipText += item.toCsvRecord() + "\n";
    }

    QGuiApplication::clipboard()->setText(clipText);
}

void MainWindow::itemPaste() {
    QString clipText = QGuiApplication::clipboard()->text();
    if (!clipText.startsWith("TableHistoryItems:")) {
        QMessageBox::information(this, "Paste Items", "The clipboard does not contain valid TableHistory items.");
        return;
    }

    QStringList lines = clipText.split('\n', Qt::SkipEmptyParts);
    if (lines.size() <= 1) {
        return;
    }

    QList<HistoryItem> newItems;
    bool firstLine = true;
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines.at(i);
        if (line.trimmed().isEmpty()) continue;
        if (line.startsWith("TableHistoryItems:")) continue;
        if (firstLine && line.contains("Kind", Qt::CaseInsensitive)) {
            firstLine = false;
            continue;
        }
        firstLine = false;

        QStringList fields = parseCsvLine(line);
        if (fields.size() >= 8) {
            newItems.append(HistoryItem::fromCsvRecord(fields));
        }
    }

    if (newItems.isEmpty()) {
        QMessageBox::information(this, "Paste Items", "No valid items found in the clipboard to paste.");
        return;
    }

    QList<HistoryItem> items = m_model->items();
    items.append(newItems);
    m_model->setItems(items);
    updateGanttItems();

    QMessageBox::information(this, "Paste Items", QString("%1 item(s) successfully pasted.").arg(newItems.size()));
}

void MainWindow::performSearch(const QString &query, int startIndex, bool wrap, bool forward) {
    if (query.isEmpty() || m_proxyModel->rowCount() == 0) return;

    int rowCount = m_proxyModel->rowCount();
    int currentIndex = startIndex;
    bool found = false;

    for (int step = 0; step < rowCount; ++step) {
        int i;
        if (forward) {
            i = (currentIndex + step) % rowCount;
        } else {
            i = (currentIndex - step % rowCount + rowCount) % rowCount;
        }

        QModelIndex proxyIndex = m_proxyModel->index(i, 0);
        QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
        const HistoryItem &item = m_model->getItem(sourceIndex.row());

        QTextDocument notesDoc;
        notesDoc.setHtml(item.notes);
        QString plainNotes = notesDoc.toPlainText();

        if (item.name.contains(query, Qt::CaseInsensitive) ||
            item.place.contains(query, Qt::CaseInsensitive) ||
            plainNotes.contains(query, Qt::CaseInsensitive)) {

            QItemSelectionModel *selModel = m_tableView->selectionModel();
            selModel->clearSelection();

            selModel->setCurrentIndex(proxyIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            m_tableView->selectRow(i);
            m_tableView->scrollTo(proxyIndex, QAbstractItemView::PositionAtCenter);
            m_tableView->setFocus();

            m_lastSearchIndex = i;
            found = true;
            break;
        }
    }

    if (!found) {
        QMessageBox::information(this, tr("Find"), tr("No matching items found for '%1'.").arg(query));
    }
}

void MainWindow::exportDiagram() {
    if (!m_ganttWidget) return;

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export Gantt Diagram"),
        "",
        tr("PDF Files (*.pdf);;All Files (*)")
        );

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".pdf", Qt::CaseInsensitive)) {
        fileName += ".pdf";
    }

    QPdfWriter pdfWriter(fileName);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setPageOrientation(QPageLayout::Landscape);
    pdfWriter.setResolution(300); // Higher resolution for crisp vector/text output

    QPainter painter;
    if (painter.begin(&pdfWriter)) {
        // Retrieve available paint area dimensions on the PDF page
        QRectF rect = pdfWriter.pageLayout().paintRectPixels(pdfWriter.resolution());

        double x_scale = rect.width() / double(m_ganttWidget->width());
        double y_scale = rect.height() / double(m_ganttWidget->height());
        // Maintain aspect ratio by using the smaller scale factor, or stretch to fit
        double scale = qMin(x_scale, y_scale);

        painter.scale(scale, scale);
        m_ganttWidget->render(&painter);
        painter.end();
    }
}

void MainWindow::findItem() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Find Item"));
    dialog.resize(350, 120);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QLabel *label = new QLabel(tr("Find text in Name, Place, or Notes:"), &dialog);
    layout->addWidget(label);

    QLineEdit *lineEdit = new QLineEdit(&dialog);
    if (!m_lastSearchQuery.isEmpty()) {
        lineEdit->setText(m_lastSearchQuery);
        lineEdit->selectAll();
    }
    layout->addWidget(lineEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton(tr("&Find"), &dialog);
    okButton->setDefault(true);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), &dialog);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString searchText = lineEdit->text().trimmed();
    if (searchText.isEmpty()) {
        return;
    }

    m_lastSearchQuery = searchText;
    m_actionFindNext->setEnabled(true);
    m_actionFindPrev->setEnabled(true);

    int startRow = 0;
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (!selected.isEmpty()) {
        startRow = (selected.first().row() + 1) % m_proxyModel->rowCount();
    }
    m_tableView->setFocus();
    performSearch(m_lastSearchQuery, startRow, true);
}

void MainWindow::findNextItem() {
    if (m_lastSearchQuery.isEmpty()) {
        findItem();
        return;
    }

    int startRow = 0;
    if (m_lastSearchIndex >= 0) {
        startRow = (m_lastSearchIndex + 1) % m_proxyModel->rowCount();
    } else {
        QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
        if (!selected.isEmpty()) {
            startRow = (selected.first().row() + 1) % m_proxyModel->rowCount();
        }
    }
    m_tableView->setFocus();
    performSearch(m_lastSearchQuery, startRow, true);
}

void MainWindow::findPreviousItem() {
    if (m_lastSearchQuery.isEmpty()) {
        findItem();
        return;
    }

    int rowCount = m_proxyModel->rowCount();
    if (rowCount == 0) return;

    int startRow = rowCount - 1;
    if (m_lastSearchIndex >= 0) {
        startRow = (m_lastSearchIndex - 1 + rowCount) % rowCount;
    } else {
        QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
        if (!selected.isEmpty()) {
            startRow = (selected.first().row() - 1 + rowCount) % rowCount;
        }
    }

    m_tableView->setFocus();
    performSearch(m_lastSearchQuery, startRow, true, false); // false = backward search
}

void MainWindow::addToRecentFiles(const QString &filePath) {
    if (filePath.isEmpty()) return;

    QSettings settings("TableHistory", "TableHistory");
    QStringList files = settings.value("recentFileList").toStringList();

    files.removeAll(filePath);
    files.prepend(filePath);

    while (files.size() > MaxRecentFiles) {
        files.removeLast();
    }

    settings.setValue("recentFileList", files);
    updateRecentFileActions();
}

void MainWindow::updateRecentFileActions() {
    QSettings settings("TableHistory", "TableHistory");
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        // Show file name with keyboard shortcut index (1-8)
        QString text = QString("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        m_recentFileActions[i]->setText(text);
        m_recentFileActions[i]->setData(files[i]);
        m_recentFileActions[i]->setVisible(true);
    }

    for (int j = numRecentFiles; j < MaxRecentFiles; ++j) {
        m_recentFileActions[j]->setVisible(false);
    }

    if (m_recentFilesMenu) {
        m_recentFilesMenu->setEnabled(numRecentFiles > 0);
    }
}

void MainWindow::fileNew() {
    m_model->setItems({});
    updateGanttItems();
    m_notesMemo->clear();
    m_currentFilePath.clear();

    updateStatusBar();

    QSettings settings("TableHistory", "TableHistory");
    settings.remove("lastFilePath");
}

void MainWindow::selectFoundItems() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Find and Select Items"));
    dialog.resize(350, 120);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QLabel *label = new QLabel(tr("Select all items containing text in Name, Place, or Notes:"), &dialog);
    layout->addWidget(label);

    QLineEdit *lineEdit = new QLineEdit(&dialog);
    if (!m_lastSearchQuery.isEmpty()) {
        lineEdit->setText(m_lastSearchQuery);
        lineEdit->selectAll();
    }
    layout->addWidget(lineEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton(tr("&Select"), &dialog);
    okButton->setDefault(true);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), &dialog);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString query = lineEdit->text().trimmed();
    if (query.isEmpty() || m_proxyModel->rowCount() == 0) {
        return;
    }

    m_lastSearchQuery = query;
    if (m_actionFindNext) {
        m_actionFindNext->setEnabled(true);
        m_actionFindPrev->setEnabled(true);
    }

    QItemSelectionModel *selModel = m_tableView->selectionModel();
    selModel->clearSelection();

    QItemSelection selection;
    int matchCount = 0;

    for (int i = 0; i < m_proxyModel->rowCount(); ++i) {
        QModelIndex proxyIndex = m_proxyModel->index(i, 0);
        QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
        const HistoryItem &item = m_model->getItem(sourceIndex.row());

        QTextDocument notesDoc;
        notesDoc.setHtml(item.notes);
        QString plainNotes = notesDoc.toPlainText();

        if (item.name.contains(query, Qt::CaseInsensitive) ||
            item.place.contains(query, Qt::CaseInsensitive) ||
            plainNotes.contains(query, Qt::CaseInsensitive)) {

            QModelIndex leftIndex = m_proxyModel->index(i, 0);
            QModelIndex rightIndex = m_proxyModel->index(i, m_proxyModel->columnCount() - 1);
            selection.select(leftIndex, rightIndex);
            matchCount++;
        }
    }

    if (matchCount > 0) {
        selModel->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        if (!selection.indexes().isEmpty()) {
            m_tableView->scrollTo(selection.indexes().first(), QAbstractItemView::PositionAtCenter);
        }
        m_tableView->setFocus();
    }

    QMessageBox::information(this, tr("Select Found Items"), tr("Found and selected %1 matching item(s).").arg(matchCount));
}

void MainWindow::fileOpen() {
    QString initialPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString path = QFileDialog::getOpenFileName(this, "Open Historical Data", initialPath, "CSV Files (*.csv)");
    if (!path.isEmpty()) {
        loadCsvFile(path);
    }
}

void MainWindow::fileSave() {
    if (m_currentFilePath.isEmpty()) {
        fileSaveAs();
    } else {
        saveCsvFile(m_currentFilePath);
    }
}

void MainWindow::fileSaveAs() {
    QString initialPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/history.csv";
    QString path = QFileDialog::getSaveFileName(this, "Save Historical Data", initialPath, "CSV Files (*.csv)");
    if (!path.isEmpty()) {
        saveCsvFile(path);
    }
}

static QStringList parseCsvLine(const QString &line) {
    QStringList fields;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < line.size(); ++i) {
        QChar ch = line.at(i);
        if (ch == '"') {
            if (inQuotes && i + 1 < line.size() && line.at(i + 1) == '"') {
                current += '"';
                i++;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (ch == ',' && !inQuotes) {
            fields.append(current);
            current.clear();
        } else {
            current += ch;
        }
    }
    fields.append(current);
    return fields;
}

void MainWindow::loadCsvFile(const QString &filePath) {
    m_notesMemo->clear();
    m_notesMemo->setEnabled(false);
    if (m_actionCopy) m_actionCopy->setEnabled(false);
    if (m_actionSelectAll) m_actionSelectAll->setEnabled(false);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open file for reading.");
        return;
    }

    QTextStream in(&file);
    QList<HistoryItem> items;

    bool firstLine = true;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        if (firstLine && line.contains("Kind", Qt::CaseInsensitive)) {
            firstLine = false;
            continue;
        }
        firstLine = false;

        QStringList fields = parseCsvLine(line);
        if (fields.size() >= 8) {
            items.append(HistoryItem::fromCsvRecord(fields));
        }
    }
    file.close();

    m_model->setItems(items);
    updateGanttItems();
    m_currentFilePath = filePath;

    updateStatusBar();

    QSettings settings("TableHistory", "TableHistory");
    settings.setValue("lastFilePath", filePath);

    m_currentFilePath = filePath;
    updateStatusBar();

    addToRecentFiles(filePath);}

bool MainWindow::saveCsvFile(const QString &filePath, bool showConfirmation) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open file for writing.");
        return false;
    }

    QTextStream out(&file);
    out << "Kind,Name,StartYear,StartUncertain,EndYear,EndUncertain,Place,Notes\n";

    for (const auto &item : m_model->items()) {
        out << item.toCsvRecord() << "\n";
    }
    file.close();

    m_currentFilePath = filePath;

    updateStatusBar();

    QSettings settings("TableHistory", "TableHistory");
    settings.setValue("lastFilePath", filePath);

    m_currentFilePath = filePath;
    updateStatusBar();

    addToRecentFiles(filePath);

    if (showConfirmation) {
        // QMessageBox::information(this, "Saved", "Data successfully saved.");
    }
    return true;
}

void MainWindow::updateStatusBar() {
    int itemCount = m_model->rowCount();
    QString statusText;

    if (!m_currentFilePath.isEmpty()) {
        statusText = QString("File: %1  |  Items: %2").arg(m_currentFilePath).arg(itemCount);
    } else {
        statusText = QString("Items: %1").arg(itemCount);
    }

    QList<QLabel*> labels = statusBar()->findChildren<QLabel*>();
    if (!labels.isEmpty()) {
        labels.first()->setText(statusText);
    }
}