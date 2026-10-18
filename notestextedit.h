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

class NotesTextEdit : public QTextEdit {
    Q_OBJECT
public:
    explicit NotesTextEdit(QWidget *parent = nullptr) : QTextEdit(parent) {
        // Setup Ctrl + Shift + V shortcut for plain text paste
        QShortcut *plainPasteShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V), this);
        connect(plainPasteShortcut, &QShortcut::activated, this, &NotesTextEdit::pastePlainText);
    }

protected:
    void insertFromMimeData(const QMimeData *source) override {
        // Normal paste: preserve rich text / HTML formatting if available
        if (source->hasHtml()) {
            insertHtml(source->html());
        } else if (source->hasText()) {
            insertPlainText(source->text());
        } else {
            QTextEdit::insertFromMimeData(source);
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

    // Override contextMenuEvent to safely add "Paste as Plain Text" to the popup menu
    void contextMenuEvent(QContextMenuEvent *event) override {
        QMenu *menu = createStandardContextMenu(event->pos());

        // Create the "Paste as Plain Text" action
        QAction *pastePlainTextAction = new QAction(tr("Paste as Plain Text"), menu);
        pastePlainTextAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V));
        connect(pastePlainTextAction, &QAction::triggered, this, &NotesTextEdit::pastePlainText);

        // Find the regular Paste action to borrow its icon and determine placement
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

        // Enable or disable based on clipboard content
        const QMimeData *source = QGuiApplication::clipboard()->mimeData();
        pastePlainTextAction->setEnabled(source && source->hasText());

        // Insert the action right below the regular Paste item
        if (actionAfterPaste) {
            menu->insertAction(actionAfterPaste, pastePlainTextAction);
        } else if (pasteAction) {
            menu->addAction(pastePlainTextAction);
        } else {
            menu->addSeparator();
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
};

#endif // NOTESTEXTEDIT_H
