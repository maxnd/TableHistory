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
#include <QFileDialog>
#include <QPixmap>

static QStringList parseCsvLine(const QString &line);

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

    if (isDarkMode) {
        tableViewStyle += QString("QAbstractScrollArea::viewport { background-color: %1; }").arg(formColor.name());
    }

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

    m_tableView->setStyleSheet(tableViewStyle);

    m_tableView->horizontalHeader()->setFixedHeight(HEADER_HEIGHT);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    m_tableView->verticalHeader()->setDefaultSectionSize(ROW_HEIGHT);
    m_tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_tableView->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

    m_tableView->setColumnHidden(HistoryTableModel::ColNotes, true);

    m_leftSplitter->addWidget(m_tableView);

    // 2. Notes Section Container with Bold, Italic, Header, and Clear Formatting Controls
    auto *notesContainer = new QWidget(this);
    auto *notesLayout = new QVBoxLayout(notesContainer);
    notesLayout->setContentsMargins(0, 6, 0, 6);

    auto *notesHeaderLayout = new QHBoxLayout();
    auto *notesLabel = new QLabel("<b>Item Notes:</b>", this);
    notesHeaderLayout->addSpacing(6);

    m_h1Btn = new QToolButton(this);
    m_h1Btn->setText("H1");
    m_h1Btn->setCheckable(true);
    m_h1Btn->setToolTip("Header 1");
    m_h1Btn->setStyleSheet("QToolButton { font-weight: bold; padding: 2px 6px; }");
    m_h1Btn->setEnabled(false);

    m_h2Btn = new QToolButton(this);
    m_h2Btn->setText("H2");
    m_h2Btn->setCheckable(true);
    m_h2Btn->setToolTip("Header 2");
    m_h2Btn->setStyleSheet("QToolButton { font-weight: bold; padding: 2px 6px; }");
    m_h2Btn->setEnabled(false);

    m_h3Btn = new QToolButton(this);
    m_h3Btn->setText("H3");
    m_h3Btn->setCheckable(true);
    m_h3Btn->setToolTip("Header 3");
    m_h3Btn->setStyleSheet("QToolButton { font-weight: bold; padding: 2px 6px; }");
    m_h3Btn->setEnabled(false);

    m_boldBtn = new QToolButton(this);
    m_boldBtn->setText("B");
    m_boldBtn->setCheckable(true);
    m_boldBtn->setShortcut(QKeySequence::Bold);
    m_boldBtn->setToolTip("Bold (Ctrl+B)");
    m_boldBtn->setStyleSheet("QToolButton { font-weight: bold; padding: 2px 8px; }");
    m_boldBtn->setEnabled(false);

    m_italicBtn = new QToolButton(this);
    m_italicBtn->setText("I");
    m_italicBtn->setCheckable(true);
    m_italicBtn->setShortcut(QKeySequence::Italic);
    m_italicBtn->setToolTip("Italic (Ctrl+I)");
    m_italicBtn->setStyleSheet("QToolButton { font-style: italic; padding: 2px 8px; }");
    m_italicBtn->setEnabled(false);

    m_clearFmtBtn = new QToolButton(this);
    m_clearFmtBtn->setText("Tx");
    m_clearFmtBtn->setToolTip("Clear Formatting");
    m_clearFmtBtn->setStyleSheet("QToolButton { padding: 2px 6px; }");
    m_clearFmtBtn->setEnabled(false);

    notesHeaderLayout->addWidget(notesLabel);
    notesHeaderLayout->addWidget(m_h1Btn);
    notesHeaderLayout->addWidget(m_h2Btn);
    notesHeaderLayout->addWidget(m_h3Btn);
    notesHeaderLayout->addWidget(m_boldBtn);
    notesHeaderLayout->addWidget(m_italicBtn);
    notesHeaderLayout->addWidget(m_clearFmtBtn);
    notesHeaderLayout->addStretch();

    m_notesMemo = new NotesTextEdit(this);
    QFont memoFont = m_notesMemo->font();
    memoFont.setPointSize(16);
    memoFont.setFamily("Avenir Next");
    m_notesMemo->setFont(memoFont);
    m_notesMemo->document()->setDefaultFont(memoFont);
    m_notesMemo->setAcceptRichText(true);
    m_notesMemo->setMinimumHeight(120);
    if (isDarkMode) {
      m_notesMemo->setStyleSheet("QTextEdit { background-color: #2b2b2b; color: #ffffff; border: 1px solid #555555; border-radius: 4px; }");
    }
    m_notesMemo->setEnabled(false);

    notesLayout->addLayout(notesHeaderLayout);
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

    // Notes formatting connections
    connect(m_boldBtn, &QToolButton::toggled, this, &MainWindow::onNotesBoldToggled);
    connect(m_italicBtn, &QToolButton::toggled, this, &MainWindow::onNotesItalicToggled);
    connect(m_h1Btn, &QToolButton::clicked, this, &MainWindow::onNotesH1Clicked);
    connect(m_h2Btn, &QToolButton::clicked, this, &MainWindow::onNotesH2Clicked);
    connect(m_h3Btn, &QToolButton::clicked, this, &MainWindow::onNotesH3Clicked);
    connect(m_clearFmtBtn, &QToolButton::clicked, this, &MainWindow::onNotesClearFmtClicked);

    connect(m_notesMemo, &QTextEdit::currentCharFormatChanged, this, &MainWindow::updateNotesFormatButtons);
    connect(m_notesMemo, &QTextEdit::cursorPositionChanged, this, &MainWindow::updateNotesFormatButtons);
    connect(m_notesMemo, &QTextEdit::textChanged, this, &MainWindow::onNotesTextChanged);
}

void MainWindow::zoomInInterface() {
    QFont font = qApp->font();
    int newSize = font.pointSize() + 1;
    if (newSize <= 14) {
        font.setPointSize(newSize);
        qApp->setFont(font);
        this->setFont(font);

        // Explicitly update the grid and its headers
        m_tableView->setFont(font);
        m_tableView->horizontalHeader()->setFont(font);
        m_tableView->verticalHeader()->setFont(font);
        m_tableView->style()->polish(m_tableView); // Forces style refresh
    }
}

void MainWindow::zoomOutInterface() {
    QFont font = qApp->font();
    int newSize = font.pointSize() - 1;
    if (newSize >= 8) {
        font.setPointSize(newSize);
        qApp->setFont(font);
        this->setFont(font);

        // Explicitly update the grid and its headers
        m_tableView->setFont(font);
        m_tableView->horizontalHeader()->setFont(font);
        m_tableView->verticalHeader()->setFont(font);
        m_tableView->style()->polish(m_tableView);
    }
}

void MainWindow::resetInterfaceZoom() {
    QFont font = qApp->font();
    font.setPointSize(m_baseFontSize); // Uses your baseline size
    qApp->setFont(font);
    this->setFont(font);

    // Explicitly update the grid and its headers
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
    QAction *exportAction = fileMenu->addAction(tr("&Export diagram..."));
    connect(exportAction, &QAction::triggered, this, &MainWindow::exportDiagram);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", QKeySequence::Quit, this, &QWidget::close);

    // --- Edit Menu (for Notes Text Field) ---
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    m_actionCopy = editMenu->addAction(tr("&Copy"), QKeySequence::Copy, m_notesMemo, &QTextEdit::copy);
    m_actionCut = editMenu->addAction(tr("Cu&t"), QKeySequence::Cut, m_notesMemo, &QTextEdit::cut);
    m_actionPaste = editMenu->addAction(tr("&Paste"), QKeySequence::Paste, m_notesMemo, &QTextEdit::paste);
    editMenu->addSeparator();
    m_actionSelectAll = editMenu->addAction(tr("Select &All"), QKeySequence::SelectAll, m_notesMemo, &QTextEdit::selectAll);
    // Initially disabled until a row with notes is selected
    m_actionCopy->setEnabled(false);
    m_actionCut->setEnabled(false);
    m_actionPaste->setEnabled(false);
    m_actionSelectAll->setEnabled(false);

    QMenu *itemsMenu = menuBar()->addMenu("&Items");
    itemsMenu->addAction("&Add Item...", QKeySequence::New, this, &MainWindow::itemAdd);
    itemsMenu->addAction("&Edit Item...", QKeySequence(Qt::CTRL | Qt::Key_E), this, &MainWindow::itemEdit);
    itemsMenu->addAction("&Delete Items", QKeySequence::Delete, this, &MainWindow::itemDelete);
    itemsMenu->addSeparator();
    itemsMenu->addAction("&Copy selected", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C), this, &MainWindow::itemCopy);
    itemsMenu->addAction("&Paste from selection", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V), this, &MainWindow::itemPaste);
    itemsMenu->addSeparator();
    m_actionFind = itemsMenu->addAction(tr("&Find..."), QKeySequence::Find, this, &MainWindow::findItem);
    m_actionFindNext = itemsMenu->addAction(tr("Find &Next"), QKeySequence(Qt::ALT | Qt::Key_F), this, &MainWindow::findNextItem);
    m_actionFindNext->setEnabled(false);

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
    m_isUpdatingNotes = true;
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    bool hasSelection = !selected.isEmpty();

    // Enable/disable text edit actions based on selection state
    if (m_actionCopy) m_actionCopy->setEnabled(hasSelection);
    if (m_actionCut) m_actionCut->setEnabled(hasSelection);
    if (m_actionPaste) m_actionPaste->setEnabled(hasSelection);
    if (m_actionSelectAll) m_actionSelectAll->setEnabled(hasSelection);

    if (hasSelection) {
        int proxyRow = selected.first().row();
        QModelIndex sourceIndex = m_proxyModel->mapToSource(selected.first());
        int row = sourceIndex.row();
        const auto &item = m_model->getItem(row);

        m_notesMemo->setEnabled(true);
        m_boldBtn->setEnabled(true);
        m_italicBtn->setEnabled(true);
        m_h1Btn->setEnabled(true);
        m_h2Btn->setEnabled(true);
        m_h3Btn->setEnabled(true);
        m_clearFmtBtn->setEnabled(true);

        m_notesMemo->setHtml(item.notes);

        m_ganttWidget->setSelectedIndex(proxyRow);
        m_ganttWidget->scrollToItem(proxyRow);
    } else {
        m_notesMemo->clear();
        m_notesMemo->setEnabled(false);
        m_boldBtn->setEnabled(false);
        m_italicBtn->setEnabled(false);
        m_h1Btn->setEnabled(false);
        m_h2Btn->setEnabled(false);
        m_h3Btn->setEnabled(false);
        m_clearFmtBtn->setEnabled(false);
        m_ganttWidget->setSelectedIndex(-1);
    }
    m_isUpdatingNotes = false;
    updateNotesFormatButtons();
}

void MainWindow::applyHeadingLevel(QTextEdit *editor, int level) {
    QTextCursor cursor = editor->textCursor();
    cursor.beginEditBlock();

    QTextBlockFormat blockFmt = cursor.blockFormat();
    int currentLevel = blockFmt.headingLevel();
    int newLevel = (currentLevel == level) ? 0 : level;

    blockFmt.setHeadingLevel(newLevel);
    cursor.setBlockFormat(blockFmt);

    qreal defaultSize = editor->font().pointSizeF();
    if (defaultSize <= 0) {
        defaultSize = 10.0;
    }

    QTextCharFormat charFmt;
    if (newLevel == 1) {
        charFmt.setFontPointSize(defaultSize * 2.0); // Match standard H1 scale (~2x)
        charFmt.setFontWeight(QFont::Bold);
    } else if (newLevel == 2) {
        charFmt.setFontPointSize(defaultSize * 1.5); // Match standard H2 scale (~1.5x)
        charFmt.setFontWeight(QFont::Bold);
    } else if (newLevel == 3) {
        charFmt.setFontPointSize(defaultSize * 1.17); // Match standard H3 scale (~1.17x)
        charFmt.setFontWeight(QFont::Bold);
    } else {
        charFmt.setFontPointSize(defaultSize);
        charFmt.setFontWeight(QFont::Normal);
    }

    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::BlockUnderCursor);
    }

    cursor.mergeCharFormat(charFmt);
    cursor.endEditBlock();

    editor->setTextCursor(cursor);
    updateNotesFormatButtons();
}

static void clearTextFormatting(QTextEdit *editor) {
    QTextCursor cursor = editor->textCursor();
    cursor.beginEditBlock();

    bool hasSel = cursor.hasSelection();
    int startPos = hasSel ? cursor.selectionStart() : cursor.position();
    int endPos = hasSel ? cursor.selectionEnd() : cursor.position();

    // Reset heading levels on all selected blocks
    QTextBlock startBlock = editor->document()->findBlock(startPos);
    QTextBlock endBlock = editor->document()->findBlock(qMax(startPos, endPos > startPos ? endPos - 1 : endPos));

    QTextBlock block = startBlock;
    while (block.isValid()) {
        QTextCursor blockCursor(block);
        QTextBlockFormat blockFmt = blockCursor.blockFormat();
        blockFmt.setHeadingLevel(0);
        blockCursor.setBlockFormat(blockFmt);

        if (block == endBlock) break;
        block = block.next();
    }

    int defaultSize = editor->font().pointSize();
    if (defaultSize <= 0) {
        defaultSize = 10;
    }

    QTextCharFormat cleanFmt;
    cleanFmt.setFontPointSize(defaultSize);
    cleanFmt.setFontWeight(QFont::Normal);
    cleanFmt.setFontItalic(false);
    cleanFmt.setFontUnderline(false);

    if (!hasSel) {
        cursor.select(QTextCursor::BlockUnderCursor);
    }

    cursor.setCharFormat(cleanFmt);
    cursor.endEditBlock();
    editor->setTextCursor(cursor);
}

void MainWindow::onNotesClearFmtClicked() {
    clearTextFormatting(m_notesMemo);
    updateNotesFormatButtons();
}

void MainWindow::onNotesBoldToggled(bool checked) {
    QTextCharFormat fmt;
    fmt.setFontWeight(checked ? QFont::Bold : QFont::Normal);
    m_notesMemo->mergeCurrentCharFormat(fmt);
}

void MainWindow::onNotesItalicToggled(bool checked) {
    QTextCharFormat fmt;
    fmt.setFontItalic(checked);
    m_notesMemo->mergeCurrentCharFormat(fmt);
}

void MainWindow::onNotesH1Clicked() { applyHeadingLevel(m_notesMemo, 1); }
void MainWindow::onNotesH2Clicked() { applyHeadingLevel(m_notesMemo, 2); }
void MainWindow::onNotesH3Clicked() { applyHeadingLevel(m_notesMemo, 3); }

void MainWindow::updateNotesFormatButtons() {
    QTextCharFormat format = m_notesMemo->currentCharFormat();
    QTextBlockFormat blockFormat = m_notesMemo->textCursor().blockFormat();
    int headingLevel = blockFormat.headingLevel();

    m_boldBtn->blockSignals(true);
    m_italicBtn->blockSignals(true);
    m_h1Btn->blockSignals(true);
    m_h2Btn->blockSignals(true);
    m_h3Btn->blockSignals(true);

    m_boldBtn->setChecked(format.fontWeight() == QFont::Bold);
    m_italicBtn->setChecked(format.fontItalic());
    m_h1Btn->setChecked(headingLevel == 1);
    m_h2Btn->setChecked(headingLevel == 2);
    m_h3Btn->setChecked(headingLevel == 3);

    m_boldBtn->blockSignals(false);
    m_italicBtn->blockSignals(false);
    m_h1Btn->blockSignals(false);
    m_h2Btn->blockSignals(false);
    m_h3Btn->blockSignals(false);
}

void MainWindow::onNotesTextChanged() {
    if (m_isUpdatingNotes) return;

    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (!selected.isEmpty()) {
        int row = m_proxyModel->mapToSource(selected.first()).row(); // Mappatura corretta
        HistoryItem item = m_model->getItem(row);

        QString htmlNotes = m_notesMemo->toHtml();
        htmlNotes.remove('\n').remove('\r');

        item.notes = htmlNotes;
        m_model->updateItem(row, item);
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
        updateGanttItems();
    }
}

void MainWindow::itemEdit() {
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Edit Item", "Please select an item to edit.");
        return;
    }

    int row = m_proxyModel->mapToSource(selected.first()).row(); // Mappatura corretta
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
    }
}

void MainWindow::itemCopy() {
    QModelIndexList selectedRows = m_tableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        return;
    }

    // Unique identifier header to recognize TableHistory items
    QString clipText = "TableHistoryItems:\n";
    clipText += "Kind,Name,StartYear,StartUncertain,EndYear,EndUncertain,Place,Notes\n";

    // Collect and sort source rows to preserve order
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

    // Append pasted items to existing ones
    QList<HistoryItem> items = m_model->items();
    items.append(newItems);
    m_model->setItems(items);
    updateGanttItems();

    QMessageBox::information(this, "Paste Items", QString("%1 item(s) successfully pasted.").arg(newItems.size()));
}

#include <QTextDocument>

void MainWindow::performSearch(const QString &query, int startIndex, bool wrap) {
    if (query.isEmpty() || m_proxyModel->rowCount() == 0) return;

    int rowCount = m_proxyModel->rowCount();
    int currentIndex = startIndex;
    bool found = false;

    // Loop through rows starting from startIndex
    for (int step = 0; step < rowCount; ++step) {
        int i = (currentIndex + step) % rowCount;

        // If we wrapped around and don't want to re-search or already finished a round
        if (!wrap && step > 0 && i <= startIndex) break;

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

            // Set current index and select the full row cleanly
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
        QMessageBox::information(this, tr("Find"), tr("No more matching items found for '%1'.").arg(query));
    }
}

void MainWindow::exportDiagram() {
    if (!m_ganttWidget) return;

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export Gantt Diagram"),
        "",
        tr("Bitmap Files (*.bmp);;All Files (*)")
        );

    if (fileName.isEmpty())
        return;

    // Ensure the filename ends with .bmp
    if (!fileName.endsWith(".bmp", Qt::CaseInsensitive)) {
        fileName += ".bmp";
    }

    // Render the full Gantt widget onto a QPixmap
    QPixmap pixmap(m_ganttWidget->size());
    m_ganttWidget->render(&pixmap);

    // Save as BMP
    if (!pixmap.save(fileName, "BMP")) {
        // Optional: Handle save failure if needed
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

    // Determine starting index: start from current selected row + 1, or 0 if none
    int startRow = 0;
    QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
    if (!selected.isEmpty()) {
        startRow = (selected.first().row() + 1) % m_proxyModel->rowCount();
    }

    performSearch(m_lastSearchQuery, startRow, true);
}

void MainWindow::findNextItem() {
    if (m_lastSearchQuery.isEmpty()) {
        findItem();
        return;
    }

    // Start looking from the row immediately after the last found match
    int startRow = 0;
    if (m_lastSearchIndex >= 0) {
        startRow = (m_lastSearchIndex + 1) % m_proxyModel->rowCount();
    } else {
        QModelIndexList selected = m_tableView->selectionModel()->selectedRows();
        if (!selected.isEmpty()) {
            startRow = (selected.first().row() + 1) % m_proxyModel->rowCount();
        }
    }

    performSearch(m_lastSearchQuery, startRow, true);

}

void MainWindow::fileNew() {
    m_model->setItems({});
    updateGanttItems();
    m_notesMemo->clear();
    m_currentFilePath.clear();

    QList<QLabel*> labels = statusBar()->findChildren<QLabel*>();
    if (!labels.isEmpty()) {
        labels.first()->clear();
    }

    QSettings settings("TableHistory", "TableHistory");
    settings.remove("lastFilePath");
}

void MainWindow::fileOpen() {
    QString path = QFileDialog::getOpenFileName(this, "Open Historical Data", QString(), "CSV Files (*.csv)");
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
    QString path = QFileDialog::getSaveFileName(this, "Save Historical Data", "history.csv", "CSV Files (*.csv)");
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
    // Clear and disable the notes memo and its controls when opening a file
    m_notesMemo->clear();
    m_notesMemo->setEnabled(false);
    if (m_boldBtn) m_boldBtn->setEnabled(false);
    if (m_italicBtn) m_italicBtn->setEnabled(false);
    if (m_h1Btn) m_h1Btn->setEnabled(false);
    if (m_h2Btn) m_h2Btn->setEnabled(false);
    if (m_h3Btn) m_h3Btn->setEnabled(false);
    if (m_clearFmtBtn) m_clearFmtBtn->setEnabled(false);
    if (m_actionCopy) m_actionCopy->setEnabled(false);
    if (m_actionCut) m_actionCut->setEnabled(false);
    if (m_actionPaste) m_actionPaste->setEnabled(false);
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

    QList<QLabel*> labels = statusBar()->findChildren<QLabel*>();
    if (!labels.isEmpty()) {
        labels.first()->setText(QString("File: %1").arg(filePath));
    }

    QSettings settings("TableHistory", "TableHistory");
    settings.setValue("lastFilePath", filePath);
}

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

    QList<QLabel*> labels = statusBar()->findChildren<QLabel*>();
    if (!labels.isEmpty()) {
        labels.first()->setText(QString("File: %1").arg(filePath));
    }

    QSettings settings("TableHistory", "TableHistory");
    settings.setValue("lastFilePath", filePath);

    if (showConfirmation) {
        QMessageBox::information(this, "Saved", "Data successfully saved.");
    }
    return true;
}
