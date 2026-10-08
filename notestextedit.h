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

class NotesTextEdit : public QTextEdit {
    Q_OBJECT
public:
    explicit NotesTextEdit(QWidget *parent = nullptr) : QTextEdit(parent) {}

protected:
    void insertFromMimeData(const QMimeData *source) override {
        if (source->hasText()) {
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
};

#endif // NOTESTEXTEDIT_H
