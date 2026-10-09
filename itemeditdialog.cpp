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

#include "itemeditdialog.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QKeySequence>
#include <QTextBlockFormat>
#include <QGuiApplication>
#include <QScreen>
#include <QStyleHints>

ItemEditDialog::ItemEditDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Add Item");
    setupUi();
}

ItemEditDialog::ItemEditDialog(const HistoryItem &item, QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Edit Item");
    setupUi();
    loadItemData(item);
}

void ItemEditDialog::setupUi() {
    bool isDarkMode = false;
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    isDarkMode = (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark);
#else
    isDarkMode = (palette().color(QPalette::Window).lightness() < 128);
#endif
    auto *layout = new QVBoxLayout(this);
    auto *formLayout = new QFormLayout();

    m_kindCombo = new QComboBox(this);
    m_kindCombo->addItem("Person", static_cast<int>(ItemKind::Person));
    m_kindCombo->addItem("Document", static_cast<int>(ItemKind::Document));
    m_kindCombo->addItem("Event", static_cast<int>(ItemKind::Event));

    m_nameEdit = new QLineEdit(this);
    int fieldWidth = m_nameEdit->sizeHint().width() * 3;
    m_nameEdit->setMinimumWidth(fieldWidth);
    if (isDarkMode) {
        m_nameEdit->setStyleSheet("QLineEdit { background-color: #2b2b2b; color: #ffffff; border: 1px solid #555555; border-radius: 4px; padding: 2px; }");
    } else {
        m_nameEdit->setStyleSheet("QLineEdit { border-radius: 4px; padding: 2px; }");
    }

    auto *startLayout = new QHBoxLayout();
    m_startYearSpin = new QSpinBox(this);
    m_startYearSpin->setRange(0, 2500);
    m_startYearSpin->setSpecialValueText(" ");
    m_startUncertainCheck = new QCheckBox("Uncertain (?)", this);
    startLayout->addWidget(m_startYearSpin);
    startLayout->addWidget(m_startUncertainCheck);

    auto *endLayout = new QHBoxLayout();
    m_endYearSpin = new QSpinBox(this);
    m_endYearSpin->setRange(0, 2500);
    m_endYearSpin->setSpecialValueText(" ");
    m_endUncertainCheck = new QCheckBox("Uncertain (?)", this);
    endLayout->addWidget(m_endYearSpin);
    endLayout->addWidget(m_endUncertainCheck);

    m_placeEdit = new QLineEdit(this);
    m_placeEdit->setMinimumWidth(fieldWidth);
    if (isDarkMode) {
      m_placeEdit->setStyleSheet("QLineEdit { background-color: #2b2b2b; color: #ffffff; border: 1px solid #555555; border-radius: 4px; padding: 2px; }");
    } else {
        m_placeEdit->setStyleSheet("QLineEdit { border-radius: 4px; padding: 2px; }");
    }

    // Notes Formatting Section
    auto *notesContainer = new QWidget(this);
    auto *notesLayout = new QVBoxLayout(notesContainer);
    notesLayout->setContentsMargins(0, 0, 0, 0);

    auto *notesHeaderLayout = new QHBoxLayout();

    m_h1Btn = new QToolButton(this);
    m_h1Btn->setText("H1");
    m_h1Btn->setCheckable(true);
    m_h1Btn->setToolTip("Header 1");
    m_h1Btn->setStyleSheet("QToolButton { font-weight: bold; padding: 2px 6px; }");

    m_h2Btn = new QToolButton(this);
    m_h2Btn->setText("H2");
    m_h2Btn->setCheckable(true);
    m_h2Btn->setToolTip("Header 2");
    m_h2Btn->setStyleSheet("QToolButton { font-weight: bold; padding: 2px 6px; }");

    m_h3Btn = new QToolButton(this);
    m_h3Btn->setText("H3");
    m_h3Btn->setCheckable(true);
    m_h3Btn->setToolTip("Header 3");
    m_h3Btn->setStyleSheet("QToolButton { font-weight: bold; padding: 2px 6px; }");

    m_boldBtn = new QToolButton(this);
    m_boldBtn->setText("B");
    m_boldBtn->setCheckable(true);
    m_boldBtn->setShortcut(QKeySequence::Bold);
    m_boldBtn->setToolTip("Bold (Ctrl+B)");
    m_boldBtn->setStyleSheet("QToolButton { font-weight: bold; padding: 2px 8px; }");

    m_italicBtn = new QToolButton(this);
    m_italicBtn->setText("I");
    m_italicBtn->setCheckable(true);
    m_italicBtn->setShortcut(QKeySequence::Italic);
    m_italicBtn->setToolTip("Italic (Ctrl+I)");
    m_italicBtn->setStyleSheet("QToolButton { font-style: italic; padding: 2px 8px; }");

    m_clearFmtBtn = new QToolButton(this);
    m_clearFmtBtn->setText("Tx");
    m_clearFmtBtn->setToolTip("Clear Formatting");
    m_clearFmtBtn->setStyleSheet("QToolButton { padding: 2px 6px; }");

    notesHeaderLayout->addWidget(m_h1Btn);
    notesHeaderLayout->addWidget(m_h2Btn);
    notesHeaderLayout->addWidget(m_h3Btn);
    notesHeaderLayout->addWidget(m_boldBtn);
    notesHeaderLayout->addWidget(m_italicBtn);
    notesHeaderLayout->addWidget(m_clearFmtBtn);
    notesHeaderLayout->addStretch();

    m_notesEdit = new NotesTextEdit(this);
    m_notesEdit->setAcceptRichText(true);
    QFont memoFont = m_notesEdit->font();
    memoFont.setPointSize(16);
    memoFont.setFamily("Avenir Next");
    m_notesEdit->setFont(memoFont);
    m_notesEdit->document()->setDefaultFont(memoFont);
    m_notesEdit->setMinimumHeight(120);
    if (isDarkMode) {
      m_notesEdit->setStyleSheet("QTextEdit { background-color: #2b2b2b; color: #ffffff; border: 1px solid #555555; border-radius: 4px; }");
    }

    notesLayout->addLayout(notesHeaderLayout);
    notesLayout->addWidget(m_notesEdit);

    formLayout->addRow("Kind:", m_kindCombo);
    formLayout->addRow("Name:", m_nameEdit);
    formLayout->addRow("Start Year:", startLayout);
    formLayout->addRow("End Year:", endLayout);
    formLayout->addRow("Place:", m_placeEdit);

    layout->addLayout(formLayout);

    // Full-width Notes
    auto *notesLabel = new QLabel("Notes:", this);
    layout->addWidget(notesLabel);
    layout->addWidget(notesContainer);

    auto *buttonLayout = new QHBoxLayout();
    auto *okBtn = new QPushButton("OK", this);
    auto *cancelBtn = new QPushButton("Cancel", this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);

    layout->addLayout(buttonLayout);

    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    connect(m_boldBtn, &QToolButton::toggled, this, &ItemEditDialog::onBoldToggled);
    connect(m_italicBtn, &QToolButton::toggled, this, &ItemEditDialog::onItalicToggled);
    connect(m_h1Btn, &QToolButton::clicked, this, &ItemEditDialog::onH1Clicked);
    connect(m_h2Btn, &QToolButton::clicked, this, &ItemEditDialog::onH2Clicked);
    connect(m_h3Btn, &QToolButton::clicked, this, &ItemEditDialog::onH3Clicked);
    connect(m_clearFmtBtn, &QToolButton::clicked, this, &ItemEditDialog::onClearFmtClicked);

    connect(m_notesEdit, &QTextEdit::currentCharFormatChanged, this, &ItemEditDialog::updateFormatButtons);
    connect(m_notesEdit, &QTextEdit::cursorPositionChanged, this, &ItemEditDialog::updateFormatButtons);

    layout->activate();
    QSize baseSize = sizeHint();
    int targetWidth = baseSize.width() * 1.5;
    int targetHeight = baseSize.height() * 1.3;

    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        targetWidth = qMin(targetWidth, screen->availableGeometry().width());
        targetHeight = qMin(targetHeight, screen->availableGeometry().height());
    }

    resize(targetWidth, targetHeight);
}

void ItemEditDialog::loadItemData(const HistoryItem &item) {
    int kindIndex = m_kindCombo->findData(static_cast<int>(item.kind));
    if (kindIndex != -1) m_kindCombo->setCurrentIndex(kindIndex);

    m_nameEdit->setText(item.name);
    m_startYearSpin->setValue(item.startYear);
    m_startUncertainCheck->setChecked(item.startUncertain);
    m_endYearSpin->setValue(item.endYear);
    m_endUncertainCheck->setChecked(item.endUncertain);
    m_placeEdit->setText(item.place);
    m_notesEdit->setHtml(item.notes);

    updateFormatButtons();
}

HistoryItem ItemEditDialog::getItem() const {
    HistoryItem item;
    item.kind = static_cast<ItemKind>(m_kindCombo->currentData().toInt());
    item.name = m_nameEdit->text();
    item.startYear = m_startYearSpin->value();
    item.startUncertain = m_startUncertainCheck->isChecked();
    item.endYear = m_endYearSpin->value();
    item.endUncertain = m_endUncertainCheck->isChecked();
    item.place = m_placeEdit->text();

    // --- STRIP NEWLINES SO HTML STAYS ON A SINGLE CSV LINE ---
    QString htmlNotes = m_notesEdit->toHtml();
    htmlNotes.remove('\n').remove('\r');
    item.notes = htmlNotes;

    return item;
}

void ItemEditDialog::applyHeadingLevel(int level) {
    QTextCursor cursor = m_notesEdit->textCursor();
    cursor.beginEditBlock();

    QTextBlockFormat blockFmt = cursor.blockFormat();
    int currentLevel = blockFmt.headingLevel();
    int newLevel = (currentLevel == level) ? 0 : level;

    blockFmt.setHeadingLevel(newLevel);
    cursor.setBlockFormat(blockFmt);

    qreal defaultSize = m_notesEdit->font().pointSizeF();
    if (defaultSize <= 0) {
        defaultSize = 10.0;
    }

    QTextCharFormat charFmt;
    if (newLevel == 1) {
        charFmt.setFontPointSize(defaultSize * 2.0); // ~2x scale
        charFmt.setFontWeight(QFont::Bold);
    } else if (newLevel == 2) {
        charFmt.setFontPointSize(defaultSize * 1.5); // ~1.5x scale
        charFmt.setFontWeight(QFont::Bold);
    } else if (newLevel == 3) {
        charFmt.setFontPointSize(defaultSize * 1.17); // ~1.17x scale
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

    m_notesEdit->setTextCursor(cursor); // Forces immediate update
    updateFormatButtons();
}

static void clearTextFormatting(QTextEdit *editor) {
    QTextCursor cursor = editor->textCursor();
    cursor.beginEditBlock();

    bool hasSel = cursor.hasSelection();
    int startPos = hasSel ? cursor.selectionStart() : cursor.position();
    int endPos = hasSel ? cursor.selectionEnd() : cursor.position();

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

    qreal defaultSize = editor->font().pointSizeF();
    if (defaultSize <= 0) {
        defaultSize = 10.0;
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

    editor->setTextCursor(cursor); // Forces immediate update
}

void ItemEditDialog::onClearFmtClicked() {
    clearTextFormatting(m_notesEdit);
    updateFormatButtons();
}

void ItemEditDialog::onBoldToggled(bool checked) {
    QTextCharFormat fmt;
    fmt.setFontWeight(checked ? QFont::Bold : QFont::Normal);
    m_notesEdit->mergeCurrentCharFormat(fmt);
}

void ItemEditDialog::onItalicToggled(bool checked) {
    QTextCharFormat fmt;
    fmt.setFontItalic(checked);
    m_notesEdit->mergeCurrentCharFormat(fmt);
}

void ItemEditDialog::onH1Clicked() { applyHeadingLevel(1); }
void ItemEditDialog::onH2Clicked() { applyHeadingLevel(2); }
void ItemEditDialog::onH3Clicked() { applyHeadingLevel(3); }

void ItemEditDialog::updateFormatButtons() {
    QTextCharFormat format = m_notesEdit->currentCharFormat();
    QTextBlockFormat blockFormat = m_notesEdit->textCursor().blockFormat();
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
