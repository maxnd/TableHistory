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

#ifndef NOTESTEXTEDIT_H
#define NOTESTEXTEDIT_H

#include <QTextEdit>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextList>
#include <QKeyEvent>
#include <QMimeData>
#include <QShortcut>
#include <QGuiApplication>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <hunspell.hxx>

class SpellCheckHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    SpellCheckHighlighter(Hunspell *hunspell, QTextDocument *parent = nullptr)
        : QSyntaxHighlighter(parent), m_hunspell(hunspell) {}

    void setHunspell(Hunspell *hunspell) {
        m_hunspell = hunspell;
        rehighlight();
    }

protected:
    void highlightBlock(const QString &text) override {
        if (!m_hunspell) return;

        QRegularExpression wordRegex(R"(\b[A-Za-zÀ-ÿ'-]+\b)");
        QRegularExpressionMatchIterator i = wordRegex.globalMatch(text);

        QTextCharFormat wavyFormat;
        wavyFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        wavyFormat.setUnderlineColor(Qt::red);

        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            QString word = match.captured(0);

            if (word.length() <= 1) continue;

            if (!m_hunspell->spell(word.toStdString())) {
                setFormat(match.capturedStart(), match.capturedLength(), wavyFormat);
            }
        }
    }

private:
    Hunspell *m_hunspell;
};

class NotesTextEdit : public QTextEdit {
    Q_OBJECT
public:
    QString extractDictFile(const QString &resPath, const QString &fileName) {
        QString destDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(destDir);
        QString destPath = destDir + "/" + fileName;

        if (QFile::exists(destPath)) {
            QFile::remove(destPath);
        }
        QFile::copy(resPath, destPath);
        QFile::setPermissions(destPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        return destPath;
    }

    void loadUserDictionary(const QString &langCode) {
        if (!m_hunspell || langCode.isEmpty() || langCode == "none") return;
        QString userDictPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/user_dict_" + langCode + ".txt";
        QFile file(userDictPath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString word = in.readLine().trimmed();
                if (!word.isEmpty()) {
                    m_hunspell->add(word.toStdString().c_str());
                }
            }
            file.close();
        }
    }

    void addUserWord(const QString &word, const QString &langCode) {
        if (!m_hunspell || word.isEmpty() || langCode.isEmpty() || langCode == "none") return;

        m_hunspell->add(word.toStdString().c_str());

        QString userDictPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/user_dict_" + langCode + ".txt";
        QFile file(userDictPath);
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file);
            out << word << "\n";
            file.close();
        }

        if (m_highlighter) {
            m_highlighter->rehighlight();
        }
    }

    explicit NotesTextEdit(QWidget *parent = nullptr) : QTextEdit(parent) {
        QSettings settings("Massimo Nardello", "TableHistory");
        QString langCode = settings.value("SpellCheck/Language", "en_US").toString();

        if (langCode.isEmpty() || langCode == "none") {
            m_hunspell = nullptr;
        } else {
            QString affPath = extractDictFile(QString(":/dicts/%1.aff").arg(langCode), langCode + ".aff");
            QString dicPath = extractDictFile(QString(":/dicts/%1.dic").arg(langCode), langCode + ".dic");
            m_hunspell = new Hunspell(affPath.toStdString().c_str(), dicPath.toStdString().c_str());
            loadUserDictionary(langCode);
        }

        m_highlighter = new SpellCheckHighlighter(m_hunspell, document());

        QShortcut *plainPasteShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V), this);
        connect(plainPasteShortcut, &QShortcut::activated, this, &NotesTextEdit::pastePlainText);
    }

    ~NotesTextEdit() {
        delete m_hunspell;
    }

public slots:
    void setLanguage(const QString &langCode) {
        QSettings settings("Massimo Nardello", "TableHistory");
        settings.setValue("SpellCheck/Language", langCode);

        if (m_hunspell) {
            delete m_hunspell;
            m_hunspell = nullptr;
        }

        if (!langCode.isEmpty() && langCode != "none") {
            QString affPath = extractDictFile(QString(":/dicts/%1.aff").arg(langCode), langCode + ".aff");
            QString dicPath = extractDictFile(QString(":/dicts/%1.dic").arg(langCode), langCode + ".dic");
            m_hunspell = new Hunspell(affPath.toStdString().c_str(), dicPath.toStdString().c_str());
            loadUserDictionary(langCode);
        }

        if (m_highlighter) {
            m_highlighter->setHunspell(m_hunspell);
        }
    }

protected:
    void insertFromMimeData(const QMimeData *source) override {
        int startPos = textCursor().position();
        QTextEdit::insertFromMimeData(source);
        int endPos = textCursor().position();

        QTextBlock block = document()->findBlock(startPos);
        QTextBlock endBlock = document()->findBlock(qMax(startPos, endPos - 1));

        qreal defaultSize = font().pointSizeF();
        if (defaultSize <= 0) defaultSize = 10.0;

        while (block.isValid() && block.position() <= endBlock.position()) {
            qreal maxPtSize = 0;
            bool hasBold = false;

            for (QTextBlock::Iterator it = block.begin(); !it.atEnd(); ++it) {
                QTextFragment fragment = it.fragment();
                if (fragment.isValid()) {
                    QTextCharFormat fmt = fragment.charFormat();
                    qreal sz = fmt.fontPointSize();
                    if (sz <= 0) {
                        sz = fmt.font().pointSizeF();
                    }
                    if (sz > maxPtSize) {
                        maxPtSize = sz;
                    }
                    if (fmt.fontWeight() >= QFont::Bold) {
                        hasBold = true;
                    }
                }
            }

            int detectedLevel = 0;
            if (maxPtSize > 0 && hasBold) {
                if (maxPtSize >= defaultSize * 1.7) {
                    detectedLevel = 1;
                } else if (maxPtSize >= defaultSize * 1.3) {
                    detectedLevel = 2;
                } else if (maxPtSize > defaultSize * 1.1) {
                    detectedLevel = 3;
                }
            }

            if (detectedLevel > 0) {
                QTextCursor cursor(block);
                QTextBlockFormat bFmt = block.blockFormat();
                bFmt.setHeadingLevel(detectedLevel);
                cursor.setBlockFormat(bFmt);
            }

            if (block == endBlock) break;
            block = block.next();
        }
    }

    void keyPressEvent(QKeyEvent *e) override {
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
            QTextCursor cursor = textCursor();
            QTextBlock block = cursor.block();
            QString text = block.text().trimmed();

            if (cursor.currentList() && text.isEmpty()) {
                cursor.setBlockFormat(QTextBlockFormat());
                e->accept();
                return;
            }
        } else if (e->key() == Qt::Key_Space) {
            QTextCursor cursor = textCursor();
            QTextBlock block = cursor.block();
            QString text = block.text().trimmed();

            if (text == "-" || text == "*" || text == "+") {
                cursor.select(QTextCursor::BlockUnderCursor);
                cursor.removeSelectedText();
                cursor.createList(QTextListFormat::ListDisc);
                e->accept();
                return;
            } else if (text == "1.") {
                cursor.select(QTextCursor::BlockUnderCursor);
                cursor.removeSelectedText();
                cursor.createList(QTextListFormat::ListDecimal);
                e->accept();
                return;
            }
        }
        QTextEdit::keyPressEvent(e);
    }

    void contextMenuEvent(QContextMenuEvent *event) override {
        QMenu *menu = createStandardContextMenu(event->pos());

        QTextCursor cursor = cursorForPosition(event->pos());
        cursor.select(QTextCursor::WordUnderCursor);
        QString clickedWord = cursor.selectedText();

        if (m_hunspell && !clickedWord.isEmpty()) {
            if (!m_hunspell->spell(clickedWord.toStdString())) {
                menu->addSeparator();

                QAction *addWordAction = menu->addAction(tr("Add to Dictionary"));
                connect(addWordAction, &QAction::triggered, this, [this, clickedWord]() {
                    QSettings settings("Massimo Nardello", "TableHistory");
                    QString langCode = settings.value("SpellCheck/Language", "en_US").toString();
                    addUserWord(clickedWord, langCode);
                });

                std::vector<std::string> suggestions = m_hunspell->suggest(clickedWord.toStdString());

                if (!suggestions.empty()) {
                    QMenu *spellMenu = menu->addMenu(tr("Spelling Suggestions"));

                    int count = 0;
                    for (const auto &sug : suggestions) {
                        if (count >= 5) break;
                        QString suggestion = QString::fromStdString(sug);
                        QAction *sugAction = spellMenu->addAction(suggestion);
                        connect(sugAction, &QAction::triggered, this, [this, cursor, suggestion]() {
                            QTextCursor c = cursor;
                            c.beginEditBlock();
                            c.removeSelectedText();
                            c.insertText(suggestion);
                            c.endEditBlock();
                        });
                        ++count;
                    }
                }
            }
        }

        menu->addSeparator();

        QAction *pastePlainTextAction = new QAction(tr("Paste as Plain Text"), menu);
        pastePlainTextAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V));
        connect(pastePlainTextAction, &QAction::triggered, this, &NotesTextEdit::pastePlainText);

        QAction *pasteAction = nullptr;
        QAction *actionAfterPaste = nullptr;
        QList<QAction *> actions = menu->actions();

        for (int i = 0; i < actions.size(); ++i) {
            QAction *action = actions.at(i);
            if (action->shortcut() == QKeySequence::Paste || action->text().contains("Paste", Qt::CaseInsensitive)) {
                pasteAction = action;
                if (!action->icon().isNull()) {
                    pastePlainTextAction->setIcon(action->icon());
                }
                if (i + 1 < actions.size()) {
                    actionAfterPaste = actions.at(i + 1);
                }
                break;
            }
        }

        const QMimeData *source = QGuiApplication::clipboard()->mimeData();
        pastePlainTextAction->setEnabled(source && source->hasText());

        if (actionAfterPaste) {
            menu->insertAction(actionAfterPaste, pastePlainTextAction);
        } else if (pasteAction) {
            menu->addAction(pastePlainTextAction);
        } else {
            menu->addAction(pastePlainTextAction);
        }

        menu->exec(event->globalPos());
        delete menu;
    }

private slots:
    void pastePlainText() {
        const QMimeData *source = QGuiApplication::clipboard()->mimeData();
        if (source && source->hasText()) {
            insertPlainText(source->text());
        }
    }

private:
    Hunspell *m_hunspell = nullptr;
    SpellCheckHighlighter *m_highlighter = nullptr;
};

#endif // NOTESTEXTEDIT_H
