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

#ifndef ITEMEDITDIALOG_H
#define ITEMEDITDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextList>
#include <QKeyEvent>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QToolButton>
#include "historyitem.h"
#include "notestextedit.h"

class ItemEditDialog : public QDialog {
    Q_OBJECT

public:
    explicit ItemEditDialog(QWidget *parent = nullptr);
    explicit ItemEditDialog(const HistoryItem &item, QWidget *parent = nullptr);

    HistoryItem getItem() const;

private slots:
    void onBoldToggled(bool checked);
    void onItalicToggled(bool checked);
    void onH1Clicked();
    void onH2Clicked();
    void onH3Clicked();
    void updateFormatButtons();
    void onClearFmtClicked();

private:
    void setupUi();
    void loadItemData(const HistoryItem &item);
    void applyHeadingLevel(int level);

    QComboBox *m_kindCombo = nullptr;
    QLineEdit *m_nameEdit = nullptr;
    QSpinBox *m_startYearSpin = nullptr;
    QCheckBox *m_startUncertainCheck = nullptr;
    QSpinBox *m_endYearSpin = nullptr;
    QCheckBox *m_endUncertainCheck = nullptr;
    QLineEdit *m_placeEdit = nullptr;
    NotesTextEdit *m_notesEdit = nullptr;

    QToolButton *m_boldBtn = nullptr;
    QToolButton *m_italicBtn = nullptr;
    QToolButton *m_h1Btn = nullptr;
    QToolButton *m_h2Btn = nullptr;
    QToolButton *m_h3Btn = nullptr;
    QToolButton *m_clearFmtBtn;
};

#endif // ITEMEDITDIALOG_H
