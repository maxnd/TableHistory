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
#include "mainwindow.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QKeySequence>
#include <QShortcut>
#include <QTextBlockFormat>
#include <QGuiApplication>
#include <QScreen>
#include <QStyleHints>
#include <QLineEdit>
#include <QKeyEvent>
#include <utility>

static QStringList s_cachedTags;

class SearchLineEdit : public QLineEdit {
public:
    explicit SearchLineEdit(QWidget *parent = nullptr) : QLineEdit(parent) {}
protected:
    void keyPressEvent(QKeyEvent *e) override {
        QLineEdit::keyPressEvent(e);
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
            e->accept(); // Marks the event as handled, preventing the dialog from closing
        }
    }
};

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
#ifdef Q_OS_APPLE
    if (isDarkMode) {
        m_kindCombo->setStyleSheet("QLineEdit { background-color: #2b2b2b; color: #ffffff; border: 1px solid #555555; border-radius: 4px; padding: 2px; }");
    } else {
        m_kindCombo->setStyleSheet("QLineEdit { border-radius: 4px; padding: 2px; }");
    }
#endif

    m_nameEdit = new QLineEdit(this);
    int fieldWidth = m_nameEdit->sizeHint().width() * 3;
    m_nameEdit->setMinimumWidth(fieldWidth);

#ifdef Q_OS_APPLE
    if (isDarkMode) {
        m_nameEdit->setStyleSheet("QLineEdit { background-color: #2b2b2b; color: #ffffff; border: 1px solid #555555; border-radius: 4px; padding: 2px; }");
    } else {
        m_nameEdit->setStyleSheet("QLineEdit { border-radius: 4px; padding: 2px; }");
    }
#endif

    auto *startLayout = new QHBoxLayout();
    m_startYearSpin = new QSpinBox(this);
    m_startYearSpin->setRange(-20000, 3000);
    m_startYearSpin->setAttribute(Qt::WA_MacShowFocusRect, false);
#ifdef Q_OS_APPLE
    if (isDarkMode) {
        m_startYearSpin->setStyleSheet("QSpinBox { background-color: #2b2b2b; color: #ffffff; border: 1px solid #555555; border-radius: 4px; padding: 2px; }");
    } else {
        m_startYearSpin->setStyleSheet("QSpinBox { border: 1px solid #cccccc; border-radius: 4px; padding: 2px; }");
    }
#endif
    m_startUncertainCheck = new QCheckBox("Uncertain (?)", this);
    startLayout->addWidget(m_startYearSpin);
    startLayout->addWidget(m_startUncertainCheck);

    auto *endLayout = new QHBoxLayout();
    m_endYearSpin = new QSpinBox(this);
    m_endYearSpin->setRange(-20000, 3000);
    m_endYearSpin->setAttribute(Qt::WA_MacShowFocusRect, false);
#ifdef Q_OS_APPLE
    if (isDarkMode) {
        m_endYearSpin->setStyleSheet("QSpinBox { background-color: #2b2b2b; color: #ffffff; border: 1px solid #555555; border-radius: 4px; padding: 2px; }");
    } else {
        m_endYearSpin->setStyleSheet("QSpinBox { border: 1px solid #cccccc; border-radius: 4px; padding: 2px; }");
    }
#endif
    m_endUncertainCheck = new QCheckBox("Uncertain (?)", this);
    endLayout->addWidget(m_endYearSpin);
    endLayout->addWidget(m_endUncertainCheck);

    m_placeEdit = new QLineEdit(this);
    m_placeEdit->setMinimumWidth(fieldWidth);
#ifdef Q_OS_APPLE
    if (isDarkMode) {
        m_placeEdit->setStyleSheet("QLineEdit { background-color: #2b2b2b; color: #ffffff; border: 1px solid #555555; border-radius: 4px; padding: 2px; }");
    } else {
        m_placeEdit->setStyleSheet("QLineEdit { border-radius: 4px; padding: 2px; }");
    }
#endif

    // Notes Formatting Section
    auto *notesContainer = new QWidget(this);
    auto *notesLayout = new QVBoxLayout(notesContainer);
    notesLayout->setContentsMargins(0, 0, 0, 0);

    auto *notesHeaderLayout = new QHBoxLayout();

    m_h1Btn = new QToolButton(this);
    m_h1Btn->setText("H1");
    m_h1Btn->setCheckable(true);
    m_h1Btn->setToolTip("Header 1");
    m_h1Btn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_1));
    m_h1Btn->setStyleSheet("QToolButton { font-weight: bold; padding: 2px 6px; }");

    m_h2Btn = new QToolButton(this);
    m_h2Btn->setText("H2");
    m_h2Btn->setCheckable(true);
    m_h2Btn->setToolTip("Header 2");
    m_h2Btn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_2));
    m_h2Btn->setStyleSheet("QToolButton { font-weight: bold; padding: 2px 6px; }");

    m_h3Btn = new QToolButton(this);
    m_h3Btn->setText("H3");
    m_h3Btn->setCheckable(true);
    m_h3Btn->setToolTip("Header 3");
    m_h3Btn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_3));
    m_h3Btn->setStyleSheet("QToolButton { font-weight: bold; padding: 2px 6px; }");

    m_boldBtn = new QToolButton(this);
    m_boldBtn->setText("B");
    m_boldBtn->setCheckable(true);
    m_boldBtn->setShortcut(QKeySequence::Bold);
    m_boldBtn->setToolTip("Bold");
    m_boldBtn->setStyleSheet("QToolButton { font-weight: bold; padding: 2px 8px; }");

    m_italicBtn = new QToolButton(this);
    m_italicBtn->setText("I");
    m_italicBtn->setCheckable(true);
    m_italicBtn->setShortcut(QKeySequence::Italic);
    m_italicBtn->setToolTip("Italic");
    m_italicBtn->setStyleSheet("QToolButton { font-style: italic; padding: 2px 8px; }");

    m_clearFmtBtn = new QToolButton(this);
    m_clearFmtBtn->setText("Tx");
    m_clearFmtBtn->setToolTip("Clear Formatting");
    m_clearFmtBtn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_4));
    m_clearFmtBtn->setStyleSheet("QToolButton { padding: 2px 6px; }");

    m_tagsCombo = new QComboBox(this);
    m_tagsCombo->setMinimumWidth(200);
    if (!s_cachedTags.isEmpty()) {
        m_tagsCombo->addItems(s_cachedTags);
    }
#ifdef Q_OS_APPLE
    if (isDarkMode) {
        m_tagsCombo->setStyleSheet("QComboBox { background-color: #2b2b2b; color: #ffffff; border: 1px solid #555555; border-radius: 4px; padding: 2px; }");
    } else {
        m_tagsCombo->setStyleSheet("QComboBox { border-radius: 4px; padding: 2px; }");
    }
#endif

    m_fillTagsBtn = new QPushButton("Fill", this);
    m_insertTagBtn = new QPushButton("Insert", this);

    notesHeaderLayout->addWidget(m_h1Btn);
    notesHeaderLayout->addWidget(m_h2Btn);
    notesHeaderLayout->addWidget(m_h3Btn);
    notesHeaderLayout->addWidget(m_boldBtn);
    notesHeaderLayout->addWidget(m_italicBtn);
    notesHeaderLayout->addWidget(m_clearFmtBtn);
    notesHeaderLayout->addSpacing(30);
    notesHeaderLayout->addWidget(m_tagsCombo);
    notesHeaderLayout->addWidget(m_fillTagsBtn);
    notesHeaderLayout->addWidget(m_insertTagBtn);
    notesHeaderLayout->addStretch();

    m_notesEdit = new NotesTextEdit(this);
    m_notesEdit->setAcceptRichText(true);
    QFont memoFont = m_notesEdit->font();
    memoFont.setPointSize(qApp->font().pointSize() + 5);
    memoFont.setFamily("Avenir Next");
    m_notesEdit->setFont(memoFont);
    m_notesEdit->document()->setDefaultFont(memoFont);
    m_notesEdit->setMinimumHeight(120);
    if (isDarkMode) {
        m_notesEdit->setStyleSheet("QTextEdit { background-color: #2b2b2b; color: #ffffff; border: 1px solid #555555; border-radius: 4px; }");
    }

#ifdef Q_OS_APPLE
    QShortcut *notesZoomInShortcut = new QShortcut(QKeySequence(Qt::META | Qt::ALT | Qt::Key_Plus), this);
    QShortcut *notesZoomInEqualShortcut = new QShortcut(QKeySequence(Qt::META | Qt::ALT | Qt::Key_Equal), this); // Handles unshifted '+' key
    QShortcut *notesZoomOutShortcut = new QShortcut(QKeySequence(Qt::META | Qt::ALT | Qt::Key_Minus), this);
#else
    QShortcut *notesZoomInShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Plus), this);
    QShortcut *notesZoomInEqualShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Equal), this); // Handles unshifted '+' key
    QShortcut *notesZoomOutShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Minus), this);
#endif

    connect(notesZoomInShortcut, &QShortcut::activated, m_notesEdit, [this]() { m_notesEdit->zoomIn(1); });
    connect(notesZoomInEqualShortcut, &QShortcut::activated, m_notesEdit, [this]() { m_notesEdit->zoomIn(1); });
    connect(notesZoomOutShortcut, &QShortcut::activated, m_notesEdit, [this]() { m_notesEdit->zoomOut(1); });

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

    auto *searchLabel = new QLabel("Search:", this);
    m_searchEdit = new SearchLineEdit(this);
    m_searchEdit->setMaximumWidth(160);
#ifdef Q_OS_APPLE
    if (isDarkMode) {
        m_searchEdit->setStyleSheet("QLineEdit { background-color: #2b2b2b; color: #ffffff; border: 1px solid #555555; border-radius: 4px; padding: 2px; }");
    } else {
        m_searchEdit->setStyleSheet("QLineEdit { border-radius: 4px; padding: 2px; }");
    }
#endif
    m_searchFirstBtn = new QPushButton("First", this);
    m_searchNextBtn = new QPushButton("Next", this);
    auto *spellLabel = new QLabel("Spell-checking:", this);
    // Creazione e popolamento del combo box per il cambio lingua
    m_langCombo = new QComboBox(this);
    m_langCombo->addItem("No spell-checking", "none");
    m_langCombo->addItem("English", "en_US");
    m_langCombo->addItem("Italian", "it_IT");
    m_langCombo->addItem("German", "de_DE");
    m_langCombo->addItem("Spanish", "es_ES");
    m_langCombo->addItem("French", "fr");
    m_langCombo->addItem("Portuguese", "pt_PT");
    m_langCombo->setMinimumWidth(180);

    // Load saved language selection into the combo box
    QSettings settings("Massimo Nardello", "TableHistory");
    QString savedLang = settings.value("SpellCheck/Language", "en_US").toString();
    int langIndex = m_langCombo->findData(savedLang);
    if (langIndex != -1) {
        m_langCombo->setCurrentIndex(langIndex);
    }
#ifdef Q_OS_APPLE
    if (isDarkMode) {
        m_langCombo->setStyleSheet("QComboBox { background-color: #2b2b2b; color: #ffffff; border: 1px solid #555555; border-radius: 4px; padding: 2px; }");
    } else {
        m_langCombo->setStyleSheet("QComboBox { border-radius: 4px; padding: 2px; }");
    }
#endif

    buttonLayout->addWidget(searchLabel);
    buttonLayout->addWidget(m_searchEdit);
    buttonLayout->addWidget(m_searchFirstBtn);
    buttonLayout->addWidget(m_searchNextBtn);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(spellLabel);
    buttonLayout->addWidget(m_langCombo);

    buttonLayout->addStretch();

    auto *okBtn = new QPushButton("OK", this);
    okBtn->setDefault(true);
    okBtn->setAutoDefault(true);
    okBtn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));

    auto *cancelBtn = new QPushButton("Cancel", this);

    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);

    layout->addLayout(buttonLayout);

    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    connect(m_searchFirstBtn, &QPushButton::clicked, this, &ItemEditDialog::searchFirst);
    connect(m_searchNextBtn, &QPushButton::clicked, this, &ItemEditDialog::searchNext);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &ItemEditDialog::searchNext);

    connect(m_boldBtn, &QToolButton::toggled, this, &ItemEditDialog::onBoldToggled);
    connect(m_italicBtn, &QToolButton::toggled, this, &ItemEditDialog::onItalicToggled);
    connect(m_h1Btn, &QToolButton::clicked, this, &ItemEditDialog::onH1Clicked);
    connect(m_h2Btn, &QToolButton::clicked, this, &ItemEditDialog::onH2Clicked);
    connect(m_h3Btn, &QToolButton::clicked, this, &ItemEditDialog::onH3Clicked);
    connect(m_clearFmtBtn, &QToolButton::clicked, this, &ItemEditDialog::onClearFmtClicked);

    connect(m_fillTagsBtn, &QPushButton::clicked, this, &ItemEditDialog::onFillTagsClicked);
    connect(m_insertTagBtn, &QPushButton::clicked, this, &ItemEditDialog::onInsertTagClicked);

    connect(m_notesEdit, &QTextEdit::currentCharFormatChanged, this, &ItemEditDialog::updateFormatButtons);
    connect(m_notesEdit, &QTextEdit::cursorPositionChanged, this, &ItemEditDialog::updateFormatButtons);

    connect(m_langCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
        QString langCode = m_langCombo->itemData(index).toString();
        m_notesEdit->setLanguage(langCode);
    });

    layout->activate();
    QSize baseSize = sizeHint();
    int targetWidth = baseSize.width() * 1.2;
    int targetHeight = baseSize.height() * 1.3;

    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        targetWidth = qMin(targetWidth, screen->availableGeometry().width());
        targetHeight = qMin(targetHeight, screen->availableGeometry().height());
    }

    resize(targetWidth, targetHeight);
}

void ItemEditDialog::clearCachedTags() {
    s_cachedTags.clear();
}

void ItemEditDialog::onFillTagsClicked() {
    QList<HistoryItem> items;
    MainWindow *mainWin = qobject_cast<MainWindow*>(parent());
    if (mainWin) {
        items = mainWin->allItems();
    }
    items.append(getItem());

    QSet<QString> uniqueTags;
    QRegularExpression tagRegex(R"(#[^#\s]+#)");

    for (const auto &item : std::as_const(items)) {
        QTextDocument doc;
        doc.setHtml(item.notes);

        QTextBlock block = doc.begin();
        while (block.isValid()) {
            QString blockText = block.text();
            QRegularExpressionMatchIterator it = tagRegex.globalMatch(blockText);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                uniqueTags.insert(match.captured(0));
            }
            block = block.next();
        }
    }

    QStringList sortedTags = uniqueTags.values();
    sortedTags.sort(Qt::CaseInsensitive);

    // Save to runtime-only cache
    s_cachedTags = sortedTags;

    m_tagsCombo->clear();
    m_tagsCombo->addItems(sortedTags);
}

void ItemEditDialog::onInsertTagClicked() {
    QString tag = m_tagsCombo->currentText();
    if (tag.isEmpty()) return;

    QTextCursor cursor = m_notesEdit->textCursor();
    cursor.insertText(tag + " "); // Inserts the tag followed by a space
    m_notesEdit->setFocus();
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

    QString html = item.notes;
    // Remove explicit font-size from inline styles so zoomIn/zoomOut works correctly
    QRegularExpression fontSizeRegex(R"(font-size\s*:\s*[^;"]+;?)", QRegularExpression::CaseInsensitiveOption);
    html.replace(fontSizeRegex, "");
    QRegularExpression emptyStyleRegex(R"(\s*style\s*=\s*["']\s*["'])", QRegularExpression::CaseInsensitiveOption);
    html.replace(emptyStyleRegex, "");
    m_notesEdit->setHtml(html);

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
        charFmt.setFontPointSize(defaultSize * 2.0);
        charFmt.setFontWeight(QFont::Bold);
    } else if (newLevel == 2) {
        charFmt.setFontPointSize(defaultSize * 1.5);
        charFmt.setFontWeight(QFont::Bold);
    } else if (newLevel == 3) {
        charFmt.setFontPointSize(defaultSize * 1.2);
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

    // Fallback: If headingLevel is not set, check font size and weight
    if (headingLevel == 0) {
        qreal defaultSize = m_notesEdit->font().pointSizeF();
        if (defaultSize <= 0) defaultSize = 10.0;
        qreal currentSize = format.fontPointSize();

        if (currentSize > 0 && format.fontWeight() >= QFont::Bold) {
            if (qAbs(currentSize - (defaultSize * 2.0)) < 0.5) headingLevel = 1;
            else if (qAbs(currentSize - (defaultSize * 1.5)) < 0.5) headingLevel = 2;
            else if (qAbs(currentSize - (defaultSize * 1.2)) < 0.5) headingLevel = 3;
        }
    }

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

void ItemEditDialog::searchFirst() {
    QString query = m_searchEdit->text();
    if (query.isEmpty()) return;

    QTextCursor cursor = m_notesEdit->textCursor();
    cursor.movePosition(QTextCursor::Start);
    m_notesEdit->setTextCursor(cursor);

    m_notesEdit->find(query);
}

void ItemEditDialog::searchNext() {
    QString query = m_searchEdit->text();
    if (query.isEmpty()) return;

    bool found = m_notesEdit->find(query);
    if (!found) {
        // Wrap around to the beginning if reaching the end of the notes
        QTextCursor cursor = m_notesEdit->textCursor();
        cursor.movePosition(QTextCursor::Start);
        m_notesEdit->setTextCursor(cursor);
        m_notesEdit->find(query);
    }
}
